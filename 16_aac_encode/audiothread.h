#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

class AudioThread : public QThread
{
    Q_OBJECT

    ~AudioThread();

public:
    explicit AudioThread(QObject *parent = nullptr);
    void run();

private:
    void printErrorMessage(const char *message, int errorCode);

signals:
    // 自定义信号
    void startSignal();
    void endSignal();
};

#endif // AUDIOTHREAD_H
