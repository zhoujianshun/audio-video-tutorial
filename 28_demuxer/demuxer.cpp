#include "demuxer.h"

#include <QDebug>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}


Demuxer::Demuxer()
{

}

int Demuxer::output_video_frame(AVFrame *frame){
    /* copy decoded frame to destination buffer:
         * this is required since rawvideo expects non aligned data */
     av_image_copy(_video_dst_data, _video_dst_linesize,
                      (const uint8_t **)(frame->data), frame->linesize,
                      _video_dec_ctx->pix_fmt, _video_dec_ctx->width, _video_dec_ctx->height);
    vFile->write((char *)_video_dst_data[0], _video_dst_bufsize);
     return 0;
}

int Demuxer::output_audio_frame(AVFrame *frame){
    //
    if (av_sample_fmt_is_planar(_audio_dec_ctx->sample_fmt)){
        // aac可尝试用f32le播放
        for(int i = 0 ; i < frame->nb_samples ; i++){
            for(int j = 0; j < frame->channels ; j++){
                aFile->write((char*)(frame->data[j] + i*_sampleSize), _sampleSize);
            }
        }
    }else{
        // 非planner
        aFile->write((char *)frame->data[0], frame->nb_samples * _sampleFrameSize);
    }
    return 0;
}

int Demuxer::decode_packet(AVCodecContext *dec, const AVPacket *pkt){
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
           qDebug() << "Error submitting a packet for decoding" << av_err2str(ret);
           return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, _frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            qDebug() << "Error during decoding " << av_err2str(ret);
            return ret;
        }

        // write thre frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO){
            ret = output_video_frame(_frame);
        }else {
            ret = output_audio_frame(_frame);
        }

        av_frame_unref(_frame);
        if (ret < 0){
            return ret;
        }
    }

    return 0;
}

int Demuxer::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type){
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0){
        qDebug() << "Could not find" << av_get_media_type_string(type);
        return ret;
    }else{
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec){
            qDebug() << "Failed to find " << av_get_media_type_string(type) << " codec";
            return AVERROR(EINVAL);
        }

        if (dec->id == AV_CODEC_ID_AAC){
            dec = avcodec_find_decoder_by_name("libfdk_aac");
            if (!dec){
                qDebug() << "Failed to find libfdk_aac  codec";
                return AVERROR(EINVAL);
            }
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx){
            qDebug() << "Failed to allocate the " << av_get_media_type_string(type) << " codec context";
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
        if (ret < 0){
            qDebug() << "Failed to copy " << av_get_media_type_string(type) << " codec parameters to decoder context";
            return ret;
        }

        /* Init the decoders */
        ret = avcodec_open2(*dec_ctx, dec, &opts);
        if (ret < 0){
            qDebug() << "Failed to open " << av_get_media_type_string(type) << " codec";
            return ret;
        }

        *stream_idx = stream_index;
    }

    return 0;
}


void Demuxer::demux(const char *inFilename, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut){
    int ret = 0;

    /* open input file, and allocate format context */
    ret = avformat_open_input(&_fmt_ctx, inFilename, NULL, NULL);
    if (ret < 0){
        qDebug() << "Could not open source file" << inFilename;
        return;
    }
    /* retrieve stream information */
    ret = avformat_find_stream_info(_fmt_ctx, NULL);
    if (ret < 0){
        qDebug() << "Could not find stream information";
        return;
    }

    // 获取视频数据流，解码上下文
    ret = open_codec_context(&_video_stream_idx, &_video_dec_ctx, _fmt_ctx, AVMEDIA_TYPE_VIDEO);
    if (ret >= 0) {
        _video_stream = _fmt_ctx->streams[_video_stream_idx];
        vOut.width = _video_dec_ctx->width;
        vOut.height = _video_dec_ctx->height;
        vOut.pixfmt = _video_dec_ctx->pix_fmt;
        AVRational rat = av_guess_frame_rate(_fmt_ctx, _fmt_ctx->streams[_video_stream_idx], NULL);
        vOut.fps = rat.num / rat.den;

        // 创建图片
        ret = av_image_alloc(_video_dst_data, _video_dst_linesize,
                                   _video_dec_ctx->width, _video_dec_ctx->height, _video_dec_ctx->pix_fmt, 1);

        if (ret < 0) {
            qDebug() << "Could not allocate raw video buffer";
            goto end;
        }
        // 图片大小
        _video_dst_bufsize = ret;
    }

    // 获取音频数据流，解码上下文
    ret = open_codec_context(&_audio_stream_idx, &_audio_dec_ctx, _fmt_ctx, AVMEDIA_TYPE_AUDIO);
    if (ret >= 0) {
        _audio_stream = _fmt_ctx->streams[_audio_stream_idx];
        aOut.fmt = _audio_dec_ctx->sample_fmt;
        aOut.sampleRate = _audio_dec_ctx->sample_rate;
        aOut.chLayout =_audio_dec_ctx->channel_layout;

        _sampleSize =  av_get_bytes_per_sample(_audio_dec_ctx->sample_fmt);
        _sampleFrameSize = _sampleSize * _audio_dec_ctx->channels;
    }


    /* dump input information to stderr */
    av_dump_format(_fmt_ctx, 0, inFilename, 0);

    if (!_audio_stream && !_video_stream) {
        qDebug() << "Could not find audio or video stream in the input, aborting";
        goto end;
   }

    // 打开输出文件
    vFile = new QFile();
    vFile->setFileName(vOut.fileName);
    if (!vFile->open(QFile::WriteOnly)){
        qDebug() << "open file error " << vOut.fileName;
        goto end;
    }

    aFile = new QFile();
    aFile->setFileName(aOut.fileName);
    if (!aFile->open(QFile::WriteOnly)){
        qDebug() << "open file error " << aOut.fileName;
        goto end;
    }

    // 创建frame，用于存放解码后的数据
    _frame = av_frame_alloc();
    if (!_frame) {
        qDebug() << "Could not allocate frame";
        goto end;
    }
    // 创建pkt，用于存放解码前的数据
    _pkt = av_packet_alloc();
    if (!_pkt) {
        qDebug() << "Could not allocate packet";
        goto end;
    }

     /* read frames from the file */
    while (av_read_frame(_fmt_ctx, _pkt) >= 0) {
        if (_pkt->stream_index == _video_stream_idx) {
            // video
            decode_packet(_video_dec_ctx, _pkt);
        } else if (_pkt->stream_index == _audio_stream_idx) {
            // audio
            decode_packet(_audio_dec_ctx, _pkt);
        }

        av_packet_unref(_pkt);
        if (ret < 0)
            break;
    }
    // 刷新缓存区
    decode_packet(_video_dec_ctx, NULL);
    decode_packet(_audio_dec_ctx, NULL);
end:
    avcodec_free_context(&_video_dec_ctx);
    avcodec_free_context(&_audio_dec_ctx);
    avformat_close_input(&_fmt_ctx);
    if (aFile)
        aFile->close();
    if (vFile)
        vFile->close();
    av_packet_free(&_pkt);
    av_frame_free(&_frame);
    av_free(_video_dst_data[0]);
}
