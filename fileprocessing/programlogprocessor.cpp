#include "programlogprocessor.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

ProgramLogProcessor::ProgramLogProcessor() {}

ProgramLogProcessor::~ProgramLogProcessor() {}

bool ProgramLogProcessor::openLogFile(const QString &logName)
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

QString ProgramLogProcessor::getFileName()
{
    return logFileName;
}

QString ProgramLogProcessor::getFilePath()
{
    return logFilePath;
}

QString ProgramLogProcessor::getFileDir()
{
    return logFileDir;
}

LogFileTypeEnum ProgramLogProcessor::getLogType()
{
    return logType;
}

QList<QString> &ProgramLogProcessor::getLogRecordList()
{
    return logLineList;
}

bool ProgramLogProcessor::getItemRecord(QList<LogRecordStruct> &recordItemsList)
{
    if (logLineList.isEmpty())
    {
        lastErrorMsg = "Null";
        return false;
    }

    //通过第一行log判断，分离出来的字段数不为5的判定为不支持的日志格式文件
    if (logLineList[0].split(PROGRAM_STRING_SEPARATOR).size() < PROGRAM_LOG_ITEM_SIZE)
    {
        lastErrorMsg = "Is not support format file.";
        return false;
    }

    for (int i = 0; i < logLineList.size(); i++)
    {
        QStringList list = logLineList.at(i).split(PROGRAM_STRING_SEPARATOR);
        if (list.size() < PROGRAM_LOG_ITEM_SIZE)
        {
            qInfo() << "Error record:" << logLineList.at(i);
            continue;
        }

        struct LogRecordStruct item;
        int startNum = 1;
        if (list[0].contains("|"))
            startNum = 2;

        item.time = list[0].section(" ", startNum);
        item.type = list[1].trimmed();
        item.id = list[2].trimmed();
        int idx = list[3].lastIndexOf(":");
        item.name = list[3].left(idx);
        item.number = list[3].right(list[3].size() - idx - 1);
        if (list.size() > PROGRAM_LOG_ITEM_SIZE)
            item.data = logLineList.at(i).section(PROGRAM_STRING_SEPARATOR, 4);
        else
            item.data = list[4].trimmed();

        recordItemsList.append(item);

        //TODO:防止界面疆死
        QCoreApplication::processEvents();

        creatLogTypeInfo(item);
    }

    return true;
}

QString ProgramLogProcessor::getLastError() const
{
    return lastErrorMsg;
}

TypeRecordMap &ProgramLogProcessor::getRecordTypeMap()
{
    return recordTypeMap;
}

void ProgramLogProcessor::cleanData(QList<QString> &list)
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

void ProgramLogProcessor::creatLogTypeInfo(const LogRecordStruct &record)
{

    if (record.type == "I")
    {
        if (recordTypeMap.contains("Info"))
            recordTypeMap["Info"].append(record);
        else
        {
            QList<struct LogRecordStruct> value;
            value.append(record);
            recordTypeMap.insert("Info", value);
        }
    }
    else if (record.type == "D")
    {
        if (recordTypeMap.contains("Debug"))
            recordTypeMap["Debug"].append(record);
        else
        {
            QList<struct LogRecordStruct> value;
            value.append(record);
            recordTypeMap.insert("Debug", value);
        }
    }
    else if (record.type == "W")
    {
        if (recordTypeMap.contains("Warning"))
            recordTypeMap["Warning"].append(record);
        else
        {
            QList<struct LogRecordStruct> value;
            value.append(record);
            recordTypeMap.insert("Warning", value);
        }
    }
    else if (record.type == "E")
    {
        if (recordTypeMap.contains("Error"))
            recordTypeMap["Error"].append(record);
        else
        {
            QList<struct LogRecordStruct> value;
            value.append(record);
            recordTypeMap.insert("Error", value);
        }
    }
}
