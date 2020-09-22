#include "TestMultiThreads.h"

#include <QString>
#include <QFileDevice>
#include <QDebug>
#include <functional>

#define MAX_THREAD_CONUT 9

const QString c_sInputFileDir = "F:\\TestMultiThreads\\read\\";
const QString c_sOutputFileDir = "F:\\TestMultiThreads\\write\\";
const QString c_sFileSuffix = ".txt";

TestMultiThreads::TestMultiThreads()
{

}

void TestMultiThreads::startTest()
{
    MultiThreadTestTool oTestTool;
    oTestTool.start();
}

MultiThreadTestTool::MultiThreadTestTool()
    : m_nCurIndex(-1), m_pLoop(nullptr), m_pInputFile(nullptr)
{

}

MultiThreadTestTool::~MultiThreadTestTool()
{
    if (nullptr != m_pLoop)
    {
        delete m_pLoop;
    }
    if (nullptr != m_pInputFile)
    {
        m_pInputFile->close();
        delete m_pInputFile;
    }

    // �ͷ��ڴ�
    for (int i = 0; i < m_oThreads.count(); i++)
    {
        if (nullptr != m_oThreads[i])
        {
            delete m_oThreads[i];
        }
    }
    m_oThreads.clear();
}

//#define TEST_QTHREAD
//#define TEST_QRUNNABLE
//#define TEST_QCONCURRENT
//#define TEST_SINGLE_THREAD

void MultiThreadTestTool::start()
{
    if (!initInputFile())
    {
        return;
    }

#ifdef TEST_QTHREAD
    testQThread();
#elif defined(TEST_QRUNNABLE)
    testQRunnable();
#elif defined(TEST_QCONCURRENT)
    testQConCurrent();
#else
    testSingleThread();
#endif

}

void MultiThreadTestTool::onThreadFinished(int threadId)
{
    // ���ڷ�ֹ�߳���δ�����ͽ��룬��Ȼstart�ǲ������õ�
    if (m_oThreads[threadId]->isRunning())
    {
        m_oThreads[threadId]->wait();
    }

    if (m_nCurIndex == MAX_THREAD_CONUT - 1)
    {
        // �����һ���Ѿ������̣߳�����Ҫ�ȴ����һ����ɼ���
        for (int i = 0; i < m_oThreads.count(); i++)
        {
            if (m_oThreads[i]->isRunning())
            {
                return;
            }
        }

        // �����̶߳���ɺ�ֹͣEventLoop������ִ�����߳�
        if (nullptr != m_pLoop)
        {
            m_pLoop->exit();
        }
    }
}

void MultiThreadTestTool::doWriteData(QFile *outputFile, int threadId)
{
    Q_UNUSED(threadId)
    oMutex.lock();
    m_pInputFile->seek(0);
    QString sInputContent(m_pInputFile->readAll());
    oMutex.unlock();

    qDebug() << sInputContent.length();
    sInputContent.replace("begin", "");
    sInputContent.replace("end", "");

    outputFile->write(sInputContent.toLocal8Bit());
}

