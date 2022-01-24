#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>

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

    QFile *file = nullptr;

    int timeId = 0;
    Yuv _yuv;
    State _state = Stoped;
    QImage *_image = nullptr;
    QRect _destRect;
    int _imageSize;

    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);
    void freeImage();
    void setState(State state);
    void yuv_killTimer();
    void finish();
    void closeFile();


signals:
    void stateChanged();

};

#endif // YUVPLAYER_H
