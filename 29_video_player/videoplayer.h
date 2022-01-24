#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QFile>
#include <list>
#include "condmutex.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT
public:

    typedef enum{
        Playing,
        Paused,
        Stoped,
//        Finished,
    } State;

    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    typedef struct {
        int64_t ch_layout;
        enum AVSampleFormat sample_fmt;
        int  sample_rate;
        int channels;
        int bytesPerSampleFrame; // 每个样本帧的大小
    } ResampleAudioSpec;

    typedef struct{
        int width;
        int height;
        AVPixelFormat format;
    } VideoSwsSpec;

    typedef enum {
        Min = 0,
        Max = 100
    } Volumn;

    void play();
    void pause();
    void stop();
    void setFileName(const char *fileName);
    State getState();

    bool isPlaying();


    int getDuration();
    // 设置音量
    void setVolumn(int volumn);
    int getVolumn();
    void setMute(bool mute);
    bool getMute();
    int getTime();
    void setTime(int time);
    bool canSeek();

private:
    /******** 音频相关*************/
    // 解码上下文
    AVCodecContext *_audio_dec_ctx = NULL;
    int _audio_stream_idx = -1;
    // 流
    AVStream *_audio_stream = NULL;
    // 存放解码后的数据
    AVFrame *_aSwrInFrame = NULL, *_aSwrOutFrame = NULL;
    // 音频包列表
    std::list<AVPacket> *_aPktList = nullptr;
    // 音频包列表锁
    CondMutex *_aCondMutex = nullptr;
    // 音频重采样上下文
    SwrContext *_aSwrCtx = nullptr;
    // 重采样输入树垂参数
    ResampleAudioSpec _aSwrInSpec, _aSwrOutSpec;
    int _out_nb_samples;// 重采样输出样本数量
    int _aSwrOutSize = 0; // 重采样数据大小
    int _aSwrOutIdx = 0; // 重采样数据已填充到sdl的suoyin
    /** 音量*/
    int _volumn = Max;
    /** 是否静音*/
    bool _mute = false;
    /** 音频当前的时间，用于音视频同步*/
    double _aTime = 0;
    /** 是否可以释放音频资源*/
    bool _aCanFree = false;
    // 用于seek音频
    int _aSeekTime = -1;


    /**添加数据到pkt列表*/
    void addAudioPkt(AVPacket &pkt);
    /** 清除音频pkt列表*/
    void clearAudioPktList();
    /** 初始化sdl*/
    int initSDL();
    void sdlAudioCallback(Uint8 * stream,
                          int len);
    /** sdl回调*/
    static void sdlAudioCallbackFunc(void *userdata, Uint8 * stream,
                          int len);
    /** 音频解码*/
    int decodeAudio();

    /** 音频信息初始化*/
    int initAudioInfo();
    /** 初始化音频重采样*/
    int initAudioSwr();
    /** 释放音频资源*/
    void freeAudio();


    /******** 视频相关*************/
    AVCodecContext *_video_dec_ctx = NULL;
    /**视频流索引*/
    int _video_stream_idx = -1;
    /**视频流*/
    AVStream *_video_stream = NULL;
    // 像素格式转换输入frame
    AVFrame *_vSwsInFrame = NULL;
    // 像素格式转换输出frame
    AVFrame *_vSwsOutFrame = NULL;
    // 视频包列表
    std::list<AVPacket> *_vPktList = nullptr;
    // 视频包列表锁
    CondMutex *_vCondMutex = nullptr;
    // 像素格式转换上下文
    SwsContext *_vSwsCtx = nullptr;
    // 像素格式输出的frame的参数
    VideoSwsSpec _vSwsOutSpec;
    /** 视频时间，用于音视频同步*/
    double _vTime = 0;
    /** 视频资源可释放*/
    bool _vCanFree = false;
    /** 图片的大小*/
    int _imageSize;

    /** 添加pkt到视频pkt列表*/
    void addVideoPkt(AVPacket &pkt);
    /** 清除视频pkt列表*/
    void clearVideoPktList();
    /** 视频解码*/
    void decodeVideo();
    /** 初始化视频信息*/
    int initVideoInfo();
    /** 初始化像素转换上下文*/
    int initVideoSws();
    /** 释放视频资源*/
    void freeVideo();
     // 用于seek视频
    int _vSeekTime = -1;

    /******** 其它*************/
    const char *_fileName;
    State _state = Stoped;
    AVFormatContext *_fmt_ctx = nullptr;
    bool _fmtCtxCanFree = false;
    int _seekTime = -1;

    void setState(State state);
    void finish();
    void readFile();
    void onFailed();


    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    /** 释放资源*/
    void free();


signals:
    /** 状态改变*/
    void stateChanged(VideoPlayer *player);
    /** 初始化完成*/
    void initFinished(VideoPlayer *player);
    /** 出现错误*/
    void failed(VideoPlayer *player);
    /** 当前的播放时间*/
    void timeChanged(VideoPlayer *player);
    /** 视频解码完成*/
    void frameDecoded(VideoPlayer *player, uint8_t *data, VideoSwsSpec spec);
};

#endif // VIDEOPLAYER_H
