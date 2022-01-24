#include "mainwindow.h"

#include <QApplication>

#include <QDebug>
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{

    // 初始化子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        return 0;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int ret = a.exec();

    SDL_Quit();

    return ret;
}
