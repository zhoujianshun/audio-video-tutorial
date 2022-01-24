#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include "mypushbutton.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("我的");

    QPushButton *btn = new QPushButton;
    btn->setFixedSize(100,30);
    btn->setText("登录");
    btn->move(100,200);
    btn->setParent(this);
    connect(btn, &QPushButton::clicked,this, &MainWindow::on_login_clicked);

    MyPushButton *myBtn = new MyPushButton(this);
    myBtn->setText("关闭");
    connect(myBtn, &QPushButton::clicked,this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_login_clicked()
{
    qDebug() << "on_login_clicked";
}


void MainWindow::on_pushButton_clicked()
{
    qDebug() << "on_pushButton_clicked";
}

