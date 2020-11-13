#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <QApplication>
#include <QFile>

class CommonHelper
{
public:
    static void setStyle(const QString &styleFile)
    {
        QFile qss(styleFile);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};
#endif // COMMONHELPER_H
