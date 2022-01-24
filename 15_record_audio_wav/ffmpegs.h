#ifndef FFMPEGS_H
#define FFMPEGS_H

#include <stdint.h>

#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

typedef struct {
    // RIFF chunk的id
    uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'}; // 0
    // RIFF chunk的data大小，即文件总长度减去8字节
    uint32_t riffChunkDataSize; // 4

    // "WAVE"
    uint8_t format[4] = {'W', 'A', 'V', 'E'}; //8

    /* fmt chunk */
    // fmt chunk的id
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '}; //12
    // fmt chunk的data大小：存储PCM数据时，是16
    uint32_t fmtChunkDataSize = 16; // 16


    // 音频编码，1表示PCM，3表示Floating Point
    uint16_t audioFormat = AUDIO_FORMAT_PCM; // 20
    // 声道数
    uint16_t numChannels; // 22
    // 采样率
    uint32_t sampleRate; // 24
    // 字节率 = sampleRate * blockAlign
    uint32_t byteRate; // 28
    // 一个样本的字节数 = bitsPerSample * numChannels >> 3
    uint16_t blockAlign; // 32
    // 位深度
    uint16_t bitsPerSample; // 34

    /* data chunk */
    // data chunk的id
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'}; // 36
    // data chunk的data大小：音频数据的总长度，即文件总长度减去文件头的长度(一般是44)
    uint32_t dataChunkDataSize; // 40


} WAVHeader;

class FFmpegs
{
public:
    FFmpegs();
    static void pcm2wav(WAVHeader &wavHeader, const char *pcmFilePath, const char *wavFilePath);
};

#endif // FFMPEGS_H
