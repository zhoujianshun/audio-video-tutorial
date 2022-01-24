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

PlayThread::PlayThread(QObject *parent) : QObject(parent)
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

//void PlayThread::run() {
//    // 像素数据
//    SDL_Surface *surface = nullptr;

//    // 窗口
//    SDL_Window *window = nullptr;

//    // 渲染上下文
//    SDL_Renderer *renderer = nullptr;

//    // 纹理（直接跟特定驱动程序相关的像素数据）
//    SDL_Texture *texture = nullptr;

//    // 矩形框
////    SDL_Rect srcRect = {0, 0, 512, 512};
////    SDL_Rect dstRect = {200, 200, 100, 100};
//    SDL_Rect rect;

//    // 初始化子系统
//    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

//    // 加载BMP
//    surface = SDL_LoadBMP("/Users/zhoujianshun/Downloads/yuv/1.bmp");
//    END(!surface, SDL_LoadBMP);

//    // 创建窗口
//    window = SDL_CreateWindow(
//                 // 标题
//                 "SDL显示BMP图片",
//                 // x
//                 SDL_WINDOWPOS_UNDEFINED,
//                 // y
//                 SDL_WINDOWPOS_UNDEFINED,
//                 // w
//                 surface->w,
//                 // h
//                 surface->h,
//                 SDL_WINDOW_SHOWN
//             );
//    END(!window, SDL_CreateWindow);

//    // 创建渲染上下文
//    renderer = SDL_CreateRenderer(window, -1,
//                                  SDL_RENDERER_ACCELERATED |
//                                  SDL_RENDERER_PRESENTVSYNC);
//    if (!renderer) {
//        renderer = SDL_CreateRenderer(window, -1, 0);
//        END(!renderer, SDL_CreateRenderer);
//    }

//    // 创建纹理
//    texture = SDL_CreateTextureFromSurface(renderer, surface);
//    END(!texture, SDL_CreateTextureFromSurface);

//    // 画一个红色的矩形框
//    END(SDL_SetRenderDrawColor(renderer,
//                               255, 0, 0, SDL_ALPHA_OPAQUE),
//        SDL_SetRenderDrawColor);
//    rect = {0, 0, 50, 50};
//    END(SDL_RenderFillRect(renderer, &rect),
//        SDL_RenderFillRect);

//    // 设置绘制颜色（画笔颜色）
//    END(SDL_SetRenderDrawColor(renderer,
//                               255, 255, 0, SDL_ALPHA_OPAQUE),
//        SDL_SetRenderDrawColor);

//    // 用绘制颜色（画笔颜色）清除渲染目标
//    END(SDL_RenderClear(renderer),
//        SDL_RenderClear);

//    // 拷贝纹理数据到渲染目标（默认是window）
//    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr),
//        SDL_RenderCopy);

//    // 更新所有的渲染操作到屏幕上
//    SDL_RenderPresent(renderer);

//    SDL_Delay(2000);

//end:
//    SDL_FreeSurface(surface);
//    SDL_DestroyTexture(texture);
//    SDL_DestroyRenderer(renderer);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//}

void PlayThread::run()
{
    SDL_Surface *surface = nullptr;
    SDL_Window *window = nullptr;

    SDL_Renderer *renderer = nullptr;

    SDL_Texture *texture = nullptr;
    int ret = -1;
    SDL_Rect srcRect = {0, 0, 1280, 766};
    SDL_Rect dstRect = {0, 0, 640, 383};
    SDL_Rect rect;
    // 初始化子系统
    // 不能在子线程，在子线程会奔溃
    ret = SDL_Init(SDL_INIT_VIDEO);
    ERROR_END(ret != 0, "Unable to initialize SDL: ");

    surface = SDL_LoadBMP("/Users/zhoujianshun/Downloads/yuv/1.bmp");
    ERROR_END(!surface, "Unable to SDL_LoadBMP: ");
    qDebug() << surface->w << surface->h;
    window = SDL_CreateWindow("show bmp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, surface->w, surface->h, SDL_WINDOW_SHOWN);
    ERROR_END(!window, "Unable to SDL_CreateWindow: ");

     srcRect = {0, 0, surface->w, surface->h};
     dstRect = {surface->w/4, surface->h/4, srcRect.w/2, srcRect.h/2};

    // 创建渲染上下文
    renderer = SDL_CreateRenderer(window, - 1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer){
        renderer =  SDL_CreateRenderer(window, - 1, 0);
        ERROR_END(!renderer, "Unable to SDL_CreateRenderer: ");
    }

    // 创建纹理
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    ERROR_END(!texture, "Unable to SDL_CreateTextureFromSurface: ");




    // 设置绘制颜色（画笔颜色）
    ERROR_END(SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE), "Unable to SDL_SetRenderDrawColor:");

    // 用绘制颜色（画笔颜色）清除渲染目标
    ERROR_END(SDL_RenderClear(renderer), "Unable to SDL_RenderClear:");



    // 画一个红色的矩形框
    END(SDL_SetRenderDrawColor(renderer,
                               255, 0, 0, SDL_ALPHA_OPAQUE),
        SDL_SetRenderDrawColor);
    rect = {0, 0, 50, 50};
    END(SDL_RenderFillRect(renderer, &rect),
        SDL_RenderFillRect);

    // 拷贝纹理到渲染目标，默认是window
    ret = SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
    ERROR_END(ret != 0, "Unable to SDL_RenderCopy: ");

    SDL_RenderPresent(renderer);

//    SDL_Delay(2000);
    qDebug() << "播放完成 ";
end:

    qDebug() << "播放完成 ";

//    if (texture){
//        SDL_DestroyTexture(texture);
//    }
//    if (renderer){
//        SDL_DestroyRenderer(renderer);
//    }

//    //
//    if(surface){
//        SDL_FreeSurface(surface);
//    }
//    if(window){
//          SDL_DestroyWindow(window);
//    }

//    // 关闭子系统
//    SDL_Quit();

}
