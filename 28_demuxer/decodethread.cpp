#include "decodethread.h"

#include <QDebug>
#import "demuxer.h"

extern "C" {

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}



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
    VideoDecodeSpec vOut;
    vOut.fileName = "/Users/zhoujianshun/Downloads/yuv/3_.yuv";

    AudioDecodeSpec aOut;
    aOut.fileName = "/Users/zhoujianshun/Downloads/yuv/3_.pcm";



   Demuxer().demux("/Users/zhoujianshun/Downloads/yuv/3.mp4", aOut, vOut);

   qDebug() << vOut.fileName << vOut.width << vOut.height << av_get_pix_fmt_name(vOut.pixfmt) << vOut.fps;
   qDebug() << aOut.fileName << aOut.sampleRate << av_get_channel_layout_nb_channels(aOut.chLayout) << av_get_sample_fmt_name(aOut.fmt);

   qDebug() << "采集完毕";
}

void DecodeThread::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}
