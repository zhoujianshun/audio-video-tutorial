#include "ffmpegs.h"

#include <QDebug>
#include <QFile>

extern "C"{
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

FFmpegs::FFmpegs()
{

}


void FFmpegs::convertRawFrame(RawVideoFrame &in, RawVideoFrame &out){

    SwsContext *ctx = nullptr;

    uint8_t *inData[4];
    uint8_t *outData[4];
    int inLinesizes[4];
    int outLinesizes[4];

    int ret = 0;

    int inFrameSize;
    int outFrameSize;


    ctx =  sws_getContext(in.width, in.height, in.format,
                          out.width, out.height, out.format,
                          SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!ctx){
        qDebug() << "sws_getContext error";
        goto end;
    }

    ret = av_image_alloc(inData, inLinesizes, in.width, in.height, in.format, 1);
    if (ret < 0){
        qDebug() << "av_image_alloc error" ;
        goto end;
    }

    ret = av_image_alloc(outData, outLinesizes, out.width, out.height, out.format, 1);
    if (ret < 0){
        qDebug() << "av_image_alloc error" ;
        goto end;
    }

    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    memcpy(inData[0], in.pixel , inFrameSize);


    sws_scale(ctx,
              inData, inLinesizes, 0, in.height,
              outData, outLinesizes);

    out.pixel = (char *)malloc(outFrameSize);
    memcpy(out.pixel, outData[0], outFrameSize);
    end:

    sws_freeContext(ctx);
    av_freep(&inData[0]);
    av_freep(&outData[0]);

}

void FFmpegs::convertRawVideo(RawVideoFile &in, RawVideoFile&out){

    SwsContext *ctx = nullptr;

    uint8_t *inData[4];
    uint8_t *outData[4];
    int inLinesizes[4];
    int outLinesizes[4];

    int ret = 0;

    int inFrameSize;
    int outFrameSize;

    QFile inFile(in.fileName);
    QFile outFile(out.fileName);

    ctx =  sws_getContext(in.width, in.height, in.format,
                          out.width, out.height, out.format,
                          SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!ctx){
        qDebug() << "sws_getContext error";
        goto end;
    }



    ret = av_image_alloc(inData, inLinesizes, in.width, in.height, in.format, 1);
    if (ret < 0){
        qDebug() << "av_image_alloc error" ;
        goto end;
    }

    ret = av_image_alloc(outData, outLinesizes, out.width, out.height, out.format, 1);
    if (ret < 0){
        qDebug() << "av_image_alloc error" ;
        goto end;
    }



    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "open inFile error";
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "open outFile error";
        goto end;
    }


    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    while ((inFile.read((char *)inData[0], inFrameSize)) == inFrameSize) {
        sws_scale(ctx,
                  inData, inLinesizes, 0, in.height,
                  outData, outLinesizes);
        outFile.write((char *)outData[0], outFrameSize);
    }

    end:

    sws_freeContext(ctx);

    av_freep(&inData[0]);
    av_freep(&outData[0]);

    inFile.close();
    outFile.close();
}
