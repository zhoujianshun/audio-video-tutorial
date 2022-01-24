#include "videoplayer.h"

#include <thread>
#include <QDebug>

#define AUDIO_MAX_PKT_SIZE 1000
#define VIDEO_MAX_PKT_SIZE 500

VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{
    // 初始化子系统
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        qDebug() << "Unable to initialize SDL: " << SDL_GetError();
        return;
    }

    _aPktList = new std::list<AVPacket>();
    _vPktList = new std::list<AVPacket>();

    _aCondMutex = new CondMutex();
    _vCondMutex = new CondMutex();
}

VideoPlayer::~VideoPlayer(){
    disconnect();
    stop();
    delete _aPktList;
    delete _vPktList;

    delete _aCondMutex;
    delete _vCondMutex;

    SDL_Quit();
}

void VideoPlayer::free()
{
    qDebug() << "start free";
    while (_audio_stream && !_aCanFree);
  qDebug() << "_aCanFree";
    while (_video_stream && !_vCanFree);
  qDebug() << "_vCanFree";
    while (!_fmtCtxCanFree);
  qDebug() << "end free";
    avformat_close_input(&_fmt_ctx);
    _fmtCtxCanFree = false;

    freeAudio();
    freeVideo();

    _seekTime = -1;

}

bool VideoPlayer::canSeek(){
    if (_video_stream){
       return _vTime >= 0;
    }
      return true;
}


int VideoPlayer::getDuration(){

    if (_fmt_ctx){
//        return  round(_fmt_ctx->duration / 1000000);
        return  round(_fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q));
    }
   return 0;
}

int VideoPlayer::getTime(){
    return round(_aTime);
}

void VideoPlayer::setTime(int time){
    _seekTime = time;
    qDebug() << "_seekTime to"<< _seekTime;
}


void VideoPlayer::setState(State state){
    if (_state == state) {
        return;
    }
    _state = state;
    emit stateChanged(this);
}

void VideoPlayer::setFileName(const char *fileName){
    _fileName = fileName;
}

bool VideoPlayer::isPlaying(){
    return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState(){
    return _state;
}

void VideoPlayer::play(){
    if (isPlaying()){
        return;
    }

    if (_state == Stoped){

        std::thread([this](){
            this->readFile();
        }).detach();

    }else if (_state == Paused){
        //
        setState(Playing);
    }


}

void VideoPlayer::pause(){
    if (isPlaying()){
        setState(Paused);
    }
}

void VideoPlayer::stop(){
    qDebug() << "stop" << _state;

    if (_state == Stoped){
        return;
    }

//    setState(Stoped);

    _state = Stoped;
    // 释放资源
    free();
    emit stateChanged(this);


//    std::thread([this](){
//         SDL_Delay(33);
//        // 释放资源
//        free();
//    }).detach();
}

//void VideoPlayer::finish(){

//    if (_state == Finished){
//        return;
//    }

//    yuv_killTimer();
//    file->seek(0);
//    setState(Finished);
//    qDebug() << "播放结束";
//}

////////
/// \brief VideoPlayer::readFile
///
///

void VideoPlayer::onFailed(){
    emit failed(this);

    _aCanFree = true;
    _vCanFree = true;
    _fmtCtxCanFree = true;
//    setState(Stoped);

    bool needEmit = _state != Stoped;
    _state = Stoped;
    free();
    if (needEmit){
        emit stateChanged(this);
    }
}


int VideoPlayer::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type){
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


void VideoPlayer::readFile(){
    int ret = 0;

    /* open input file, and allocate format context */
    ret = avformat_open_input(&_fmt_ctx, _fileName, NULL, NULL);

    if (ret < 0){
        qDebug() << "Could not open source file" << _fileName;
        onFailed();
        return;
    }
    /* retrieve stream information */
    ret = avformat_find_stream_info(_fmt_ctx, NULL);
    if (ret < 0){
        qDebug() << "Could not find stream information";
        onFailed();
        return;
    }

    // 获取视频数据流，解码上下文
    ret = open_codec_context(&_video_stream_idx, &_video_dec_ctx, _fmt_ctx, AVMEDIA_TYPE_VIDEO);
    if (ret >= 0) {
        _video_stream = _fmt_ctx->streams[_video_stream_idx];
    }

    // 获取音频数据流，解码上下文
    ret = open_codec_context(&_audio_stream_idx, &_audio_dec_ctx, _fmt_ctx, AVMEDIA_TYPE_AUDIO);
    if (ret >= 0) {
        _audio_stream = _fmt_ctx->streams[_audio_stream_idx];
    }
    /* dump input information to stderr */
    av_dump_format(_fmt_ctx, 0, _fileName, 0);

    if (!_audio_stream && !_video_stream) {
        qDebug() << "Could not find audio or video stream in the input, aborting";
        onFailed();
        return;
    }

    if(_audio_stream){
        // 音频信息初始化
        ret = initAudioInfo();
        if (ret < 0){
            qDebug() << "initAudioInfo error";
            onFailed();
            return;
        }
    }

    if (_video_stream){
        // 视频信息初始化
        ret = initVideoInfo();
        if (ret < 0){
            qDebug() << "initVideoInfo error";
            onFailed();
            return;
        }
    }

    emit initFinished(this);

    // 设置状态开始播放
    setState(Playing);


    if (_video_stream){
        // 开始解码
        std::thread([this](){
            this->decodeVideo();
        }).detach();
    }

    if (_audio_stream){
        SDL_PauseAudio(0);
    }

    AVPacket pkt;
     /* read frames from the file */
    while (true) {

        if(_state == Stoped){
            break;
        }

        if (_seekTime >=0) {
            int streamIndex = _audio_stream_idx;
            if (!_audio_stream){
                streamIndex = _video_stream_idx;
            }
            // 现实时间转换为ffmpeg中定义的时间
            int timeStamp = _seekTime / (av_q2d(_fmt_ctx->streams[streamIndex]->time_base));
            ret = av_seek_frame(_fmt_ctx, streamIndex, timeStamp, AVSEEK_FLAG_BACKWARD);

            if (ret < 0){
                // 失败
                _seekTime = -1;
            }else{

                clearAudioPktList();
                clearVideoPktList();
                // 成功
                _vSeekTime = _seekTime;
                _aSeekTime = _seekTime;
                _seekTime = -1;

                _aTime = -1;
                _vTime = -1;

            }
        }

        if (_aPktList->size() >= AUDIO_MAX_PKT_SIZE || _vPktList->size() >= VIDEO_MAX_PKT_SIZE){
//            SDL_Delay(10);
            if(_state == Stoped){
                break;
            }
            continue;
        }

        ret = av_read_frame(_fmt_ctx, &pkt);
        if (ret == 0){
            if (pkt.stream_index == _video_stream_idx){
                // 处理视频包
                addVideoPkt(pkt);
            }else if(pkt.stream_index == _audio_stream_idx){
                // 处理音频包
                addAudioPkt(pkt);
            }else{
                // 其它流不做处理
                av_packet_unref(&pkt);
            }
        }else{
            if (ret == AVERROR_EOF){
//                qDebug() << "已经读取到文件尾部";
//                break;

                if(_aPktList->size() == 0 && _vPktList->size() == 0){
                    _fmtCtxCanFree = true;
                    break;
                }
            }
            else{
                continue;
            }
        }
    }
    if(_fmtCtxCanFree){
        stop();
    }
    _fmtCtxCanFree = true;

    qDebug() << "readFile end" ;
}



