#include "ffmpges.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
// 设备
//#include <libavdevice/avdevice.h>

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

FFmpges::FFmpges()
{

}

void printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode, errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}


/* check that a given sample format is supported by the encoder */
static int check_pix_fmt(const AVCodec *codec, enum AVPixelFormat pix_fmt)
{
    const enum AVPixelFormat *p = codec->pix_fmts;

    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pix_fmt)
            return 1;
        p++;
    }
    return 0;
}


static int encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
                   QFile *out)
{
    int ret;

    /* send the frame for encoding */
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        qDebug() << "Error sending the frame to the encoder";
        return ret;
    }

    /* read all the available output packets (in general there may be any
     * number of them */
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            // 继续
            return 0;
        else if (ret < 0) {
            qDebug() << "Error encoding audio frame";
            // 出错了退出
            return ret;
        }

//        fwrite(pkt->data, 1, pkt->size, output);
        out->write((char *)pkt->data, pkt->size);
        av_packet_unref(pkt);
    }
}

 void FFmpges::h264Encode(VideoEncodeSpec in, const char *out){
     // 文件
    QFile inFile(in.fileName);
    QFile outFile(out);

    // 返回值
    int ret = 0;

    // 编码器
    AVCodec *codec = nullptr;

    // 编码上下文
    AVCodecContext *ctx = nullptr;

    // 存放编码前的数据
    AVFrame *frame = nullptr;
    // 存放编码后的数据
    AVPacket *pkt = nullptr;
    AVDictionary *options = nullptr;

    int frameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);

    uint8_t *buf = ( uint8_t *)malloc(frameSize);

    // 创建编码器
    const char* encoderName = "libx264";
    codec = avcodec_find_encoder_by_name(encoderName);
    if (!codec){
        qDebug() << "encoder not found " << encoderName;
        return;
    }

    qDebug() << codec->name;

    // 创建编码上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx){
        qDebug() << "Could not allocate audio codec context";
        return;
    }

    // 检查输入数据的采样格式。libfdk_aac必须是16位整数
     /* check that the encoder supports sample_fmt input */

    if (!check_pix_fmt(codec, in.format)) {
        qDebug() << "Encoder does not support sample format " << av_get_sample_fmt_name(ctx->sample_fmt);
        goto end;
    }
    ctx->pix_fmt = in.format;
    // 设置输入参数
    ctx->width = in.width;
    ctx->height = in.height;
    ctx->time_base = {1, in.fps};

    // 设置输出参数
//    ctx->bit_rate = 48000;
//    ctx->profile = FF_PROFILE_AAC_HE_V2;

//    // 设置一些编码器特殊的属性
//    av_dict_set(&options,"vbr","1",0);

    // 打开编码器
    ret = avcodec_open2(ctx, codec, &options);
    if (ret < 0){
        qDebug() << "codec open error";
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)){
        qDebug() << "file open error:" << in.fileName;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)){
        qDebug() << "file open error:" << out;
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt){
        qDebug() << "could not allocate the packet";
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame){
        qDebug() << "Could not allocate audio frame";
        goto end;
    }

    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->pts = 0;

//    ret = av_frame_get_buffer(frame, 0);

     // 创建输入缓冲区方法一
//     ret = av_image_alloc(frame->data, frame->linesize, in.width, in.height, in.format,1);
    ret = av_image_fill_arrays(frame->data, frame->linesize, buf,in.format, in.width, in.height, 1);
    if (ret < 0) {
        qDebug() << "Could not allocate audio data buffers";
        goto end;
    }

    while ((ret = inFile.read((char *)frame->data[0], frameSize)) > 0) {
//        if (ret < frame->linesize[0]){
//            int bytes_per_sample = av_get_bytes_per_sample((AVSampleFormat)frame->format);
//            int channels = av_get_channel_layout_nb_channels(frame->channel_layout);
//            frame->nb_samples = ret/(  bytes_per_sample * channels );
//        }

        if(encode(ctx, frame, pkt, &outFile) < 0){
            goto end;
        }
        frame->pts++;
    }

    /* flush the encoder */
    encode(ctx, NULL, pkt, &outFile);

end:
    inFile.close();
    outFile.close();

    if (frame){
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
    }


    av_packet_free(&pkt);

    avcodec_free_context(&ctx);
}
