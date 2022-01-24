#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include "videoplayer.h"


// 显示视频
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

    QRect _destRect;

private:
    QImage *_image = nullptr;
    void paintEvent(QPaintEvent *event);
    void freeImage();
signals:

public slots:
    void onFrameDecoded(VideoPlayer *player, uint8_t *data, VideoPlayer::VideoSwsSpec spec);
    void onPlayerStateChanged(VideoPlayer *player);
};

#endif // VIDEOWIDGET_H
