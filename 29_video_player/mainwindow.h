#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "videoplayer.h"


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
    void on_stopBtn_clicked();

    void on_playBtn_clicked();

    void on_openFileBtn_clicked();

    void on_volumnSlider_valueChanged(int value);

    void on_currentSlider_valueChanged(int value);

    void onPlayerStateChanged(VideoPlayer *player);
    void onInitFinished(VideoPlayer *player);
    void onPlayerFailed(VideoPlayer *player);
    void onPlayerTimeChanged(VideoPlayer *player);

    void on_silenceBtn_clicked();

    void on_currentSlider_sliderPressed();

    void on_currentSlider_sliderMoved(int position);

    void on_currentSlider_sliderReleased();


private:
    Ui::MainWindow *ui;
    VideoPlayer *_player;
    QString getTimeText(int64_t time);
    bool continuePlay = false;
    bool _index = -1;

};
#endif // MAINWINDOW_H
