#include "yuvplayer.h"

#include <QDebug>
#include <QPainter>

#include "ffmpegs.h"

extern "C" {
    #include <libavutil/imgutils.h>
}

#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        return; \
    }


YUVPlayer::YUVPlayer(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("background:black");

}

YUVPlayer::~YUVPlayer(){

    closeFile();
    freeImage();
    yuv_killTimer();
}

void YUVPlayer::freeImage(){
    if (_image){
        free(_image->bits());
        if (_image){
            delete _image;
            _image = nullptr;
        }

    }
}

void YUVPlayer::closeFile(){
    if (file){
        file->close();
        delete file;
        file = nullptr;
    }
}

void YUVPlayer::yuv_killTimer(){
    if (timeId){
        killTimer(timeId);
        timeId = 0;
    }
}

void YUVPlayer::setState(State state){
    if (_state == state) {
        return;
    }
    _state = state;
    emit stateChanged();
}

void YUVPlayer::setYuv(Yuv &yuv){
//    bool startPlay = false;
    if (isPlaying()){
        stop();

//        startPlay = true;
    }
    closeFile();

    _yuv = yuv;
    file = new QFile(_yuv.fileName);
    if (!file->open(QFile::ReadOnly)){
        qDebug() << "打开文件失败";
        return;
    }

    int w = width();
    int h = height();

    int dw = w;
    int dh = _yuv.height * dw / _yuv.width ;

    if (dh > h){
        dh = h;
        dw = _yuv.width * dh / _yuv.height ;
    }

    int dx = (w - dw) >> 1;
    int dy = (h - dh) >> 1;

    _destRect = QRect(dx,dy, dw, dh);
    _imageSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);

//    if (startPlay) {
//        play();
//    }
}

bool YUVPlayer::isPlaying(){
    return _state == Playing;
}

YUVPlayer::State YUVPlayer::getState(){
    return _state;
}

void YUVPlayer::play(){
    if (isPlaying()){
        return;
    }
    timeId = startTimer(1000/_yuv.fps);
    setState(Playing);
}

void YUVPlayer::pause(){
    if (isPlaying()){
        yuv_killTimer();
        setState(Paused);
    }
}

void YUVPlayer::stop(){

    if (_state == Stoped){
        return;
    }

    yuv_killTimer();
    file->seek(0);
    freeImage();
    update();
    setState(Stoped);
}

void YUVPlayer::finish(){

    if (_state == Finished){
        return;
    }

    yuv_killTimer();
    file->seek(0);
    setState(Finished);
    qDebug() << "播放结束";
}

void YUVPlayer::paintEvent(QPaintEvent *event){

    if (!_image) {
        return;
    }
    QPainter painter(this);
    painter.drawImage(_destRect, *_image);
}

void YUVPlayer::timerEvent(QTimerEvent *event){
//    int size = _yuv.width * _yuv.height * 1.5;
    int size = _imageSize;
    char data[size];
    int ret = file->read(data, size);
    if( ret != size ){
        finish();
        return;
    }else{
//        if ( _yuv.pixelFormat == AV_PIX_FMT_RGB24){
//            freeImage();
//            _image = new QImage((uchar *)data, _yuv.width,_yuv.height, QImage::Format_RGB888);
//        }else{

//        }

        RawVideoFrame in ={
            data,
            _yuv.width,
            _yuv.height,
            _yuv.pixelFormat,
        };
        RawVideoFrame out ={
            nullptr,
            _yuv.width >> 4 << 4, // 最好是16的倍数，否则性能会有影响
            _yuv.height >> 4 << 4,
            AV_PIX_FMT_RGB24,
        };
        FFmpegs::convertRawFrame(in, out);
        freeImage();
        _image = new QImage((uchar *)out.pixel, out.width,out.height, QImage::Format_RGB888);

        update();
    }

}


