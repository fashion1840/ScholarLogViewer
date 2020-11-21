#ifndef CONFIGURATIONWIDGET_H
#define CONFIGURATIONWIDGET_H

#include <QWidget>

namespace Ui
{
    class ConfigurationWidget;
}

class ConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigurationWidget(QWidget *parent = nullptr);
    ~ConfigurationWidget();

private slots:

    void on_btnConfigClose_clicked();

private:
    Ui::ConfigurationWidget *ui;
};

#endif // CONFIGURATIONWIDGET_H
