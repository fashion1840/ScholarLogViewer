#ifndef WAITINGWIDGET_H
#define WAITINGWIDGET_H

#include <QMovie>
#include <QWidget>

#include "UtilityDefine.h"
#include "imaskwidget.h"

namespace Ui
{
    class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingWidget(QWidget *parent = nullptr);
    ~WaitingWidget();

    void loading(WaitTipsEnum type);

private:
    Ui::WaitingWidget *ui;
    QMovie *pMove;
};

#endif // WAITINGWIDGET_H
