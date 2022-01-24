#include "videoplayer.h"

#include <QDebug>

extern "C" {

//#include <libavutil/imgutils.h>
//#include <libavutil/samplefmt.h>
//#include <libavutil/timestamp.h>
//#include <libavformat/avformat.h>
}



void VideoPlayer::setVolumn(int volumn){
    _volumn = volumn;
}

int VideoPlayer::getVolumn(){
    return _volumn;
}

void VideoPlayer::setMute(bool mute)
{
    _mute = mute;
}

bool VideoPlayer::getMute(){
    return _mute;
}

int VideoPlayer::initAudioInfo(){
    int ret = initAudioSwr();
    if (ret < 0){
        qDebug() << "initSwr error";
        onFailed();
        return ret;
    }

    ret = initSDL();
    if (ret < 0){
        qDebug() << "initSDL error";
        onFailed();
        return ret;
    }

    return 0;
}

void VideoPlayer::freeAudio()
{
    _aSwrOutIdx = 0;
    _aSwrOutSize = 0;
    _audio_stream_idx = -1;
    _aSeekTime = -1;

//    _vCondMutex->signal();

    if (_audio_dec_ctx){
        avcodec_free_context(&_audio_dec_ctx);
    }

    clearAudioPktList();

    av_frame_free(&_aSwrInFrame);
    if(_aSwrOutFrame){
        av_free(_aSwrOutFrame->data[0]);
        av_frame_free(&_aSwrOutFrame);
    }
    swr_free(&_aSwrCtx);

     _audio_stream = nullptr;

    // 停止播放
    SDL_PauseAudio(1);
    SDL_CloseAudio();

    _aTime = 0;
    _aCanFree = false;
}



void VideoPlayer::addAudioPkt(AVPacket &pkt){
    _aCondMutex->lock();
    _aPktList->push_back(pkt);
    _aCondMutex->signal();
    _aCondMutex->unlock();
}

void VideoPlayer::clearAudioPktList()
{
    _aCondMutex->lock();
    _aPktList->clear();
    for(AVPacket &pkt: *_aPktList){
        av_packet_unref(&pkt);
    }
    _aCondMutex->unlock();
}

int VideoPlayer::initSDL()
{
    // 音频参数
    SDL_AudioSpec spec;
    // 采样率
    spec.freq = 44100;
    // 声道数
    spec.channels = 2;
    // 采样格式（f32le）
    spec.format = AUDIO_S16LSB;
    // 音频缓冲区的样本数量（必须是2的幂）
    spec.samples = 512; // 1024个样本
    // 回调，拉取数据
    spec.callback = sdlAudioCallbackFunc;
    spec.userdata = this;

    // 打开设备
    if(SDL_OpenAudio(&spec, nullptr) != 0){
        qDebug() << "Unable to open SDL audio: " << SDL_GetError();
//        SDL_Quit();
        return -1;
    }
    return 0;
}

int VideoPlayer::initAudioSwr()
{
    _aSwrInSpec.sample_fmt = _audio_dec_ctx->sample_fmt;
    _aSwrInSpec.sample_rate = _audio_dec_ctx->sample_rate;
    _aSwrInSpec.ch_layout = _audio_dec_ctx->channel_layout;
    _aSwrInSpec.channels = _audio_dec_ctx->channels;


    _aSwrOutSpec.sample_fmt = AV_SAMPLE_FMT_S16;
    _aSwrOutSpec.sample_rate = 44100;
    _aSwrOutSpec.ch_layout = AV_CH_LAYOUT_STEREO;
    _aSwrOutSpec.channels = 2;
    _aSwrOutSpec.bytesPerSampleFrame = _aSwrOutSpec.channels * av_get_bytes_per_sample(_aSwrOutSpec.sample_fmt);



    // 返回值
    int ret;

    // 创建上下文
    _aSwrCtx = swr_alloc_set_opts(nullptr,
                                            // 输出参数
                                            _aSwrOutSpec.ch_layout, _aSwrOutSpec.sample_fmt, _aSwrOutSpec.sample_rate,
                                            // 输入参数
                                            _aSwrInSpec.ch_layout, _aSwrInSpec.sample_fmt, _aSwrInSpec.sample_rate,
                                            0, nullptr);
    if (!_aSwrCtx){
//        printErrorMessage("swr_alloc_set_opts error:",)
        qDebug() << "swr_alloc_set_opts error";
        return -1;
    }
    // 初始化上下文
    ret = swr_init(_aSwrCtx);
    if (ret < 0){
//        printErrorMessage("swr_init error:", ret);
        qDebug() << "swr_init error:" << ret;
        return ret;
    }


    // 创建frame，用于存放解码后的数据
    _aSwrInFrame = av_frame_alloc();
    if (!_aSwrInFrame) {
        qDebug() << "Could not allocate frame";
        return -1;
    }

    // 创建frame，用于存放解码后的数据
    _aSwrOutFrame = av_frame_alloc();
    if (!_aSwrOutFrame) {
        qDebug() << "Could not allocate frame";
        return -1;
    }

//    _aSwrOutFrame->data[0] = malloc(4096* _aSwrOutSpec.bytesPerSampleFrame);
    ret = av_samples_alloc(_aSwrOutFrame->data,
                           _aSwrOutFrame->linesize,
                           _aSwrOutSpec.channels,
                           8192,
                           _aSwrOutSpec.sample_fmt,
                           1);
    if (ret < 0){
        qDebug() << "Could not av_samples_alloc";
        return -1;
    }
    return 0;
}

