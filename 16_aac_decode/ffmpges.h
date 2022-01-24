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
} AudioEncodeSpec;

class FFmpges
{
public:
    FFmpges();
    static void aacEncode(AudioEncodeSpec in, const char *out);
    static void aacDecode( const char *in, AudioEncodeSpec &out);
};

#endif // FFMPGES_H
