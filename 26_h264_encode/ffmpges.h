#ifndef FFMPGES_H
#define FFMPGES_H

extern "C" {

#include <libavutil/avutil.h>

}

typedef struct {
    const char *fileName;
    int width;
    int height;
    enum AVPixelFormat format;
    int  fps;
} VideoEncodeSpec;

class FFmpges
{
public:
    FFmpges();
    static void h264Encode(VideoEncodeSpec in, const char *out);

};

#endif // FFMPGES_H
