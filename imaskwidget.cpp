#include "imaskwidget.h"
#include "ui_imaskwidget.h"

#include <QPainter>

IMaskWidget::IMaskWidget(QWidget *parent) : QWidget(parent), ui(new Ui::IMaskWidget)
{
    ui->setupUi(this);

    //设置窗口全透明
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
}

IMaskWidget::~IMaskWidget()
{
    delete ui;
}

void IMaskWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    //创建绘画对象
    QPainter painter(this);

    //为窗口添加一个半透明的矩形遮罩
    painter.fillRect(this->rect(), QColor(0, 0, 0, 100));
}
