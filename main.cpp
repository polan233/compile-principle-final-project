#include "compiler.h"

#include <QApplication>
compiler* w;
QEventLoop loop;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    w=new compiler;
    w->show();
    return a.exec();
}
