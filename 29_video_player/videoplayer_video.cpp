#include "videoplayer.h"

#include <QDebug>
#include <thread>

extern "C"{
    #include <libavutil/imgutils.h>
}

int VideoPlayer::initVideoSws(){

    _vSwsOutSpec.width = _video_dec_ctx->width;
    _vSwsOutSpec.height = _video_dec_ctx->height;
    _vSwsOutSpec.format = AV_PIX_FMT_RGB24;
    _vSwsCtx =  sws_getContext(_video_dec_ctx->width, _video_dec_ctx->height, _video_dec_ctx->pix_fmt,
                          _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.format,
                          SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!_vSwsCtx){
        qDebug() << "sws_getContext error";
        return -1;
    }

    _vSwsInFrame = av_frame_alloc();
    if (!_vSwsInFrame) {
        qDebug() << "Could not allocate _vSwsInFrame";
        onFailed();
        return -1;
    }

    _vSwsOutFrame = av_frame_alloc();
    if (!_vSwsOutFrame) {
        qDebug() << "Could not allocate _vSwsOutFrame";
        onFailed();
        return -1;
    }

    int ret = av_image_alloc(_vSwsOutFrame->data, _vSwsOutFrame->linesize, _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.format, 1);
    if (ret < 0){
        qDebug() << "av_image_alloc error" ;
        return ret;
    }
    //
    // 图片大小. 也可以使用av_image_get_buffer_size()计算
    _imageSize = ret;
    return 0;
}

int VideoPlayer::initVideoInfo() {

    initVideoSws();
    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt){
    _vCondMutex->lock();
    _vPktList->push_back(pkt);
    _vCondMutex->signal();
    _vCondMutex->unlock();
}

void VideoPlayer::clearVideoPktList(){
    _vCondMutex->lock();
    _vPktList->clear();
    for(AVPacket &pkt: *_vPktList){
        av_packet_unref(&pkt);
    }
    _vCondMutex->unlock();
}

void VideoPlayer::freeVideo()
{
    _video_stream_idx = -1;
    _vSeekTime = -1;

    if (_vSwsCtx){
        sws_freeContext(_vSwsCtx);
        _vSwsCtx = nullptr;
    }

    av_frame_free(&_aSwrInFrame);
    if(_vSwsOutFrame){
        av_free(_vSwsOutFrame->data[0]);
        av_frame_free(&_vSwsOutFrame);
    }

    avcodec_free_context(&_video_dec_ctx);
    clearVideoPktList();

    _video_stream = nullptr;
    _vTime = 0;
    _vCanFree = false;

//    emit frameDecoded(this, nullptr, _vSwsOutSpec);
}

void VideoPlayer::decodeVideo(){

    while (true) {

        if (_state == Paused && _vSeekTime == -1){
            continue;
        }
        if ( _state == Stoped){
            _vCanFree = true;
            qDebug() << "decodeVideo break" ;
            break;
        }
        _vCondMutex->lock();

        if (_vPktList->empty()){
            _vCondMutex->unlock();
            continue;
        }

        AVPacket pkt = _vPktList->front();
        _vPktList->pop_front();
        _vCondMutex->unlock();

        if ( pkt.pts != AV_NOPTS_VALUE) {
            _vTime = av_q2d(_video_stream->time_base) * pkt.dts;
        }

        /// 视频解码
        int ret = 0;
        // submit the packet to the decoder
        ret = avcodec_send_packet(_video_dec_ctx, &pkt);
        // 释放pkt
        av_packet_unref(&pkt);

        if (ret < 0) {
            qDebug() << "Error submitting a packet for decoding" << av_err2str(ret);
            continue;
        }

        while (true) {

            if(_state == Stoped){
                break;
            }

            ret = avcodec_receive_frame(_video_dec_ctx, _vSwsInFrame);
            if (ret < 0) {
                // those two return values are special and mean there is no output
                // frame available, but there were no errors during decoding
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    break;

                qDebug() << "Error during decoding " << av_err2str(ret);
                continue;
            }

            // 发现视频帧的时间早于seekTime，则丢弃视频帧
            if (_vSeekTime >= 0){
                if ( _vTime < _vSeekTime){
                    continue;
                }else{
                    _vSeekTime = -1;
                }
            }

            // 像素格式转换
            sws_scale(_vSwsCtx,
                      _vSwsInFrame->data, _vSwsInFrame->linesize, 0, _video_dec_ctx->height,
                      _vSwsOutFrame->data, _vSwsOutFrame->linesize);
//            qDebug() << _vSwsOutFrame->data[0];

            if (_audio_stream){
                // 音视频同步
                while (_vTime > _aTime && _state == Playing) {
    //                SDL_Delay(5);
                }
            }else{
                // 如果没有视频需要单独处理
                // TODO
                SDL_Delay(33);
            }

            // 像素格式转换完成之后，拷贝一份出来
            uint8_t *data = (uint8_t *)malloc(_imageSize);
            memcpy(data, _vSwsOutFrame->data[0], _imageSize);
            emit frameDecoded(this, data, _vSwsOutSpec);
        }

    }
    qDebug() << "decodeVideo end" ;
}
