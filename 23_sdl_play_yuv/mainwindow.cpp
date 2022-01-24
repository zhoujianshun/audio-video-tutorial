#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        return; \
    }

#define F_WDITH 1280
#define F_HEIGHT 720




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _widget = new QWidget(this);
    _widget->setGeometry(100, 0, 1280, 766);


    file = new QFile("/Users/zhoujianshun/Downloads/yuv/2.yuv");


//    SDL_Rect src = {0,0,width, height};
//    SDL_Rect dst = {0,0,width, height};
    // 初始化子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    qDebug() << ui->label->winId() << _widget->winId();
    // 创建窗口
    window = SDL_CreateWindowFrom((void *)_widget->winId());

    // 创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    // 创建纹理
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, F_WDITH, F_HEIGHT);
    END(!texture, SDL_CreateTextureFromSurface);


    if (!file->open(QFile::ReadOnly)){
        qDebug() << "打开文件失败";
        return;
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    file->close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

/**
SDL播放音频两种方式：
Push（推）：【程序】主动推送数据给【音频设备】
Pull（拉）：【音频设备】主动向【程序】拉去数据
*/
void MainWindow::on_playButton_clicked()
{
   timeId = startTimer(33);
}

void MainWindow::timerEvent(QTimerEvent *event){

    int size = F_WDITH * F_HEIGHT * 1.5;
    char data[size];
    int ret = file->read(data, size);
    if( ret == 0 ){
        killTimer(timeId);
        qDebug() << "播放结束";
        return;
    }

    // 将YUV数据填充到渲染目标
     END(SDL_UpdateTexture(texture, nullptr, data, F_WDITH) != 0, SDL_UpdateTexture);

    // 设置绘制颜色（画笔颜色）
    END(SDL_SetRenderDrawColor(renderer,
                               255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);

    // 用绘制颜色（画笔颜色）清除渲染目标
    END(SDL_RenderClear(renderer),
        SDL_RenderClear);

    // 拷贝纹理数据到渲染目标（默认是window）
    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
        SDL_RenderCopy);

//    END(SDL_RenderCopy(renderer, texture, &srcRect, &dstRect), SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);
}
