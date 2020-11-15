#include "actionlogprocessor.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

ActionLogProcessor::ActionLogProcessor() {}

ActionLogProcessor::~ActionLogProcessor() {}

bool ActionLogProcessor::openLogFile(const QString &logName)
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

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        lastErrorMsg = "Failed to opened.";
        return false;
    }

    logLineList.clear();

    while (!fp.atEnd())
    {
        QByteArray txt = fp.readLine().trimmed();
        if (txt.isEmpty())
            continue;
        logLineList << txt;
    }

    fp.close();

    cleanData(logLineList);

    return true;
}

QString ActionLogProcessor::getFileName()
{
    return logFileName;
}

QString ActionLogProcessor::getFilePath()
{
    return logFilePath;
}

LogFileTypeEnum ActionLogProcessor::getLogType()
{
    return logType;
}

QList<QString> &ActionLogProcessor::getLogRecordList()
{
    return logLineList;
}

bool ActionLogProcessor::getItemRecord(QList<LogRecordStruct> &recordItemsList)
{
    if (logLineList.isEmpty())
    {
        lastErrorMsg = "Null";
        return false;
    }

    //通过第一行log判断，分离出来的字段数不为5的判定为不支持的日志格式文件
    if (logLineList[0].split(ACTION_STRING_SEPARATOR).size() < ACTION_LOG_ITEM_SIZE)
    {
        lastErrorMsg = "Is not support format file.";
        return false;
    }

    for (int i = 0; i < logLineList.size(); i++)
    {
        QStringList list = logLineList.at(i).split(ACTION_STRING_SEPARATOR);
        if (list.size() < ACTION_LOG_ITEM_SIZE)
        {
            qInfo() << "Error record:" << logLineList.at(i);
            continue;
        }

        struct LogRecordStruct item;

        item.time = list[0].section("|", 0, 0).trimmed();
        item.type = list[1].trimmed();
        item.id = list[3].trimmed();
        item.name = list[5];
        item.number = "0";
        if (list.size() > ACTION_LOG_ITEM_SIZE)
            item.data = logLineList.at(i).section(ACTION_STRING_SEPARATOR, 6);
        else
            item.data = list[6].trimmed();

        recordItemsList.append(item);
    }

    return true;
}

QString ActionLogProcessor::getLastError() const
{
    return lastErrorMsg;
}

TypeRecordMap &ActionLogProcessor::getRecordTypeMap()
{
    return recordTypeMap;
}

void ActionLogProcessor::cleanData(QList<QString> &list)
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
