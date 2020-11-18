#ifndef CONFIGURATIONWIDGET_H
#define CONFIGURATIONWIDGET_H

#include <QWidget>

namespace Ui {
class ConfigurationWidget;
}

class ConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigurationWidget(QWidget *parent = nullptr);
    ~ConfigurationWidget();

private:
    Ui::ConfigurationWidget *ui;
};

#endif // CONFIGURATIONWIDGET_H
