#include "actionlogprocessor.h"

#include <QDateTime>
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
    logFileDir = info.path();

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

QString ActionLogProcessor::getFileDir()
{
    return logFileDir;
}

LogFileTypeEnum ActionLogProcessor::getLogType()
{
    return logType;
}

QList<QString> &ActionLogProcessor::getLogRecordList()
{
    return logLineList;
}

bool ActionLogProcessor::getItemRecord(QList<QStringList> &recordItemsList)
{
    if (logLineList.isEmpty())
    {
        lastErrorMsg = "Null";
        return false;
    }

    //通过第一行log判断，分离出来的字段数不为5的判定为不支持的日志格式文件
    int listSize = logLineList[0].split(ACTION_STRING_SEPARATOR).size();
    if (listSize < ACTION_LOG_ITEM_SIZE_FIRST)
    {
        lastErrorMsg = "Is not support format file.";
        return false;
    }

    for (int i = 0; i < logLineList.size(); i++)
    {
        QStringList list = logLineList.at(i).split(ACTION_STRING_SEPARATOR);
        bool isExpand = false;
        if (list[0].contains("|"))
            isExpand = true;

        if (isExpand)
        {
            if (list.size() < ACTION_LOG_ITEM_SIZE_SECOND)
            {
                qInfo() << "Error record:" << logLineList.at(i);
                continue;
            }
        }
        else
        {
            if (list.size() < ACTION_LOG_ITEM_SIZE_FIRST)
            {
                qInfo() << "Error record:" << logLineList.at(i);
                continue;
            }
        }

        QStringList item;

        if (isExpand)
        {
            item << list[0].section("|", 0, 0).trimmed();
            item << list[1].trimmed();
            item << list[3].trimmed();
            item << list[5];
            if (list.size() > ACTION_LOG_ITEM_SIZE_SECOND)
                item << logLineList.at(i).section(ACTION_STRING_SEPARATOR, 6);
            else
                item << list[6].trimmed();
        }
        else
        {
            QDateTime time = QDateTime::fromMSecsSinceEpoch(list[3].toLongLong()); //时间戳-毫秒级
            QString strStartTime = time.toString("yyyy-mm-dd hh:mm:ss");
            item << strStartTime;
            item << list[0].trimmed();
            item << list[2].trimmed();
            item << list[4].trimmed();
            if (list.size() > ACTION_LOG_ITEM_SIZE_FIRST)
                item << logLineList.at(i).section(ACTION_STRING_SEPARATOR, 5);
            else
                item << list[5].trimmed();
        }

        recordItemsList.append(item);
    }

    creatLogTypeInfo(recordItemsList);

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

void ActionLogProcessor::creatLogTypeInfo(const QList<QStringList> &recordList)
{
    for (auto item : recordList)
    {
        recordTypeMap[item.at(1)].append(item);
    }
}
