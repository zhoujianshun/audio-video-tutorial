#ifndef FFMPGES_H
#define FFMPGES_H

extern "C" {

#include <libavutil/avutil.h>

}

typedef struct {
    const char *fileName;
    int  width;
    int  height;
    enum AVPixelFormat pixfmt;
    int fps;
} VideoDecodeSpec;

class FFmpges
{
public:
    FFmpges();
    static void h264Decode( const char *in, VideoDecodeSpec &out);
};

#endif // FFMPGES_H
