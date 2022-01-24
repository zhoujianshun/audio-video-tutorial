#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <SDL2/SDL.h>

class PlayThread : public QObject
{
    Q_OBJECT
private:
    void listenSDLEvents ();
    void quitSDL();
    void showClick(SDL_Event &event);

    bool gameOver = false;
    SDL_Window *window = nullptr;

    SDL_Renderer *renderer = nullptr;

    SDL_Texture *texture = nullptr;

public:
    explicit PlayThread(QObject *parent = nullptr);
    ~PlayThread();
    void run();
    void start();
signals:

};

#endif // PLAYTHREAD_H
