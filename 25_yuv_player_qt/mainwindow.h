#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "yuvplayer.h"

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
    void on_stopButton_clicked();
    void on_playStateChanged();

    void on_nextButton_clicked();

private:
     Ui::MainWindow *ui;

     YUVPlayer *_yuvPlayer;
     int _index = 0;
};
#endif // MAINWINDOW_H