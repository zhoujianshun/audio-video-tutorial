#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playthread.h"

#include <SDL2/SDL.h>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();


private:
     Ui::MainWindow *ui;
     QWidget *_widget;
     // 窗口
     SDL_Window *window = nullptr;
     // 渲染上下文
     SDL_Renderer *renderer = nullptr;
     // 纹理（直接跟特定驱动程序相关的像素数据）
     SDL_Texture *texture = nullptr;
     QFile *file = nullptr;

     void timerEvent(QTimerEvent *event);
      int timeId;

};
#endif // MAINWINDOW_H
