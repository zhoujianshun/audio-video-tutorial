#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

#include "ffmpges.h"

extern "C" {

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_MAC
    #define FILE_NAME "/Users/zhoujianshun/Downloads/yuv/1.yuv"
#elif Q_OS_WIN
    #define FORMATNAME "dshow"
    #define DEVICE_NAME ""
    #define FILE_NAME ""
#else

#endif


AudioThread::AudioThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread()
{
    qDebug() << "~AudioThread";
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}



void AudioThread::run()
{
    VideoEncodeSpec in;
    in.fileName = FILE_NAME;
    in.width =  1280;
    in.height = 720;
    in.format = AV_PIX_FMT_YUV420P;
    in.fps = 24;

    FFmpges::h264Encode(in,"/Users/zhoujianshun/Downloads/yuv/1.h264");
    qDebug() << "采集完毕";
}

void AudioThread::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}
