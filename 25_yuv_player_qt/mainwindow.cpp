#include "mainwindow.h"
#include "ui_mainwindow.h"

static const Yuv yuvs[] = {
    {
        "/Users/zhoujianshun/Downloads/yuv/yuyv422.yuv",
        1280,
        720,
        AV_PIX_FMT_YUYV422,
        30,
    },
    {
        "/Users/zhoujianshun/Downloads/yuv/3.yuv",
        1280,
        720,
        AV_PIX_FMT_YUV420P,
        30,
    },
    {
        "/Users/zhoujianshun/Downloads/yuv/1.rgb",
        1280,
        720,
        AV_PIX_FMT_RGB24,
        30,
    },
    {
        "/Users/zhoujianshun/Downloads/yuv/p420.yuv",
        1280,
        766,
        AV_PIX_FMT_YUV420P,
        1,
    },
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _yuvPlayer = new YUVPlayer(this);
    // 设置播放器尺寸和位置
    int w = 400;
    int h = 400;
    int x = (width() - w) >> 1;
    int y = (height() - h) >> 1;
    _yuvPlayer->setGeometry(x,y,w,h);

    // 设置播放文件
    Yuv yuv = yuvs[_index];
//    yuv.fileName = "/Users/zhoujianshun/Downloads/yuv/1.rgb";
//    yuv.width = 1280;
//    yuv.height = 720;
//    yuv.pixelFormat = AV_PIX_FMT_RGB24;
//    yuv.fps = 30;
    _yuvPlayer->setYuv(yuv);

    connect(_yuvPlayer, &YUVPlayer::stateChanged, this, &MainWindow::on_playStateChanged);
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

void MainWindow::on_playStateChanged()
{
   YUVPlayer::State state = _yuvPlayer->getState();
    if ( state == YUVPlayer::Playing) {
        ui->playButton->setText("暂停");
    } else if ( state == YUVPlayer::Paused) {
        ui->playButton->setText("播放");
    } else if ( state == YUVPlayer::Stoped) {
        ui->playButton->setText("播放");
    } else if ( state == YUVPlayer::Finished) {
        ui->playButton->setText("播放");
//        on_nextButton_clicked();
    }
}

void MainWindow::on_nextButton_clicked()
{
    int yuvCount = sizeof(yuvs)/sizeof(Yuv);
    _index = ( yuvCount + _index + 1) % yuvCount ;

    Yuv yuv = yuvs[_index];
    _yuvPlayer->setYuv(yuv);
    _yuvPlayer->play();
    ui->indexLabel->setText(QString::number(_index));
}

