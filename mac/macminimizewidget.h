#ifndef MACMINIMIZEWIDGET_H
#define MACMINIMIZEWIDGET_H

#include <QWidget>

class MacMinimizeWidget
{
public:
    static void AllowMinimizeForFramelessWindow(QWidget *window);
};

#endif // MACMINIMIZEWIDGET_H
