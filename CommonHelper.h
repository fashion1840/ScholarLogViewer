#ifndef COMMONHELPER_H
#define COMMONHELPER_H

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <chrono>

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

    static qint64 getCurrentTimeOfMillisecond()
    {
        using namespace std;

        auto time_now = chrono::system_clock::now();
        auto duration_in_ms = chrono::duration_cast<chrono::milliseconds>(time_now.time_since_epoch());
        return duration_in_ms.count();
    }
};
#endif // COMMONHELPER_H
