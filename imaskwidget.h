#ifndef IMASKWIDGET_H
#define IMASKWIDGET_H

#include <QWidget>

namespace Ui
{
    class IMaskWidget;
}

class IMaskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IMaskWidget(QWidget *parent = nullptr);
    ~IMaskWidget();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::IMaskWidget *ui;
};

#endif // IMASKWIDGET_H
