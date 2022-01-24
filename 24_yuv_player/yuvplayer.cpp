#include "yuvplayer.h"

#include <QDebug>

extern "C" {
    #include <libavutil/imgutils.h>
}

#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        return; \
    }

static const std::map<AVPixelFormat,SDL_PixelFormatEnum> PIXEL_FORMAT_MAP = {
            { AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV },
            { AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2 },
            { AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN },
        };

YUVPlayer::YUVPlayer(QWidget *parent) : QWidget(parent)
{
    // 创建窗口
    window = SDL_CreateWindowFrom((void *)this->winId());

    // 创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

}

YUVPlayer::~YUVPlayer(){

    file->close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void YUVPlayer::setYuv(Yuv &yuv){
    _yuv = yuv;

    // 创建纹理
    texture = SDL_CreateTexture(renderer,
                                PIXEL_FORMAT_MAP.find(yuv.pixelFormat)->second,
                                SDL_TEXTUREACCESS_STREAMING,
                                yuv.width, yuv.height);
    END(!texture, SDL_CreateTextureFromSurface);

    file = new QFile(_yuv.fileName);
    if (!file->open(QFile::ReadOnly)){
        qDebug() << "打开文件失败";
        return;
    }
}

bool YUVPlayer::isPlaying(){
    return _state == Playing;
}

YUVPlayer::State YUVPlayer::getState(){
    return _state;
}

void YUVPlayer::play(){
    timeId = startTimer(1000/_yuv.fps);
    _state = Playing;
}

void YUVPlayer::pause(){
    if (timeId){
        killTimer(timeId);
    }
    _state = Paused;
}

void YUVPlayer::stop(){
    if (timeId){
        killTimer(timeId);
    }
    file->seek(0);
    _state = Stoped;
}

void YUVPlayer::timerEvent(QTimerEvent *event){
//    int size = _yuv.width * _yuv.height * 1.5;
    int size = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);
    char data[size];
    int ret = file->read(data, size);
    if( ret == 0 ){
        killTimer(timeId);
        qDebug() << "播放结束";
        return;
    }

    // 将YUV数据填充到渲染目标
     END(SDL_UpdateTexture(texture, nullptr, data, _yuv.width) != 0, SDL_UpdateTexture);

    // 设置绘制颜色（画笔颜色）
    END(SDL_SetRenderDrawColor(renderer,
                               255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

    // 用绘制颜色（画笔颜色）清除渲染目标
    END(SDL_RenderClear(renderer),
        SDL_RenderClear);

    // 拷贝纹理数据到渲染目标（默认是window）
    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
        SDL_RenderCopy);

//    END(SDL_RenderCopy(renderer, texture, &srcRect, &dstRect), SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);
}
