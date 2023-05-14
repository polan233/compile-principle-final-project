#include "compiler.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    compiler w;
    w.show();
    return a.exec();
}
