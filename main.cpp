#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QQuickStyle::setStyle("Material");
    //QApplication::setStyle("windowsvista");
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
