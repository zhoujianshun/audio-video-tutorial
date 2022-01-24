#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <SDL2/SDL.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showSDLInfo()
{
    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}

/**
SDL播放音频两种方式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉去数据
*/
void MainWindow::on_playButton_clicked()
{
    if(_playThread){
        _playThread->requestInterruption();
        _playThread = nullptr;
        ui->playButton->setText("开始播放");
    }else{
        _playThread = new PlayThread(this);
        connect(_playThread, &PlayThread::finished,
        [this]() { // 线程结束
            _playThread = nullptr;
             ui->playButton->setText("开始播放");
        });
        _playThread->start();
        ui->playButton->setText("暂停播放");
    }

}