void VideoPlayer::sdlAudioCallbackFunc(void *userdata, Uint8 * stream,
                      int len)
{

    VideoPlayer *player = (VideoPlayer*)userdata;
    player->sdlAudioCallback(stream, len);
}

void VideoPlayer::sdlAudioCallback(Uint8 *stream, int len)
{
    /// 填充stream

    // 清空stream
    SDL_memset(stream, 0, len);

    while (len > 0) {

        if (_state == Paused){
            break;
        }

        if(_state == Stoped){
            _aCanFree = true;
            break;
        }

        // 当重采样的pcm数据读取完毕后，重新读取缓冲区
        if (_aSwrOutIdx >= _aSwrOutSize ){
            // 读取新的PCM数据
           _aSwrOutSize = decodeAudio();
           // 重置
           _aSwrOutIdx = 0;
           // 没读取到数据，就静音处理
           if (_aSwrOutSize <= 0){
               _aSwrOutSize = 1024;
               memset(_aSwrOutFrame->data[0], 0, _aSwrOutSize);
           }
        }

        // 计算填充数据的长度
        int fillLen = std::min(_aSwrOutSize - _aSwrOutIdx, len);
        // 计算音量
        int vol = _mute ? 0 : (1.0 * _volumn / Max) * SDL_MIX_MAXVOLUME;
        // 填充sdl缓冲区
        SDL_MixAudio(stream, (Uint8 *)(_aSwrOutFrame->data[0] + _aSwrOutIdx), fillLen, vol);

        // 移动偏移量
        len -= fillLen;
        _aSwrOutIdx += fillLen;
        stream += fillLen;

//        qDebug() << _aSwrOutSize;
    }

}

int VideoPlayer::decodeAudio()
{
    _aCondMutex->lock();
    if (_aPktList->empty() || _state == Stoped) {

        _aCondMutex->unlock();
        return 0;
    }

//    while (_aPktList->empty()) {

//         _aCondMutex->wait();
////         if (_state == Stoped){
////             return 0;
////         }
//    }

    AVPacket pkt = _aPktList->front();
    _aPktList->pop_front();
    _aCondMutex->unlock();



    // 获取当前pkt的时间，用于音视频同步，和显示进度
    if ( pkt.pts != AV_NOPTS_VALUE) {
        _aTime = av_q2d(_audio_stream->time_base) * pkt.pts;
        if (_aTime < _aSeekTime){
            return 0;
        }
        qDebug() << "_aTime" <<_aTime;
        emit timeChanged(this);
    }

    /// 1.音频解码
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(_audio_dec_ctx, &pkt);
    // 释放pkt
    av_packet_unref(&pkt);

    if (ret < 0) {
        qDebug() << "Error submitting a packet for decoding" << av_err2str(ret);
        return ret;
    }

    ret = avcodec_receive_frame(_audio_dec_ctx, _aSwrInFrame);
    if (ret < 0) {
        // those two return values are special and mean there is no output
        // frame available, but there were no errors during decoding
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
            return 0;

        qDebug() << "Error during decoding " << av_err2str(ret);
        return ret;
    }

    /// 2.重采样
    // 计算输出样本数量
    _out_nb_samples = av_rescale_rnd(_aSwrOutSpec.sample_rate, _aSwrInFrame->nb_samples, _aSwrInSpec.sample_rate, AV_ROUND_UP);
    // 输出的样本数量
     ret = swr_convert(_aSwrCtx,
                _aSwrOutFrame->data, _out_nb_samples,
                (const uint8_t **)_aSwrInFrame->data, _aSwrInFrame->nb_samples);

//    qDebug() << _aSwrInFrame->sample_rate <<  _aSwrInFrame->channels << av_get_sample_fmt_name((AVSampleFormat)_aSwrInFrame->format);

    // 计算数据大小：样本数量x每个样本帧的大小
    return ret*_aSwrOutSpec.bytesPerSampleFrame;
}


