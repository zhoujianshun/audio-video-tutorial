#include "aduioresamplethread.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#import "ffmpges.h"

extern "C" {
// 设备
//#include <libavdevice/avdevice.h>

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

#ifdef Q_OS_MAC
    #define IN_FILE_NAME "/Users/zhoujianshun/Downloads/48000_f32le_1.pcm"
    #define OUT_FILE_PATH "/Users/zhoujianshun/Downloads/44100_s16le_2.pcm"

    #define OUT_FILE_PATH_NEW "/Users/zhoujianshun/Downloads/48000_f32le_1_new.pcm"
//    #define IN_FILE_NAME "/Users/zhoujianshun/Downloads/44100_s16le_2.pcm"
#elif Q_OS_WIN
    #define FORMATNAME "dshow"
    #define DEVICE_NAME ""
    #define FILE_NAME ""
#else

#endif


AduioResampleThread::AduioResampleThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AduioResampleThread::finished,
            this, &AduioResampleThread::deleteLater);
}

AduioResampleThread::~AduioResampleThread()
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

//void printErrorMessage(const char *message, int errorCode){
//    char errbuf[1024];
//    av_strerror(errorCode, errbuf, sizeof(errbuf));
//    qDebug() << message << errbuf << errorCode;
//}
void AduioResampleThread::run()
{
//    const char *inFileName = IN_FILE_NAME;
//    int64_t  in_ch_layout = AV_CH_LAYOUT_MONO;
//    AVSampleFormat  in_sample_fmt = AV_SAMPLE_FMT_FLT;
//    int  in_sample_rate = 48000;


//    /// 输出参数
//    const char *outFileName = OUT_FILE_PATH;
//    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO; // 立体声
//    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
//    int out_sample_rate = 44100;


//    FFmpges::resampleAudio(inFileName, in_ch_layout, in_sample_fmt, in_sample_rate,
//                           outFileName, out_ch_layout,out_sample_fmt,out_sample_rate);

//    FFmpges::resampleAudio(
//                           outFileName, out_ch_layout,out_sample_fmt,out_sample_rate,
//                OUT_FILE_PATH_NEW, in_ch_layout, in_sample_fmt, in_sample_rate);

    ResampleAudioSpec res1;
    res1.fileName = IN_FILE_NAME;
    res1.ch_layout = AV_CH_LAYOUT_MONO;
    res1.sample_fmt = AV_SAMPLE_FMT_FLT;
    res1.sample_rate = 48000;

    ResampleAudioSpec res2;
    res2.fileName = OUT_FILE_PATH;
    res2.ch_layout = AV_CH_LAYOUT_STEREO;
    res2.sample_fmt = AV_SAMPLE_FMT_S16;
    res2.sample_rate = 44100;

    ResampleAudioSpec res3;
    res3.fileName = "/Users/zhoujianshun/Downloads/48000_s32le_2.pcm";
    res3.ch_layout = AV_CH_LAYOUT_STEREO;
    res3.sample_fmt = AV_SAMPLE_FMT_S32;
    res3.sample_rate = 48000;

//    FFmpges::resampleAudio(res1, res3);
    FFmpges::resampleAudio(res3, res2);

}

//void run2()
//{
//    /// 输入参数
//    int64_t  in_ch_layout = AV_CH_LAYOUT_MONO;
//    AVSampleFormat  in_sample_fmt = AV_SAMPLE_FMT_FLT;
//    int  in_sample_rate = 48000;

//    uint8_t **in_data = nullptr;
//    int in_linesize = 0;
//    int in_nb_channels = 0;
//    int in_nb_samples = 1024;
//    QFile in_file(IN_FILE_NAME);
//    qint64 len = 0;

//    /// 输出参数
//    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO; // 立体声
//    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
//    int out_sample_rate = 44100;

//    uint8_t **out_data = nullptr;
//    int out_linesize = 0;
//    int out_nb_channels = 0;
//    int out_nb_samples = av_rescale_rnd(in_nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);
//    QFile out_file(OUT_FILE_PATH);
//    int out_bytes_per_sample;

//    // 返回值
//    int ret;

//    // 创建上下文
//    SwrContext *swrCtx = swr_alloc_set_opts(nullptr,
//                                            out_ch_layout, out_sample_fmt, out_sample_rate,
//                                            in_ch_layout, in_sample_fmt, in_sample_rate,
//                                            0, nullptr);
//    if (!swrCtx){
////        printErrorMessage("swr_alloc_set_opts error:",)
//        qDebug() << "swr_alloc_set_opts error";
//        goto end;
//    }
//    // 初始化上下文
//    ret = swr_init(swrCtx);
//    if (ret < 0){
//        printErrorMessage("swr_init error:", ret);
//        goto end;
//    }

//    // 创建缓冲区
//    // 输入缓冲区
//    in_nb_channels = av_get_channel_layout_nb_channels(in_ch_layout);
//    ret = av_samples_alloc_array_and_samples(&in_data, &in_linesize, in_nb_channels, in_nb_samples, in_sample_fmt, 1);
//    if (ret < 0){
//        printErrorMessage("输入缓冲区 error:", ret);
//        goto end;
//    }
//    // 输出缓冲区
//    out_nb_channels = av_get_channel_layout_nb_channels(out_ch_layout);
//    ret = av_samples_alloc_array_and_samples(&out_data, &out_linesize, out_nb_channels, out_nb_samples, out_sample_fmt, 1);
//    if (ret < 0){
//        printErrorMessage("输出缓冲区 error:", ret);
//        goto end;
//    }
//    // 打开文件
//    if(!in_file.open(QFile::ReadOnly)){
//        qDebug() << "open file error" << IN_FILE_NAME;
//        goto end;
//    }

//    if(!out_file.open(QFile::WriteOnly)){
//        qDebug() << "open file error" << OUT_FILE_PATH;
//        goto end;
//    }

//    out_bytes_per_sample = ( out_nb_channels * av_get_bytes_per_sample(out_sample_fmt));
//    // 重采样
//    while ((len = in_file.read((char*)*in_data, in_linesize)) > 0) {
//        // 读取的样本数量
//        int in_sample = len / (in_nb_channels * av_get_bytes_per_sample(in_sample_fmt));

//        // 输出的样本数量
//         ret = swr_convert(swrCtx,
//                    out_data, out_nb_samples,
//                    (const uint8_t **)in_data, in_sample);
//        if(ret <= 0){
//            printErrorMessage("swr_convert error:", ret);
//            break;
//        }
//        int outSamples = ret;
//        out_file.write((char *)*out_data, outSamples * out_bytes_per_sample);
//    }

//    // 清理输出缓冲区残留
//    while ((ret = swr_convert(swrCtx,
//                              out_data, out_nb_samples,
//                              nullptr, 0)) > 0) {
//        out_file.write((char *)*out_data, ret * out_bytes_per_sample);
//    }

//    // 关闭资源
//end:

////    fclose(dst_file);
//    in_file.close();
//    out_file.close();

//    if (in_data)
//        av_freep(&in_data[0]);
//    av_freep(&in_data);

//    if (out_data)
//        av_freep(&out_data[0]);
//    av_freep(&out_data);

//    swr_free(&swrCtx);

//}

