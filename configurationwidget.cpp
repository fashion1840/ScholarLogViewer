#include "configurationwidget.h"
#include "ui_configurationwidget.h"

ConfigurationWidget::ConfigurationWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ConfigurationWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
}

ConfigurationWidget::~ConfigurationWidget()
{
    delete ui;
}

void ConfigurationWidget::on_btnConfigClose_clicked()
{
    this->close();
}
