#include "audiowavthread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

#include "ffmpegs.h"

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
//    #define FILE_NAME "/Users/zhoujianshun/Downloads/out.pcm"
    #define FILE_PATH "/Users/zhoujianshun/Downloads/"
#elif Q_OS_WIN
    #define FORMATNAME "dshow"
    #define DEVICE_NAME ""
    #define FILE_NAME ""
#else

#endif


AudioWavThread::AudioWavThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioWavThread::finished,
            this, &AudioWavThread::deleteLater);
}

AudioWavThread::~AudioWavThread()
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


void AudioWavThread::run()
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
//     QString wavFilePath = fileName + ".wav";
     fileName += ".wav";
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


    // 获取输入流
   AVStream *stream = context->streams[0];
   // 获取音频参数
   AVCodecParameters *params = stream->codecpar;
    // 写入wav文件头
    WAVHeader header;
    header.sampleRate = params->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(params->codec_id);
    header.numChannels = params->channels;
    if (params->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = AUDIO_FORMAT_FLOAT;
    }

    header.blockAlign = (header.bitsPerSample * header.numChannels) >> 3;
    header.byteRate = header.blockAlign * header.sampleRate;


    file.write((const char *)&header, sizeof (header));

    emit this->startSignal();
    // 数据包
    unsigned long long size = 0;
    AVPacket *pkt = av_packet_alloc();
    // 采集数据
    while (!this->isInterruptionRequested()) {
        ret = av_read_frame(context, pkt);
        if (ret == AVERROR(EAGAIN)){

            continue;
        }else if (ret < 0) {
            //            this->printErrorMessage("av_read_frame error",ret);
            break;
        }else{
            // 写入文件
            file.write((const char*)pkt->data, pkt->size);
            // 计算录音时常
            size += pkt->size;
//            qDebug() << (size *1000) / (header.byteRate);
            emit this->timeChanged((size *1000) / (header.byteRate));
        }

        av_packet_unref(pkt);
    }
    emit this->endSignal();


    // 释放资源
    av_packet_free(&pkt);


    qDebug() << "采集完毕";


   header.dataChunkDataSize = file.size() - sizeof (WAVHeader);
   header.riffChunkDataSize = file.size() - sizeof (header.riffChunkId) - sizeof (header.riffChunkDataSize);

   file.seek(sizeof (WAVHeader) - sizeof (header.dataChunkDataSize));
   file.write((const char*)&header.dataChunkDataSize, sizeof (header.dataChunkDataSize));

   file.seek(sizeof (header.riffChunkId));
   file.write((const char*)&header.riffChunkDataSize, sizeof (header.riffChunkDataSize));
   // 关闭文件
   file.close();

    avformat_close_input(&context);
    qDebug() << "全部完毕";
//    FFmpegs::pcm2wav("1", "2");
}

void AudioWavThread::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}
