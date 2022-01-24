#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qtimer.h>

#include "audiothread.h"
#include "decodethread.h"


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
    AudioThread *thread = new AudioThread(this);
    thread->start();
}



void MainWindow::on_decodeButton_clicked()
{
    DecodeThread *thread = new DecodeThread(this);
    thread->start();
}

