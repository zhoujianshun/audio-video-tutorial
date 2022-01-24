#include "mainwindow.h"

#include <QApplication>
#include "ffmpegs.h"

extern "C"{
    #include <libavdevice/avdevice.h>
}

int main(int argc, char *argv[])
{
//    WAVHeader header;

//    header.numChannels = 1;
//    header.sampleRate = 48000;
//    header.bitsPerSample = 32;
//    header.audioFormat = AUDIO_FORMAT_FLOAT;

//    FFmpegs::pcm2wav(header, "/Users/zhoujianshun/Downloads/12_10_17_12_54.pcm", "/Users/zhoujianshun/Downloads/12_10_17_12_54_3.wav");

    // 注册设备
    avdevice_register_all();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
