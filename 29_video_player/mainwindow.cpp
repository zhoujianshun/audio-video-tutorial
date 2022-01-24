#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 注册信号参数类型
    qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec");


    _player = new VideoPlayer();
    connect(_player, &VideoPlayer::stateChanged,
            this, &MainWindow::onPlayerStateChanged);
    connect(_player, &VideoPlayer::initFinished,
            this, &MainWindow::onInitFinished);
    connect(_player, &VideoPlayer::failed,
            this, &MainWindow::onPlayerFailed);
    connect(_player, &VideoPlayer::timeChanged,
            this, &MainWindow::onPlayerTimeChanged);
    connect(_player, &VideoPlayer::frameDecoded,
            ui->videoWidget, &VideoWidget::onFrameDecoded);
    connect(_player, &VideoPlayer::stateChanged,
            ui->videoWidget, &VideoWidget::onPlayerStateChanged);


    ui->volumnSlider->setRange(VideoPlayer::Min, VideoPlayer::Max);
    ui->volumnSlider->setValue(ui->volumnSlider->maximum()/4);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _player;
}

void MainWindow::onInitFinished(VideoPlayer *player){
    qDebug() << "duration" << player->getDuration();

    this->ui->currentSlider->setMaximum(player->getDuration());
    this->ui->durationLabel->setText(getTimeText(player->getDuration()));
}


void MainWindow::onPlayerFailed(VideoPlayer *player){
    QMessageBox::critical(NULL,"提示","出错了");
}

void MainWindow::onPlayerStateChanged(VideoPlayer *player){
    VideoPlayer::State state = player->getState();

    if (state == VideoPlayer::Playing){
        ui->playBtn->setText("暂停");

    } else {
        ui->playBtn->setText("播放");
    }

    if (state == VideoPlayer::Stoped) {

        ui->playWidget->setCurrentIndex(0);
        ui->playBtn->setEnabled(false);
        ui->stopBtn->setEnabled(false);
        ui->currentSlider->setEnabled(false);
        ui->volumnSlider->setEnabled(false);
        ui->silenceBtn->setEnabled(false);

        ui->currentSlider->setValue(0);
//        ui->volumnSlider->setValue(100);
        ui->durationLabel->setText(getTimeText(0));



    } else {
        ui->playWidget->setCurrentIndex(1);

        ui->playBtn->setEnabled(true);
        ui->stopBtn->setEnabled(true);
        ui->currentSlider->setEnabled(true);
        ui->volumnSlider->setEnabled(true);
        ui->silenceBtn->setEnabled(true);

    }
}

void MainWindow::onPlayerTimeChanged(VideoPlayer *player){
    int time = player->getTime();
    qDebug() << "get time" << time;
    if (time >= ui->currentSlider->minimum() && time <= ui->currentSlider->maximum()){
        this->ui->currentSlider->setValue(time);
    }

}

void MainWindow::on_stopBtn_clicked()
{
     _player->stop();
//    if (_player->isPlaying()){
//        _player->stop();
//    }
}

void MainWindow::on_playBtn_clicked()
{

    if (_player->isPlaying()){
        _player->pause();
    } else {
        _player->play();
    }
}


void MainWindow::on_openFileBtn_clicked()
{
    QString fileName =  QFileDialog::getOpenFileName(NULL,
                                 "选择多媒体文件",
                                 "/Users/zhoujianshun/Downloads/yuv",
                                 "多媒体文件 (*.mp3 *.aac *.mp4 *.avi *.flv *.h264)");
    qDebug() << fileName;
    if (fileName.isEmpty()){
        return;
    }

//    _player->setFileName(fileName.toStdString().c_str());
    _player->setFileName(fileName.toUtf8().data());
    _player->play();
}


void MainWindow::on_volumnSlider_valueChanged(int value)
{
     ui->volumnLabel->setText(QString("%1").arg(value));
     _player->setVolumn(value);
}


void MainWindow::on_currentSlider_valueChanged(int value)
{
    this->ui->currentLabel->setText(getTimeText(value));
}


///
///

QString MainWindow::getTimeText(int64_t time)
{


    //    QLatin1Char fill = QLatin1Char('0');
    //   return QString("%1:%2:%3")
    //          .arg(time / 3600, 2, 10, fill)
    //          .arg((time / 60) % 60, 2, 10, fill)
    //          .arg(time % 60, 2, 10, fill);

    QString h = QString("%1").arg(time / 3600, 2, 10, QLatin1Char('0'));
    QString m = QString("%1").arg((time / 60) % 60, 2, 10, QLatin1Char('0'));
    QString s = QString("%1").arg(time % 60, 2, 10, QLatin1Char('0'));

    return QString("%1:%2:%3").arg(h).arg(m).arg(s);
}

void MainWindow::on_silenceBtn_clicked()
{
    if (_player->getMute()){
        _player->setMute(false);
        ui->silenceBtn->setText("静音");
    } else {
        _player->setMute(true);
        ui->silenceBtn->setText("出音");
    }
}


void MainWindow::on_currentSlider_sliderPressed()
{
    qDebug() << "on_currentSlider_sliderPressed";
    if (_player->isPlaying()){
        _player->pause();
        continuePlay = true;
    }

    _player->setTime(ui->currentSlider->value());
}


void MainWindow::on_currentSlider_sliderMoved(int position)
{
    qDebug() << "on_currentSlider_sliderMoved" << ui->currentSlider->value();

//    if(_player->canSeek()){
//      _player->setTime(ui->currentSlider->value());
//    }
    _player->setTime(ui->currentSlider->value());
}


void MainWindow::on_currentSlider_sliderReleased()
{
       qDebug() << "on_currentSlider_sliderReleased";

       if(continuePlay){
           continuePlay = false;
           _player->play();
       }
}

