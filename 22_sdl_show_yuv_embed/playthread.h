#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>

class PlayThread : public QObject
{
    Q_OBJECT
private:
    void *_winId;

public:
    explicit PlayThread(void *winId, QObject *parent = nullptr);
    ~PlayThread();
    void run();
    void start();
signals:

};

#endif // PLAYTHREAD_H
