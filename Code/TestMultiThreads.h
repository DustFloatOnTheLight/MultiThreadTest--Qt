#ifndef TESTMULTITHREADS_H
#define TESTMULTITHREADS_H

#include <QRunnable>
#include <QThreadPool>
#include <QThread>
#include <QtConcurrent>

#include <QMutexLocker>
#include <QFile>
#include <QList>
#include <QEventLoop>

class TestThread;
class TestRunnable;

class TestMultiThreads
{
public:
    TestMultiThreads();

    static void startTest();
};

class MultiThreadTestTool : public QObject
{
    Q_OBJECT

public:
    MultiThreadTestTool();
    ~MultiThreadTestTool();

    void start();

private slots:
    void onThreadFinished(int threadId);
    void doWriteData(QFile *outputFile, int threadId);

private:
    bool initInputFile();

    void testSingleThread();

    void testQThread();
    void initQThreads();

    void testQRunnable();
    void initQRunnables();

    void testQConCurrent();
    void startQConCurrentTasks();

private:
    int m_nCurIndex;

    QList<TestThread *> m_oThreads;
    QList<TestRunnable *> m_oRunnables;

    QEventLoop *m_pLoop;
    QFile *m_pInputFile;
    QMutex oMutex;
    QThreadPool m_oThreadPool;

    QString m_sInputFileName;
};

class TestThread : public QThread
{
    Q_OBJECT
public:
    TestThread(int id, QFile *inputFile, QString outputFileName = "");
    TestThread(int id, QString inputFileName = "", QString outputFileName = "");
    ~TestThread();

protected:
    virtual void run() override;

private slots:
    void onFineshed();

signals:
    void finish(int threadId);
    void writeData(QFile *outputFile, int threadId);

private:
    int m_nThreadID;

    QFile *m_pInputFile;
    QFile *m_pOutputFile;
    QString m_sInputFileName;
    QString m_sOutputFileName;
};

class TestRunnable : public QRunnable
{
public:
    TestRunnable(int id, QString inputFileName = "", QString outputFileName = "");
    virtual ~TestRunnable();

    virtual void run() override;

private:
    int m_nThreadID;

    QFile *m_pInputFile;
    QFile *m_pOutputFile;
    QString m_sInputFileName;
    QString m_sOutputFileName;
};

class TestConcurrent
{
public:
    static void start(int id, QString inputFileName = "", QString outputFileName = "");
};

class TestUtils
{
public:
    static void transferData(QFile *outputFile, QFile *inputFile);
};

#endif // TESTMULTITHREADS_H
