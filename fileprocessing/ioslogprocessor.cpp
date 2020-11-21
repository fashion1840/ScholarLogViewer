#include "ioslogprocessor.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

IosLogProcessor::IosLogProcessor() {}

IosLogProcessor::~IosLogProcessor() {}

bool IosLogProcessor::openLogFile(const QString &logName)
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

QString IosLogProcessor::getFileName()
{
    return logFileName;
}

QString IosLogProcessor::getFilePath()
{
    return logFilePath;
}

QString IosLogProcessor::getFileDir()
{
    return logFileDir;
}

LogFileTypeEnum IosLogProcessor::getLogType()
{
    return logType;
}

QList<QString> &IosLogProcessor::getLogRecordList()
{
    return logLineList;
}

bool IosLogProcessor::getItemRecord(QList<LogRecordStruct> &recordItemsList)
{
    if (logLineList.isEmpty())
    {
        lastErrorMsg = "Null";
        return false;
    }

    //通过第一行log判断，分离出来的字段数不为5的判定为不支持的日志格式文件
    if (logLineList[0].split(IOS_STRING_SEPARATOR, QString::SkipEmptyParts).size() < IOS_LOG_ITEM_SIZE)
    {
        lastErrorMsg = "Is not support format file.";
        return false;
    }

    for (int i = 0; i < logLineList.size(); i++)
    {
        QString str = logLineList.at(i);
        QStringList list = str.split(IOS_STRING_SEPARATOR, QString::SkipEmptyParts);
        if (list.size() < IOS_LOG_ITEM_SIZE)
        {
            qInfo() << "Error record:" << logLineList.at(i);
            continue;
        }

        struct LogRecordStruct item;
        item.time = QString("%1 %2").arg(list[3]).arg(list[4]);
        QString t = list[1].section("|", 1);
        item.type = t.remove(":");
        if (item.type.isEmpty())
            continue;
        item.id = list[5];
        item.name = list[6];
        item.number = list[7];
        item.data = str.section(" ", 8, -1, QString::SectionSkipEmpty);

        recordItemsList.append(item);

        //TODO:防止界面疆死
        QCoreApplication::processEvents();
    }

    creatLogTypeInfo(recordItemsList);

    return true;
}

QString IosLogProcessor::getLastError() const
{
    return lastErrorMsg;
}

TypeRecordMap &IosLogProcessor::getRecordTypeMap()
{
    return recordTypeMap;
}

void IosLogProcessor::cleanData(QList<QString> &list)
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

void IosLogProcessor::creatLogTypeInfo(const QList<LogRecordStruct> &recordList)
{
    for (auto item : recordList)
    {
        recordTypeMap[item.type].append(item);
    }
}
