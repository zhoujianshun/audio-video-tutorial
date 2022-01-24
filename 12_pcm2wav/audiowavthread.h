#ifndef AUDIOWAVTHREAD_H
#define AUDIOWAVTHREAD_H

#include <QThread>

/// 直接输出wav
class AudioWavThread : public QThread
{
    Q_OBJECT

    ~AudioWavThread();

public:
    explicit AudioWavThread(QObject *parent = nullptr);
    void run();

private:
    void printErrorMessage(const char *message, int errorCode);

signals:
    // 自定义信号
    void startSignal();
    void endSignal();
};

#endif // AUDIOWAVTHREAD_H
