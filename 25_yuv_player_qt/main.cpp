#include "mainwindow.h"

#include <QApplication>

#define __STDC_CONSTANT_MACROS


#define D__STDC_CONSTANT_MACROS

#include <QDebug>
#include "ffmpegs.h"



int main(int argc, char *argv[])
{
    RawVideoFile in = {
        "/Users/zhoujianshun/Downloads/yuv/1.yuv",
        1280,
        720,
        AV_PIX_FMT_YUV420P,
    };

    RawVideoFile out ={
        "/Users/zhoujianshun/Downloads/yuv/1.rgb",
        1280,
        720,
        AV_PIX_FMT_RGB24,
    };

    FFmpegs::convertRawVideo(in, out);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int ret = a.exec();

    return ret;
}
