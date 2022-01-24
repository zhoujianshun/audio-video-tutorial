#ifndef DEMUXER_H
#define DEMUXER_H

#include <QFile>

extern "C" {

#include <libavformat/avformat.h>
}

typedef struct {
    const char *fileName;
    int  width;
    int  height;
    enum AVPixelFormat pixfmt;
    int fps;
} VideoDecodeSpec;

typedef struct {
    const char *fileName;
    int  sampleRate;
    uint64_t  chLayout;
    enum AVSampleFormat fmt;
} AudioDecodeSpec;

class Demuxer
{

private:
    AVFormatContext *_fmt_ctx = nullptr;
    AVCodecContext *_video_dec_ctx = NULL, *_audio_dec_ctx = NULL;
    int _video_stream_idx = -1, _audio_stream_idx = -1;
    AVStream *_video_stream = NULL, *_audio_stream = NULL;
    AVFrame *_frame = NULL;
    AVPacket *_pkt = NULL;
    QFile *vFile = nullptr;
    QFile *aFile = nullptr;
    uint8_t *_video_dst_data[4] = {NULL};
    int      _video_dst_linesize[4];
    int _video_dst_bufsize = 0;
    int _sampleSize = 0;
    int _sampleFrameSize = 0;

    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    int output_video_frame(AVFrame *frame);
    int output_audio_frame(AVFrame *frame);


public:
    Demuxer();

    void demux(const char *inFilename, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut);
};

#endif // DEMUXER_H
