#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qtimer.h>

#include "aduioresamplethread.h"


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


void MainWindow::on_audioButton_clicked()
{
    if(this->audioThread){
        this->audioThread->requestInterruption();
        this->audioThread = nullptr;
        ui->audioButton->setText("开始录音");

    }else{
        // 开启线程录音
        this->audioThread = new AudioThread(this);
        this->audioThread->start();
        ui->audioButton->setText("结束录音");

        connect(this->audioThread, &AudioThread::finished,
        [this]() { // 线程结束
            this->audioThread = nullptr;
            ui->audioButton->setText("开始录音");
        });

        // 连接
        connect(this->audioThread,
                &AudioThread::startSignal,
                this,
                &MainWindow::on_start_record);
        connect(this->audioThread,
                &AudioThread::endSignal,
                this,
                &MainWindow::on_end_record);
    }

}

void MainWindow::on_start_record()
{
    qDebug() << "on_start_record";
    if(!this->timer){
        this->timer = new QTimer(this);
        connect(this->timer,&QTimer::timeout,this,&MainWindow::on_timeout);
        this->timer->start(1000);
        this->count = 0;
    }

}

void MainWindow::on_end_record()
{
    qDebug() << "on_end_record";
    if(this->timer){
        this->timer->stop();
        this->timer = nullptr;

    }
}
void MainWindow::on_timeout()
{
    qDebug() << "on_timeout";
    ui->timeLabel->setText( QString::number(++this->count));
}




void MainWindow::on_resampleButton_clicked()
{
    AduioResampleThread *thread = new AduioResampleThread(this);
    thread->start();
}

