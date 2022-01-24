#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include "libavutil/avutil.h"
}

typedef struct{
    char *pixel;
    int width;
    int height;
    AVPixelFormat format;

} RawVideoFrame;

typedef struct{
    const char *fileName;
    int width;
    int height;
    AVPixelFormat format;

} RawVideoFile;

class FFmpegs
{
public:
    FFmpegs();
    static void convertRawFrame(RawVideoFrame &in, RawVideoFrame&out);
    static void convertRawVideo(RawVideoFile &in, RawVideoFile&out);
};

#endif // FFMPEGS_H
