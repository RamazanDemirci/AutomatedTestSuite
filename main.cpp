#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QQuickStyle::setStyle("Material");
    //QApplication::setStyle("windowsvista");
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/res/recon_icon.ico"));


    MainWindow w;
    w.show();

    return a.exec();
}
