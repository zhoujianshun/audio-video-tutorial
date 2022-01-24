#include "playthread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

// ffprobe -ac 1 -ar 48000 -f f32le /Users/zhoujianshun/Downloads/out.pcm
#define FILENAME "/Users/zhoujianshun/Downloads/12_10_17_12_54_2.wav"


// 缓冲区样本数量
#define SAMPLES 1024

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);

}


PlayThread::~PlayThread(){
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this <<"PlayThread::~PlayThread()";
}



typedef struct {
    Uint32 bufferLength = 0;
    Uint32 pullLen = 0; // 当前拉取的样本大小
    Uint8 *bufferData = nullptr;
} AudioBuffer;

// 音频回调
void pull_audio_data(void *userdata,
                     // 需要往stream中填充pcm数据
                     Uint8 * stream,
                     // 希望填充的大小（sample * format * channels）(1024 * (32/8) * 1)
                     int len)
{

    AudioBuffer *audioBuffer = (AudioBuffer *)userdata;

    // 清空stream
    SDL_memset(stream, 0, len);
    if( audioBuffer->bufferLength <= 0){
        return;
    }

    audioBuffer->pullLen = audioBuffer->bufferLength > len ? len : audioBuffer->bufferLength;

    SDL_MixAudio(stream, (Uint8 *)audioBuffer->bufferData, audioBuffer->pullLen, SDL_MIX_MAXVOLUME);
    audioBuffer->bufferData += audioBuffer->pullLen;
    audioBuffer->bufferLength -= audioBuffer->pullLen;
}

void PlayThread::run()
{
    // 初始化子系统
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
//        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        qDebug() << "Unable to initialize SDL: " << SDL_GetError();
        return;
    }


    // 加载pcm
    // 音频信息
    SDL_AudioSpec spec;
    // pcm数据
    Uint8 * audio_buf;
    // pcm数据长度
    Uint32  audio_len;
    if(!SDL_LoadWAV(FILENAME, &spec, &audio_buf, &audio_len)){
        qDebug() << "Unable to SDL_LoadWAV: " << SDL_GetError();
        SDL_Quit();
        return;
    }
    spec.callback = pull_audio_data;
    // 缓冲区样本数量
    spec.samples = 1024;

    AudioBuffer audioBuffer;
    audioBuffer.bufferData = audio_buf;
    audioBuffer.bufferLength = audio_len;

    spec.userdata = &audioBuffer;
    // 打开设备
    if(SDL_OpenAudio(&spec, nullptr) != 0){
        qDebug() << "Unable to open SDL audio: " << SDL_GetError();
        SDL_Quit();
        return;
    }
    qDebug() << "打开设备成功 ";

    qDebug() << "开始播放（取消暂停） ";
    // 开始播放（取消暂停）
    SDL_PauseAudio(0);

    int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
    int bytePerSample = (sampleSize * spec.channels) >> 3;
    while (!isInterruptionRequested()) {
        // 使用完了在读取数据
        while (audioBuffer.bufferLength > 0) {
            continue;
        }

        if (audioBuffer.bufferLength <= 0){

            if(audioBuffer.pullLen > 0 ){
                // 延迟结束的时间
                // 读取完数据后，可能还没播放完
                int samples = audioBuffer.pullLen / bytePerSample;
                int ms = samples / sampleSize * 1000;
                SDL_Delay(ms);
                qDebug() << ms;
            }
            break;
        }
//        audioBuffer.bufferData = data;
    }

    qDebug() << "播放完成 ";
    // 关闭文件
    SDL_FreeWAV(audio_buf);
    // 关闭设备
    SDL_CloseAudio();
    // 关闭子系统
    SDL_Quit();

}
