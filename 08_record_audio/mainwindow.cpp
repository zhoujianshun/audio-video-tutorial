#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFile>

extern "C"{
    #include <libavdevice/avdevice.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
}


#ifdef Q_OS_MAC
    #define FORMATNAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILE_NAME "/Users/zhoujianshun/Downloads/out.pcm"
#elif Q_OS_WIN
    #define FORMATNAME "dshow"
    #define DEVICE_NAME ""
    #define FILE_NAME ""
#else

#endif

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

     //
    // ffmpeg -devices
    // 查看所有设备
    // ffmpeg -list_devices true -f avfoundation -i dummy 查看avfoundation可用的设备
    // 查看适用于avfoundation的参数
    // ffmpeg -h demuxer=avfoundation
    //
    // ffmpeg -formats | grep PCM
    // ffplay -ac 1 -ar 48000 -f f32le /Users/zhoujianshun/Downloads/out.pcm


    const char* formatName = FORMATNAME;
    /// 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(formatName);

    if (!fmt){
        qDebug() << "获取输入格式失败" << formatName;
        return;
    }
    /// 打开设备

    // 格式上下文，用来操作设备
    AVFormatContext *context = nullptr;
    // 设备名称
    const char* deviceName = DEVICE_NAME;
    qDebug() << "打开设备1";

    int ret = avformat_open_input(&context, deviceName,fmt, nullptr);
    qDebug() << "打开设备2";
    if (ret < 0){
        this->printErrorMessage("打开设备失败",ret);
        return;
    }

    /// 采集数据
     qDebug() << "打开文件";
    // 打开文件
    const char* fileName = FILE_NAME;
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)){
        // 文件打开失败
        qDebug() << "文件打开失败" << fileName;
        avformat_close_input(&context);
        return;
    }
    qDebug() << "采集50次";
    // 采集50次
    int count = 50;

    // 数据包
    AVPacket pkt;
    // 采集数据
    while (  count >0) {

        ret = av_read_frame(context, &pkt);
        if (ret < 0) {
            this->printErrorMessage("av_read_frame error",ret);
            break;
        }else{
            file.write((const char*)pkt.data, pkt.size);
            count--;
        }
    }

    // 关闭文件
    avformat_close_input(&context);
    file.close();
    qDebug() << "采集50次完毕";
}

void MainWindow::printErrorMessage(const char *message, int errorCode){
    char errbuf[1024];
    av_strerror(errorCode,errbuf, sizeof(errbuf));
    qDebug() << message << errbuf << errorCode;
}

