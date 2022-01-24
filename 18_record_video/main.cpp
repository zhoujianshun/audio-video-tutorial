#include "mainwindow.h"

#include <QApplication>
extern "C"{
    #include <libavdevice/avdevice.h>
}

int main(int argc, char *argv[])
{

    // 注册设备
    avdevice_register_all();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
