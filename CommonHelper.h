#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <QApplication>
#include <QFile>
#include <QJsonDocument>

class CommonHelper
{
public:
    static void loadQss(const QString &styleFile)
    {
        QFile qss(styleFile);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    static bool stringToJsonFormat(const QString &str, QString &jsonString)
    {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError)
            return false;

        jsonString = doc.toJson(QJsonDocument::Indented);

        return true;
    }
};
#endif // COMMONHELPER_H
