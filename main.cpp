#include "gui/MainWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget wgt;
    wgt.show();
    return a.exec();
}
