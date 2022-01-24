#include "decodethread.h"


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
    #define FILE_NAME "/Users/zhoujianshun/Downloads/44100_2_s16le.pcm"
#elif Q_OS_WIN
    #define FORMATNAME "dshow"
    #define DEVICE_NAME ""
    #define FILE_NAME ""
#else

#endif


DecodeThread::DecodeThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &DecodeThread::finished,
            this, &DecodeThread::deleteLater);
}

DecodeThread::~DecodeThread()
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



void DecodeThread::run()
{
    VideoDecodeSpec out;
    out.fileName = "/Users/zhoujianshun/Downloads/yuv/h264.yuv";
//    in.sample_fmt = AV_SAMPLE_FMT_S16;
//    in.sample_rate = 44100;
//    in.ch_layout = AV_CH_LAYOUT_STEREO;

    FFmpges::h264Decode("/Users/zhoujianshun/Downloads/yuv/1.h264", out);
    qDebug() << "采集完毕";
}

void DecodeThread::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}
