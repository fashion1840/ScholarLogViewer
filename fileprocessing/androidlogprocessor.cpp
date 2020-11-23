#include "androidlogprocessor.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

AndroidLogProcessor::AndroidLogProcessor() {}

AndroidLogProcessor::~AndroidLogProcessor() {}

bool AndroidLogProcessor::openLogFile(const QString &logName)
{
    QFile fp(logName);
    if (!fp.exists())
    {
        lastErrorMsg = "The open file was not exist.";
        return false;
    }

    //文件路径信息
    QFileInfo info(logName);
    logFileName = info.fileName();
    logFilePath = logName;
    logFileDir = info.path();

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        lastErrorMsg = "Failed to opened.";
        return false;
    }

    logLineList.clear();
    recordTypeMap.clear();

    while (!fp.atEnd())
    {
        QByteArray txt = fp.readLine().trimmed();
        if (txt.isEmpty())
            continue;
        logLineList << txt;

        //TODO:防止界面疆死
        QCoreApplication::processEvents();
    }

    fp.close();

    cleanData(logLineList);

    return true;
}

QString AndroidLogProcessor::getFileName()
{
    return logFileName;
}

QString AndroidLogProcessor::getFilePath()
{
    return logFilePath;
}

QString AndroidLogProcessor::getFileDir()
{
    return logFileDir;
}

LogFileTypeEnum AndroidLogProcessor::getLogType()
{
    return logType;
}

QList<QString> &AndroidLogProcessor::getLogRecordList()
{
    return logLineList;
}

bool AndroidLogProcessor::getItemRecord(QList<QStringList> &recordItemsList)
{
    if (logLineList.isEmpty())
    {
        lastErrorMsg = "Null";
        return false;
    }

    QStringList temp = logLineList[0].split(ANDROID_STRING_SEPARATOR, QString::SkipEmptyParts);
    int size = temp.size();

    //通过第一行log判断，分离出来的字段数不为5的判定为不支持的日志格式文件
    if (logLineList[0].split(ANDROID_STRING_SEPARATOR, QString::SkipEmptyParts).size() < ANDROID_LOG_ITEM_SIZE)
    {
        lastErrorMsg = "Is not support format file.";
        return false;
    }

    for (int i = 0; i < logLineList.size(); i++)
    {
        QString str = logLineList.at(i);
        QStringList list = str.split(ANDROID_STRING_SEPARATOR, QString::SkipEmptyParts);
        if (list.size() < ANDROID_LOG_ITEM_SIZE)
        {
            qInfo() << "Error record:" << logLineList.at(i);
            continue;
        }

        QStringList item;

        item << QString("%1 %2").arg(list[0]).arg(list[2]);
        item << list[3].mid(1, 1);

        int idx = str.indexOf(list[3]);
        QStringList subList = str.mid(idx + list[3].size() + 1).split("\t");
        item << subList[0].remove("[").remove("]");
        item << subList[1].remove("[").remove("]");
        item << subList[2];
        item << subList[3];

        recordItemsList.append(item);

        //TODO:防止界面疆死
        QCoreApplication::processEvents();

        creatLogTypeInfo(item);
    }

    return true;
}

QString AndroidLogProcessor::getLastError() const
{
    return lastErrorMsg;
}

TypeRecordMap &AndroidLogProcessor::getRecordTypeMap()
{
    return recordTypeMap;
}

void AndroidLogProcessor::cleanData(QList<QString> &list)
{
    if (list.isEmpty())
        return;

    //过滤多个日志文件合并后添加的一些注释信息
    while (true)
    {
        if (list.first().startsWith("Log Merge From:") || list.first().startsWith("TS:"))
            list.removeFirst();
        else
            break;
    }

    //清理二次处理过的平台日志文件
    if (list.first().startsWith("<textarea"))
        list.removeFirst();

    if (list.last().startsWith("</textarea>"))
        list.removeLast();
}

void AndroidLogProcessor::creatLogTypeInfo(const QStringList &record)
{
    QString type = record.at(1);
    if (type == "I")
    {
        if (recordTypeMap.contains("Info"))
            recordTypeMap["Info"].append(record);
        else
        {
            QList<QStringList> value;
            value.append(record);
            recordTypeMap.insert("Info", value);
        }
    }
    else if (type == "D")
    {
        if (recordTypeMap.contains("Debug"))
            recordTypeMap["Debug"].append(record);
        else
        {
            QList<QStringList> value;
            value.append(record);
            recordTypeMap.insert("Debug", value);
        }
    }
    else if (type == "W")
    {
        if (recordTypeMap.contains("Warning"))
            recordTypeMap["Warning"].append(record);
        else
        {
            QList<QStringList> value;
            value.append(record);
            recordTypeMap.insert("Warning", value);
        }
    }
    else if (type == "E")
    {
        if (recordTypeMap.contains("Error"))
            recordTypeMap["Error"].append(record);
        else
        {
            QList<QStringList> value;
            value.append(record);
            recordTypeMap.insert("Error", value);
        }
    }
}
