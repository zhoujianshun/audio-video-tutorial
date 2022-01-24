#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QThread>

class DecodeThread : public QThread
{
    Q_OBJECT

    ~DecodeThread();

public:
    explicit DecodeThread(QObject *parent = nullptr);
    void run();

private:
    void printErrorMessage(const char *message, int errorCode);

signals:
    // 自定义信号
    void startSignal();
    void endSignal();
};

#endif // DECODETHREAD_H
