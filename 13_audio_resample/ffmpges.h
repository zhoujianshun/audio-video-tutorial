#ifndef FFMPGES_H
#define FFMPGES_H

extern "C" {

#include <libavformat/avformat.h>

}

typedef struct {
    const char *fileName;
    int64_t ch_layout;
    enum AVSampleFormat sample_fmt;
    int  sample_rate;
} ResampleAudioSpec;

class FFmpges
{
public:
    FFmpges();
    static void resampleAudio(ResampleAudioSpec inAudio, ResampleAudioSpec outAudio);
    static void resampleAudio(const char *inFileName, int64_t in_ch_layout, enum AVSampleFormat in_sample_fmt, int in_sample_rate,
                         const char *outFileName, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate);
};

#endif // FFMPGES_H