bool MultiThreadTestTool::initInputFile()
{
    m_sInputFileName = c_sInputFileDir + QString::number(0).append(c_sFileSuffix);
    m_pInputFile = new QFile(m_sInputFileName);
    if (!m_pInputFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    return true;
}

void MultiThreadTestTool::testSingleThread()
{
    QString sOutputFileName = c_sOutputFileDir + QString("%1").append(c_sFileSuffix);
    for (int i = 0 ; i < MAX_THREAD_CONUT; i++)
    {
        m_pInputFile->seek(0);
        QFile oOutputFile(sOutputFileName.arg(i));
        if (oOutputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            TestUtils::transferData(&oOutputFile, m_pInputFile);
            oOutputFile.close();
        }
    }
}

void MultiThreadTestTool::testQThread()
{
    initQThreads();

    // �������߳�
    m_pLoop = new QEventLoop;
    m_pLoop->exec();
}

void MultiThreadTestTool::initQThreads()
{
    QString sOutputFileName = c_sOutputFileDir + QString("%1").append(c_sFileSuffix);
    for (int i = 0 ; i < MAX_THREAD_CONUT; i++)
    {
//        TestThread *pThread = new TestThread(i, m_sInputFileName, sOutputFileName.arg(i));
        TestThread *pThread = new TestThread(i, m_pInputFile, sOutputFileName.arg(i));
        connect(pThread, &TestThread::finish, this, &MultiThreadTestTool::onThreadFinished);
        connect(pThread, &TestThread::writeData, this, &MultiThreadTestTool::doWriteData, Qt::DirectConnection);
        pThread->start();

        m_oThreads.append(pThread);
        ++m_nCurIndex;
    }
}

void MultiThreadTestTool::testQRunnable()
{
    initQRunnables();
}

void MultiThreadTestTool::initQRunnables()
{
    QString sOutputFileName = c_sOutputFileDir + QString("%1").append(c_sFileSuffix);
    for (int i = 0 ; i < MAX_THREAD_CONUT; i++)
    {
        // �̳߳�Ĭ���Զ�����QRunnable�������ڣ�����Ҫ�ֶ��ͷ�
        TestRunnable *pRunnable = new TestRunnable(i, m_sInputFileName, sOutputFileName.arg(i));
        m_oThreadPool.start(pRunnable);

        m_oRunnables.append(pRunnable);
    }
    m_oThreadPool.waitForDone();
}

void MultiThreadTestTool::testQConCurrent()
{
    startQConCurrentTasks();
}

#ifdef TEST_QCONCURRENT
//    #define METHOD1
//    #define METHOD2
    #define METHOD3_1
//    #define METHOD3_2
#endif

void MultiThreadTestTool::startQConCurrentTasks()
{
    // ���Բ�ʹ���̳߳�
    typedef void (*TestConcurrentFunc)(int, QString, QString);
    TestConcurrentFunc pFunc(TestConcurrent::start);

    std::function<void (int, QString, QString)> oFunc1(TestConcurrent::start);
    std::function<void (int, QString, QString)> oFunc2(pFunc);

    QString sOutputFileName = c_sOutputFileDir + QString("%1").append(c_sFileSuffix);
    for (int i = 0 ; i < MAX_THREAD_CONUT; i++)
    {
#ifdef METHOD1
        // ʹ��Concurrent�ļ��ַ�ʽ
        QtConcurrent::run(&m_oThreadPool, TestConcurrent::start, i, m_sInputFileName, sOutputFileName.arg(i)); // ��ȷʹ��
#elif defined(METHOD2)
        QtConcurrent::run(&m_oThreadPool, pFunc, i, m_sInputFileName, sOutputFileName.arg(i)); // ��ȷʹ��
#elif defined(METHOD3_1)
//        QtConcurrent::run(&m_oThreadPool, oFunc1); // ����ʹ��
        QtConcurrent::run(&m_oThreadPool, oFunc1, i, m_sInputFileName, sOutputFileName.arg(i)); // ��ȷʹ��
#elif defined(METHOD3_2)
//        QtConcurrent::run(&m_oThreadPool, oFunc2); // ����ʹ��
        QtConcurrent::run(&m_oThreadPool, oFunc2, i, m_sInputFileName, sOutputFileName.arg(i)); // ��ȷʹ��
#endif
    }
}

TestThread::TestThread(int id, QFile *inputFile, QString outputFileName)
    : m_nThreadID(id), m_pInputFile(inputFile), m_sInputFileName(""), m_sOutputFileName(outputFileName)
{
    m_pOutputFile = new QFile(m_sOutputFileName);
    connect(this, &QThread::finished, this, &TestThread::onFineshed);
}

TestThread::TestThread(int id, QString inputFileName, QString outputFileName)
    : m_nThreadID(id), m_pInputFile(nullptr), m_sInputFileName(inputFileName), m_sOutputFileName(outputFileName)
{
    m_pOutputFile = new QFile(m_sOutputFileName);
    connect(this, &QThread::finished, this, &TestThread::onFineshed);
}

TestThread::~TestThread()
{
    delete m_pOutputFile;
    m_pOutputFile = nullptr;

    if (!m_sInputFileName.isEmpty())
    {
        delete m_pInputFile;
    }
    m_pInputFile = nullptr;
}

void TestThread::run()
{
    if (!m_pOutputFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    if (!m_sInputFileName.isEmpty()) // ��һ�ַ�ʽ��ÿ���̷ֱ߳��inputFile��ʹ�ø��Ե�QFile���ֱ�д�벻ͬ��outputFile
    {
        m_pInputFile = new QFile(m_sInputFileName);
        if (!m_pInputFile->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }
        TestUtils::transferData(m_pOutputFile, m_pInputFile);
        m_pInputFile->close();
    }
    else // �ڶ��ַ�ʽ�������߳��д�inputFile��ÿ���߳�ʹ��ͬһ��QFile���ֱ�д�벻ͬ��outputFile
    {
        emit writeData(m_pOutputFile, m_nThreadID);
    }
    m_pOutputFile->close();
}

void TestThread::onFineshed()
{
    emit finish(m_nThreadID);
}

TestRunnable::TestRunnable(int id, QString inputFileName, QString outputFileName)
    : m_nThreadID(id), m_pInputFile(nullptr), m_sInputFileName(inputFileName), m_sOutputFileName(outputFileName)
{
    m_pOutputFile = new QFile(m_sOutputFileName);
}

TestRunnable::~TestRunnable()
{
    if (nullptr != m_pOutputFile)
    {
        delete m_pOutputFile;
    }
    if (!m_sInputFileName.isEmpty())
    {
        delete m_pInputFile;
    }
}

void TestRunnable::run()
{
    if (!m_pOutputFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    if (!m_sInputFileName.isEmpty()) // ��һ�ַ�ʽ��ÿ���̷ֱ߳��inputFile��ʹ�ø��Ե�QFile���ֱ�д�벻ͬ��outputFile
    {
        m_pInputFile = new QFile(m_sInputFileName);
        if (!m_pInputFile->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }
        TestUtils::transferData(m_pOutputFile, m_pInputFile);
        m_pInputFile->close();
    }
    m_pOutputFile->close();
}

void TestConcurrent::start(int id, QString inputFileName, QString outputFileName)
{
    Q_UNUSED(id)
    QFile oOutputFile(outputFileName);
    if (!oOutputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    if (!inputFileName.isEmpty()) // ��һ�ַ�ʽ��ÿ���̷ֱ߳��inputFile��ʹ�ø��Ե�QFile���ֱ�д�벻ͬ��outputFile
    {
        QFile oInputFile(inputFileName);
        if (!oInputFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }
        TestUtils::transferData(&oOutputFile, &oInputFile);
        oInputFile.close();
    }
    oOutputFile.close();
}

void TestUtils::transferData(QFile *outputFile, QFile *inputFile)
{
    QString sInputContent(inputFile->readAll());
    qDebug() << sInputContent.length();
    sInputContent.replace("begin", "");
    sInputContent.replace("end", "");

    outputFile->write(sInputContent.toLocal8Bit());
}
