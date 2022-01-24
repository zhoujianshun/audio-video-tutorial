#ifndef CONDMUTEX_H
#define CONDMUTEX_H

#include <SDL2/SDL.h>


class CondMutex
{
public:
    CondMutex();
    ~CondMutex();

    void lock();
    void unlock();
    void tryLock();
    void wait();
    void signal();
    void broaddcast();



private:
    SDL_mutex *_mutex;
    SDL_cond *_cond;
};

#endif // CONDMUTEX_H
