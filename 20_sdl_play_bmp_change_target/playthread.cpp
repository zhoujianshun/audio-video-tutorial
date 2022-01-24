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
    quitSDL();
    gameOver = true;
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

SDL_Texture *createTexture(SDL_Renderer *renderer){


   SDL_Texture *texture =  SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, 50, 50);

   if (!texture){ return nullptr; }

   // 设置纹理为渲染目标
   if (SDL_SetRenderTarget(renderer, texture) != 0) return nullptr;

   // 设置画笔颜色
   if (SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE) != 0) return nullptr;

   SDL_Rect rect = {0,0,50,50 };
   if (SDL_RenderDrawRect(renderer, &rect) != 0) return nullptr;
   if (SDL_RenderDrawLine(renderer,0, 0, 50, 50) != 0) return nullptr;
   if (SDL_RenderDrawLine(renderer,0, 50, 50, 0) != 0) return nullptr;

   return texture;
}

void PlayThread::quitSDL(){

        if (texture){
            SDL_DestroyTexture(texture);
        }
        if (renderer){
            SDL_DestroyRenderer(renderer);
        }

        //
//        if(surface){
//            SDL_FreeSurface(surface);
//        }
        if(window){
           SDL_DestroyWindow(window);
        }

        // 关闭子系统
        SDL_Quit();
}

void PlayThread::showClick(SDL_Event &event){
    SDL_MouseButtonEvent button = event.button;
    int x = button.x;
    int y = button.y;
    int w = 0;
    int h = 0;
    qDebug() << x << y;

    if (SDL_QueryTexture(texture,nullptr, nullptr,&w,&h) != 0){
         qDebug() << "SDL_QueryTexture error";
        return;
    }

//    SDL_RenderClear(renderer);
    SDL_Rect dstRect = { x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

    SDL_RenderPresent(renderer);
}
void PlayThread::listenSDLEvents(){
//    bool gameOver = false;
    while( gameOver == false ){
        SDL_Event gameEvent;
        while ( SDL_PollEvent(&gameEvent) != 0 ){  /*　SDL_PollEvent(&gameEvent);  if(&gameEvent != 0 ){　*/
            // window的关闭按钮
            if ( gameEvent.type == SDL_QUIT ){
                gameOver = true;
            }else if(gameEvent.type == SDL_MOUSEBUTTONUP){
                showClick(gameEvent);
            }

            //esc按钮
            if ( gameEvent.type == SDL_KEYUP ){   /* if ( gameEvent.type == SDL_KEYDOWN ） */
                if ( gameEvent.key.keysym.sym == SDLK_ESCAPE ){
                    gameOver = true;
                }
            }
        }
        // do something in loop
//        qDebug()<< "loop";
    }
    quitSDL();
    return;

}

void PlayThread::run()
{

//    SDL_Window *window = nullptr;

//    SDL_Renderer *renderer = nullptr;

//    SDL_Texture *texture = nullptr;
    int ret = -1;

    SDL_Rect dstRect = {100,100,50,50};
    SDL_Rect rect;
    // 初始化子系统
    // 不能在子线程，在子线程会奔溃
    ret = SDL_Init(SDL_INIT_VIDEO);
    ERROR_END(ret != 0, "Unable to initialize SDL: ");


    window = SDL_CreateWindow("show bmp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 766, SDL_WINDOW_SHOWN);
    ERROR_END(!window, "Unable to SDL_CreateWindow: ");


    // 创建渲染上下文
    renderer = SDL_CreateRenderer(window, - 1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer){
        renderer =  SDL_CreateRenderer(window, - 1, 0);
        ERROR_END(!renderer, "Unable to SDL_CreateRenderer: ");
    }

    // 创建纹理
    texture = createTexture(renderer);
    if(!texture){
        qDebug() << "createTexture error";
    }
    ERROR_END(SDL_SetRenderTarget(renderer, nullptr), "Unable to SDL_SetRenderTarget:");
    // 设置绘制颜色（画笔颜色）
    ERROR_END(SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE), "Unable to SDL_SetRenderDrawColor:");

    // 用绘制颜色（画笔颜色）清除渲染目标
    ERROR_END(SDL_RenderClear(renderer), "Unable to SDL_RenderClear:");


    // 画一个红色的矩形框
//    ERROR_END(SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE), "Unable to SDL_SetRenderDrawColor:");
//    rect = {0, 0, 50, 50};
//    ERROR_END(SDL_RenderFillRect(renderer, &rect), "Unable to SDL_RenderFillRect:");


    //拷贝纹理到渲染目标
    ERROR_END(SDL_RenderCopy(renderer, texture, nullptr, &dstRect)!=0, "Unable to SDL_RenderCopy:");

    SDL_RenderPresent(renderer);

    // 监听事件
    listenSDLEvents();

//    SDL_Delay(2000);
    qDebug() << "播放完成 ";
end:

    qDebug() << "播放完成 ";

}




