#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>

#include <SDL2/SDL.h>
#include <QFile>

extern "C" {
    #include "libavutil/avutil.h"
}

typedef struct{
    const char *fileName;
    int width;
    int height;
    AVPixelFormat pixelFormat;
    int fps;
} Yuv;



class YUVPlayer : public QWidget
{
    Q_OBJECT
public:

    typedef enum{
        Default,
        Playing,
        Paused,
        Stoped,
        Finished,
    } State;

    explicit YUVPlayer(QWidget *parent = nullptr);
    ~YUVPlayer();

    void play();
    void pause();
    void stop();
    void setYuv(Yuv &yuv);
    State getState();

    bool isPlaying();



private:
    // 窗口
    SDL_Window *window = nullptr;
    // 渲染上下文
    SDL_Renderer *renderer = nullptr;
    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *texture = nullptr;
    QFile *file = nullptr;

    int timeId = 0;
    Yuv _yuv;
    State _state;

    void timerEvent(QTimerEvent *event);

signals:

};

#endif // YUVPLAYER_H
