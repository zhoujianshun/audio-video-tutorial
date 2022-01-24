#include "playthread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

// ffprobe -ac 1 -ar 48000 -f f32le /Users/zhoujianshun/Downloads/out.pcm
#define FILENAME "/Users/zhoujianshun/Downloads/out.pcm"
#define SAMPLE_RATE 48000

#define FORMAT AUDIO_F32LSB
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(FORMAT) // 样本大小（位）
#define CHANNELS 1 // 声道数
#define SAMPLES 1024 // 缓冲区样本数量

#define BYTE_PRE_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3) // 每个样本的大小

#define BUFFER_SIZE (SAMPLES * BYTE_PRE_SAMPLE) // 缓冲区大小（字节）

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
    int bufferLength = 0;
    int pullLen = 0; // 当前拉取的样本大小
    char *bufferData = nullptr;
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

    AudioBuffer audioBuffer;


    // 音频参数
    SDL_AudioSpec spec;
    // 采样率
    spec.freq = SAMPLE_RATE;
    // 声道数
    spec.channels = CHANNELS;
    // 采样格式（f32le）
    spec.format = FORMAT;
    // 音频缓冲区的样本数量（必须是2的幂）
    spec.samples = SAMPLES; // 1024个样本
    // 回调，拉取数据
    spec.callback = pull_audio_data;
    spec.userdata = &audioBuffer;

    // 打开设备
    if(SDL_OpenAudio(&spec, nullptr) != 0){
        qDebug() << "Unable to open SDL audio: " << SDL_GetError();
        SDL_Quit();
        return;
    }
    qDebug() << "打开设备成功 ";
    // 打开文件
    QFile file(FILENAME);
    if(file.open(QFile::ReadOnly) == 0){
        // 返回值为0表示失败
        qDebug() << "Open file error: " << FILENAME;

        SDL_CloseAudio();
        SDL_Quit();
        return;
    }
    qDebug() << "开始播放（取消暂停） ";
    // 开始播放（取消暂停）
    SDL_PauseAudio(0);

    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        // 使用完了在读取数据
        while (audioBuffer.bufferLength > 0) {
            continue;
        }

        // 从文件读取数据
        audioBuffer.bufferLength = file.read(data, BUFFER_SIZE);

        if (audioBuffer.bufferLength <= 0){

            if(audioBuffer.pullLen > 0 ){
                // 延迟结束的时间
                // 读取完数据后，可能还没播放完
                int samples = audioBuffer.pullLen / BYTE_PRE_SAMPLE;
                int ms = samples / SAMPLE_RATE * 1000;
                SDL_Delay(ms);
                qDebug() << ms;
            }
            break;
        }
        audioBuffer.bufferData = data;
    }


//    while (!isInterruptionRequested()) {
//          // 从文件读取数据
//        bufferLength = file.read(data, BUFFER_SIZE);

//        if (bufferLength <= 0){
//            break;
//        }
//        bufferData = data;

//        // 使用完了在读取数据
//        while (bufferLength > 0) {
//            SDL_Delay(1);
//        }
//    }

    qDebug() << "播放完成 ";
    // 关闭文件
    file.close();
    // 关闭设备
    SDL_CloseAudio();
    // 关闭子系统
    SDL_Quit();

}
