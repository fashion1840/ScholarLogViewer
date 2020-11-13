#ifndef UTILITYDEFINE_H
#define UTILITYDEFINE_H

#include <QString>

const int PROGRAM_LOG_ITEM_SIZE = 5;            //程序日志行字段数
const QString PROGRAM_STRING_SEPARATOR = " - "; //程序日志分割符

const int ACTION_LOG_ITEM_SIZE = 7;          //打点日志行字段数
const QString ACTION_STRING_SEPARATOR = ","; //打点日志分割符

///
/// \brief landi上课平台日志
///
struct LogRecordStruct
{
    QString time;
    QString type;
    QString thread_id;
    QString function;
    QString line_number;
    QString data;
};

///
/// \brief 打点日志结构体
///
struct ActionLogInfo
{
    QString time;
    QString type;
    QString version;
    QString acton_name;
    QString action_data;
};

enum LogFileTypeEnum
{
    LOG_PROGRAM = 0, //上课平台日志
    LOG_ACTION,      //打点日志
    LOG_EVENT        //事件日志
};

#endif // UTILITYDEFINE_H
