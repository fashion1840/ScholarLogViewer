#ifndef UTILITYDEFINE_H
#define UTILITYDEFINE_H

#include <QString>

//PC平台日志定义
const int PROGRAM_LOG_ITEM_SIZE = 5;            //程序日志行字段数
const QString PROGRAM_STRING_SEPARATOR = " - "; //程序日志分割符

//Android平台日志定义
const int ANDROID_LOG_ITEM_SIZE = 5;          //程序日志行字段数
const QString ANDROID_STRING_SEPARATOR = " "; //程序日志分割符

//iOS平台日志定义
const int IOS_LOG_ITEM_SIZE = 8;          //程序日志行字段数
const QString IOS_STRING_SEPARATOR = " "; //程序日志分割符

//ACTION日志定义
const int ACTION_LOG_ITEM_SIZE = 7;          //打点日志行字段数
const QString ACTION_STRING_SEPARATOR = ","; //打点日志分割符

///
/// \brief landi上课平台日志
///
struct LogRecordStruct
{
    QString time;   //此条日志信息记录的时间
    QString type;   //日志的类型
    QString id;     //字段在不同的日志类型中表示不同的函义，程序日志为 thread id，打点日志为 version id，
    QString name;   //函数名
    QString number; //行号
    QString data;   //日志信息
};

enum LogFileTypeEnum
{
    LOG_PROGRAM = 0, //上课平台日志
    LOG_IOS,         //iOS日志
    LOG_ANDROID,     //安卓日志
    LOG_ACTION,      //打点日志
    LOG_EVENT        //事件日志
};

typedef QMap<QString, QList<QStringList>> TypeRecordMap;

enum WaitTipsEnum
{
    LOADING_MOVIE = 0,
    SEACHING_MOVIE
};

#endif // UTILITYDEFINE_H
