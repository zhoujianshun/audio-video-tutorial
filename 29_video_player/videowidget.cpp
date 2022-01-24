#include "videowidget.h"

#include <QDebug>
#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    qDebug()<<"VideoWidget";
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background:black");
}

VideoWidget::~VideoWidget(){
    freeImage();
}

void VideoWidget::freeImage(){
    if (_image){
        free(_image->bits());
        if (_image){
            delete _image;
            _image = nullptr;
        }
    }
}

void VideoWidget::paintEvent(QPaintEvent *event){

    if (!_image) {
        return;
    }
    QPainter painter(this);
    painter.drawImage(_destRect, *_image);
}

void VideoWidget::onFrameDecoded(VideoPlayer *player, uint8_t *data, VideoPlayer::VideoSwsSpec spec){
    // 停止后，不需要更新UI
    if(player->getState() == VideoPlayer::Stoped){
        return;
    }
    freeImage();
    if (data){
        _image = new QImage((uchar *)data, spec.width,spec.height, QImage::Format_RGB888);

        int w = width();
        int h = height();

        int dw = w;
        int dh = spec.height * dw / spec.width ;

        if (dh > h){
            dh = h;
            dw = spec.width * dh / spec.height ;
        }

        int dx = (w - dw) >> 1;
        int dy = (h - dh) >> 1;

        _destRect = QRect(dx,dy, dw, dh);
    }

    update();
}

void VideoWidget::onPlayerStateChanged(VideoPlayer *player){
    if (player->getState() == VideoPlayer::Stoped){
        qDebug() << "onPlayerStateChanged Stoped";
        freeImage();
        update();
    }
}
