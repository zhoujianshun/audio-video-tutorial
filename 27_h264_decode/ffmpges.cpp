#include "ffmpges.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {

// 工具（比如错误处理）
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

#define INBUF_SIZE 4096

FFmpges::FFmpges()
{

}

static void printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode, errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}

static int decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
                   QFile &outFile)
{
    char buf[1024];
    int ret;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        printErrorMessage("Error submitting the packet to the decoder", ret);
        //        qDebug() << "Error submitting the packet to the decoder";
        //        exit(1);
        return ret;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
    ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret < 0) {
            qDebug() << "Error during decoding";
            return ret;
    }
//    int imageSize = av_image_get_buffer_size(dec_ctx->pix_fmt, dec_ctx->width, dec_ctx->height,1);
    outFile.write((char *) frame->data[0], frame->linesize[0] * frame->height);
    outFile.write((char *) frame->data[1], frame->linesize[1] * frame->height >> 1);
    outFile.write((char *) frame->data[2], frame->linesize[2] * frame->height >> 1);

//    qDebug() << frame->linesize[0] << frame->linesize[1] << frame->linesize[2];
    }

    return 0;
}

///decode
void FFmpges::h264Decode(const char* in, VideoDecodeSpec &out)
{

    const AVCodec *codec;
    AVCodecContext *ctx= NULL;
    AVCodecParserContext *parser = NULL;

    QFile inFile(in);
    QFile outFile(out.fileName);

    AVPacket *pkt;
    // 保存解码后的数据
    AVFrame *decoded_frame = NULL;

    char inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *data;
    // 每次文件读取的长度
    size_t   data_size;
    int ret;
    int atEnd = 0;


    enum AVPixelFormat pixFmt;
//    int n_channels = 0;
    const char *fmt;

    //  创建解码器
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec){
        qDebug() << "Codec not found:" << "AV_CODEC_ID_H264";
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt){
        qDebug() << "could not allocate the packet";
        goto end;
    }

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    parser = av_parser_init(codec->id);
    if (!parser){
        qDebug() << "Parser not found";
        goto end;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx){
        qDebug() << "Could not allocate audio codec context";
        goto end;
    }

    // 打开解码器
    ret = avcodec_open2(ctx, codec, NULL);
    if (ret < 0){
         qDebug() << "Could not open codec";
         goto end;
    }

    decoded_frame = av_frame_alloc();
    if (!decoded_frame){
        qDebug() << "Could not allocate audio frame";
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)){
        qDebug() << "Could not open" << in;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)){
        qDebug() << "Could not open" << in;
        goto end;
    }

//    data = inbuf;
//    data_size = inFile.read(inbuf,INBUF_SIZE);

    do {
        data_size =  inFile.read(inbuf, INBUF_SIZE);
        atEnd = !data_size;
        data = inbuf;
        while (data_size > 0 || atEnd) {
            // 解析器解析
            ret = av_parser_parse2(parser, ctx,
                                   &pkt->data,
                                   &pkt->size,
                                   (const uint8_t *)data,
                                   data_size,
                                   AV_NOPTS_VALUE,
                                   AV_NOPTS_VALUE,
                                   0);
            if (ret < 0){
                qDebug() << "Error while parsing";
                goto end;
            }

            data += ret;
            data_size -= ret;

            if (pkt->size){
                ret = decode(ctx, pkt, decoded_frame, outFile);
                if (ret < 0){
                    qDebug() << "decode error";
                    goto end;
                }
            }

            if (atEnd){
                break;
            }

        }
    }while (!atEnd);


    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(ctx, NULL, decoded_frame, outFile);

    pixFmt = ctx->pix_fmt;
    out.pixfmt = pixFmt;
    out.width = ctx->width;
    out.height = ctx->height;
    out.fps = ctx->framerate.den;

    qDebug() << "decode complete" << av_get_pix_fmt_name(pixFmt) << out.width << out.height <<  out.fps;

end:
    inFile.close();
    outFile.close();

    av_packet_free(&pkt);
    av_frame_free(&decoded_frame);
    av_parser_close(parser);
    avcodec_free_context(&ctx);

}


