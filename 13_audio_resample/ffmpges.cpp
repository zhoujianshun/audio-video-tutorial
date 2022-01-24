#include "ffmpges.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
// 设备
//#include <libavdevice/avdevice.h>

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

FFmpges::FFmpges()
{

}

void printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode, errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}

void FFmpges::resampleAudio(ResampleAudioSpec inAudio, ResampleAudioSpec outAudio){
    resampleAudio(inAudio.fileName, inAudio.ch_layout, inAudio.sample_fmt, inAudio.sample_rate,
                           outAudio.fileName, outAudio.ch_layout, outAudio.sample_fmt, outAudio.sample_rate);
}




void FFmpges::resampleAudio(const char *inFileName, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate,
                             const char *outFileName, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate)
{

    /// 输入参数


    uint8_t **in_data = nullptr;
    int in_linesize = 0;
    int in_nb_channels = 0;
    int in_nb_samples = 1024;
    QFile in_file(inFileName);
    qint64 len = 0;

    /// 输出参数


    uint8_t **out_data = nullptr;
    int out_linesize = 0;
    int out_nb_channels = 0;
    int out_nb_samples = av_rescale_rnd(in_nb_samples, out_sample_rate, in_sample_rate, AV_ROUND_UP);
    QFile out_file(outFileName);
    int out_bytes_per_sample;

    // 返回值
    int ret;

    // 创建上下文
    SwrContext *swrCtx = swr_alloc_set_opts(nullptr,
                                            out_ch_layout, out_sample_fmt, out_sample_rate,
                                            in_ch_layout, in_sample_fmt, in_sample_rate,
                                            0, nullptr);
    if (!swrCtx){
//        printErrorMessage("swr_alloc_set_opts error:",)
        qDebug() << "swr_alloc_set_opts error";
        goto end;
    }
    // 初始化上下文
    ret = swr_init(swrCtx);
    if (ret < 0){
        printErrorMessage("swr_init error:", ret);
        goto end;
    }

    // 创建缓冲区
    // 输入缓冲区
    in_nb_channels = av_get_channel_layout_nb_channels(in_ch_layout);
    ret = av_samples_alloc_array_and_samples(&in_data, &in_linesize, in_nb_channels, in_nb_samples, in_sample_fmt, 1);
    if (ret < 0){
        printErrorMessage("输入缓冲区 error:", ret);
        goto end;
    }
    // 输出缓冲区
    out_nb_channels = av_get_channel_layout_nb_channels(out_ch_layout);
    ret = av_samples_alloc_array_and_samples(&out_data, &out_linesize, out_nb_channels, out_nb_samples, out_sample_fmt, 1);
    if (ret < 0){
        printErrorMessage("输出缓冲区 error:", ret);
        goto end;
    }
    // 打开文件
    if(!in_file.open(QFile::ReadOnly)){
        qDebug() << "open file error" << inFileName;
        goto end;
    }

    if(!out_file.open(QFile::WriteOnly)){
        qDebug() << "open file error" << outFileName;
        goto end;
    }

    out_bytes_per_sample = ( out_nb_channels * av_get_bytes_per_sample(out_sample_fmt));
    // 重采样
    while ((len = in_file.read((char*)*in_data, in_linesize)) > 0) {
        // 读取的样本数量
        int in_sample = len / (in_nb_channels * av_get_bytes_per_sample(in_sample_fmt));

        // 输出的样本数量
         ret = swr_convert(swrCtx,
                    out_data, out_nb_samples,
                    (const uint8_t **)in_data, in_sample);
        if(ret <= 0){
            printErrorMessage("swr_convert error:", ret);
            break;
        }
        int outSamples = ret;
        out_file.write((char *)*out_data, outSamples * out_bytes_per_sample);
    }

    // 清理输出缓冲区残留
    while ((ret = swr_convert(swrCtx,
                              out_data, out_nb_samples,
                              nullptr, 0)) > 0) {
        out_file.write((char *)*out_data, ret * out_bytes_per_sample);
    }

    // 关闭资源
end:

//    fclose(dst_file);
    in_file.close();
    out_file.close();

    if (in_data)
        av_freep(&in_data[0]);
    av_freep(&in_data);

    if (out_data)
        av_freep(&out_data[0]);
    av_freep(&out_data);

    swr_free(&swrCtx);

 }
