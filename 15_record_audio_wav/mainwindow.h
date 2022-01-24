#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "audiowavthread.h"

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
    void on_audioButton_clicked();
    void on_start_record();
    void on_end_record();
    void on_timeout();
    void on_time_changed(unsigned long long ms);

private:
    Ui::MainWindow *ui;
    AudioWavThread *audioThread = nullptr;
    QTimer *timer = nullptr;
    int count = 0;


};
#endif // MAINWINDOW_H
