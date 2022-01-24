#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _yuvPlayer = new YUVPlayer(this);
    int w = 600;
    int h = 600;
    int x = (width() - w) >> 1;
    int y = (height() - h) >> 1;
    _yuvPlayer->setGeometry(x,y,w,h);

    Yuv yuv;
    yuv.fileName = "/Users/zhoujianshun/Downloads/yuv/2.yuv";
    yuv.width = 1280;
    yuv.height = 720;
    yuv.pixelFormat = AV_PIX_FMT_YUV420P;
    yuv.fps = 30;

    _yuvPlayer->setYuv(yuv);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
SDL播放音频两种方式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉去数据
*/
void MainWindow::on_playButton_clicked()
{
    if (_yuvPlayer->isPlaying()){
        _yuvPlayer->pause();
        ui->playButton->setText("播放");
    } else {
        _yuvPlayer->play();
        ui->playButton->setText("暂停");
    }
}

void MainWindow::on_stopButton_clicked()
{
    _yuvPlayer->stop();
    ui->playButton->setText("播放");
}

