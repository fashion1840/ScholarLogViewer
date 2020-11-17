#include "waitingwidget.h"
#include "ui_waitingwidget.h"

#include <QMovie>

WaitingWidget::WaitingWidget(QWidget *parent) : QWidget(parent), ui(new Ui::WaitingWidget), pMove(nullptr)
{
    ui->setupUi(this);

    //无边框和标题栏
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);
}

WaitingWidget::~WaitingWidget()
{
    if (pMove)
    {
        pMove->stop();
        delete pMove;
    }

    delete ui;
}

void WaitingWidget::loading(WaitTipsEnum type)
{
    QString img;
    switch (type)
    {
        case LOADING_MOVIE:
            img = ":/images/gif/loading.gif";
            break;
        case SEACHING_MOVIE:
            img = ":/images/gif/seach.gif";
            break;
    }

    ui->labMovie->setStyleSheet("background-color: transparent;");
    pMove = new QMovie(img);
    ui->labMovie->setMovie(pMove);
    ui->labMovie->setScaledContents(true);
    pMove->start();
}
