#include "Dialog.h"
#include "TestMultiThreads.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Dialog w;
//    w.show();

    TestMultiThreads::startTest();

    return 0;
}
