#include "mainwindow.h"
#include <QtWidgets\QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8")); // for visualization in Qt Russian symblos

    MainWindow w;
    w.show();

    return a.exec();
}
