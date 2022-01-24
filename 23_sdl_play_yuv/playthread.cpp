#include "playthread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

#define ERROR_END(flags, message) \
    if (flags) { \
        qDebug() << message << SDL_GetError(); \
        goto end; \
    }

#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        goto end; \
    }

PlayThread::PlayThread(void *winId, QObject *parent) : QObject(parent), _winId(winId)
{
//    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}


PlayThread::~PlayThread(){
    disconnect();
//    requestInterruption();
//    quit();
//    wait();
    qDebug() << this <<"PlayThread::~PlayThread()";
}
void PlayThread::start(){
    this->run();
}

void PlayThread::run() {
    // 像素数据

    // 窗口
    SDL_Window *window = nullptr;

    // 渲染上下文
    SDL_Renderer *renderer = nullptr;

    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *texture = nullptr;


    QFile file("/Users/zhoujianshun/Downloads/yuv/p420.yuv");

    int width = 1280;
    int height = 766;

    SDL_Rect src = {0,0,width, height};
    SDL_Rect dst = {0,0,width, height};
    // 初始化子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    // 创建窗口
    window = SDL_CreateWindowFrom(_winId);
//    END(!window, SDL_CreateWindow);

    // 创建渲染上下文
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    // 创建纹理
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    END(!texture, SDL_CreateTextureFromSurface);


    if (!file.open(QFile::ReadOnly)){
        qDebug() << "打开文件失败";
        goto end;
    }
    // 将YUV数据填充到渲染目标
    END(SDL_UpdateTexture(texture, nullptr, file.readAll().data(), width) != 0, SDL_UpdateTexture);


    // 设置绘制颜色（画笔颜色）
    END(SDL_SetRenderDrawColor(renderer,
                               255, 255, 0, SDL_ALPHA_OPAQUE),
        SDL_SetRenderDrawColor);

    // 用绘制颜色（画笔颜色）清除渲染目标
    END(SDL_RenderClear(renderer),
        SDL_RenderClear);

    // 拷贝纹理数据到渲染目标（默认是window）
    END(SDL_RenderCopy(renderer, texture, &src, &dst),
        SDL_RenderCopy);

//    END(SDL_RenderCopy(renderer, texture, &srcRect, &dstRect), SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);

//    SDL_Delay(6000);

end:
    file.close();
//    SDL_DestroyTexture(texture);
//    SDL_DestroyRenderer(renderer);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
      qDebug() << "打开文件失败";
}

