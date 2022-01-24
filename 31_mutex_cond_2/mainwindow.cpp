#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _list = new std::list<QString>();

    _mutex = SDL_CreateMutex();
    _cond = SDL_CreateCond();
    _cond2 = SDL_CreateCond();

    consume("消费者1");
    consume("消费者2");
    consume("消费者3");

}

MainWindow::~MainWindow()
{
    delete ui;

    delete _list;

    SDL_DestroyMutex(_mutex);
    SDL_DestroyCond(_cond);
    SDL_DestroyCond(_cond2);
}


void MainWindow::on_pushButton_clicked()
{
    produce("生产者1");
    produce("生产者2");
    produce("生产者3");
}

void MainWindow::consume(QString name){
    // 创建消费者
    std::thread([this, name](){
       SDL_LockMutex(_mutex);

       while (true) {
           qDebug() << name << "消费者拿到锁";
           while (!_list->empty()) {
               qDebug()<< name << "消费者消费了：" << _list->front();
               _list->pop_front();
//                SDL_UnlockMutex(_mutex);
               std::this_thread::sleep_for(std::chrono::milliseconds(500));
//               SDL_LockMutex(_mutex);
           }
           qDebug() << name << "消费者开始等待" ;
           SDL_CondSignal(_cond2);

           // 释放锁，开始等待条件
           // 等到了条件，加锁
           SDL_CondWait(_cond, _mutex);
       }

        SDL_UnlockMutex(_mutex);
    }).detach();
}

void MainWindow::produce(QString name){
    std::thread([this, name](){
        // 生产者
        SDL_LockMutex(_mutex);
        while (true) {
            qDebug() << name << "开始生产";
            _list->push_back(QString("%1").arg(++_index));
            _list->push_back(QString("%1").arg(++_index));
            _list->push_back(QString("%1").arg(++_index));

            SDL_CondSignal(_cond);
            SDL_CondWait(_cond2, _mutex);
        }

        SDL_UnlockMutex(_mutex);
    }).detach();
}

