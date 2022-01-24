#include "condmutex.h"

CondMutex::CondMutex()
{
    _mutex = SDL_CreateMutex();
    _cond = SDL_CreateCond();
}

CondMutex::~CondMutex()
{
    SDL_DestroyMutex(_mutex);
    SDL_DestroyCond(_cond);
}

void CondMutex::lock()
{
    SDL_LockMutex(_mutex);
}

void CondMutex::unlock()
{
    SDL_UnlockMutex(_mutex);
}

void CondMutex::tryLock()
{
    SDL_TryLockMutex(_mutex);
}

void CondMutex::wait()
{
    SDL_CondWait(_cond, _mutex);
}

void CondMutex::signal()
{
    SDL_CondSignal(_cond);
}

void CondMutex::broaddcast()
{
    SDL_CondBroadcast(_cond);
}

