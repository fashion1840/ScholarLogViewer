#include "configurationwidget.h"
#include "ui_configurationwidget.h"

ConfigurationWidget::ConfigurationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigurationWidget)
{
    ui->setupUi(this);
}

ConfigurationWidget::~ConfigurationWidget()
{
    delete ui;
}
