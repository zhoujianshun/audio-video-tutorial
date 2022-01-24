#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
// 设备
#include <libavdevice/avdevice.h>
// 格式
#include <libavformat/avformat.h>
// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_MAC
    #define FORMATNAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILE_NAME "/Users/zhoujianshun/Downloads/out.pcm"
    #define FILE_PATH "/Users/zhoujianshun/Downloads/"
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

void showSpec(AVFormatContext *context)
{
    AVStream *stream = context->streams[0];
    AVCodecParameters *params = stream->codecpar;

    qDebug() << "比特率" << params->bit_rate;
    qDebug() << "声道数" << params->channels;
    qDebug() << "采样率" << params->sample_rate;
    qDebug() << "采样格式" << params->format;
    qDebug() << "frame_size" << params->frame_size;
    qDebug() << "codec_id" << params->codec_id;

    qDebug() << "bytes_per_sample" << av_get_bytes_per_sample((AVSampleFormat)params->format);

}


void AudioThread::run()
{
    // 查看所有设备
    // ffmpeg -list_devices true -f avfoundation -i dummy 查看avfoundation可用的设备
    // 查看适用于avfoundation的参数
    // ffmpeg -h demuxer=avfoundation

    const char* formatName = FORMATNAME;
    /// 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(formatName);

    if (!fmt){
        qDebug() << "获取输入格式失败" << formatName;
        return;
    }
    /// 打开设备

    // 格式上下文，用来操作设备
    AVFormatContext *context = nullptr;
    // 设备名称
    const char* deviceName = DEVICE_NAME;
    qDebug() << "打开设备1";

    int ret = avformat_open_input(&context, deviceName, fmt, nullptr);
    qDebug() << "打开设备2";
    if (ret < 0){
        this->printErrorMessage("打开设备失败",ret);
        return;
    }
    showSpec(context);
    /// 采集数据
     qDebug() << "打开文件";
     QString fileName = FILE_PATH;
     fileName += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
     fileName += ".pcm";
    // 打开文件
//    const char* fileName = FILE_NAME;
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)){
        // 文件打开失败
        qDebug() << "文件打开失败" << fileName;
        avformat_close_input(&context);
        return;
    }
    qDebug() << "开始采集";

    emit this->startSignal();
    // 数据包
    // AVPacket pkt;
    AVPacket *pkt = av_packet_alloc();
    // 采集数据
    while (!this->isInterruptionRequested()) {

//        ret = av_read_frame(context, &pkt);
        ret = av_read_frame(context, pkt);
        if (ret == AVERROR(EAGAIN)){


//            this->printErrorMessage("av_read_frame error",ret);
//            sleep(1);
            continue;
        }else if (ret < 0) {
//            this->printErrorMessage("av_read_frame error",ret);
            break;
        }else{
//            file.write((const char*)pkt.data, pkt.size);
            file.write((const char*)pkt->data, pkt->size);
        }

        av_packet_unref(pkt);
    }
    emit this->endSignal();
    // 关闭文件
    file.close();

    // 释放资源
    av_packet_free(&pkt);
    avformat_close_input(&context);

    qDebug() << "采集完毕";
}

void AudioThread::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}
