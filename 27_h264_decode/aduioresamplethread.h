#ifndef ADUIORESAMPLETHREAD_H
#define ADUIORESAMPLETHREAD_H

#include <QThread>

class AduioResampleThread : public QThread
{
    Q_OBJECT

    ~AduioResampleThread();

public:
    explicit AduioResampleThread(QObject *parent = nullptr);
    void run();

};

#endif // ADUIORESAMPLETHREAD_H
