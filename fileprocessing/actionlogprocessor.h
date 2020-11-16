#ifndef ACTIONLOGPROCESSOR_H
#define ACTIONLOGPROCESSOR_H

#include "baselogprocessor.h"
#include <QList>

class ActionLogProcessor : public BaseLogProcessor
{
public:
    explicit ActionLogProcessor();
    ~ActionLogProcessor() override;

    bool openLogFile(const QString &logName) override;
    QString getFileName() override;
    QString getFilePath() override;
    QString getFileDir() override;
    LogFileTypeEnum getLogType() override;
    QList<QString> &getLogRecordList() override;
    QString getLastError() const override;
    bool getItemRecord(QList<LogRecordStruct> &recordItemsList) override;
    TypeRecordMap &getRecordTypeMap() override;

private:
    void cleanData(QList<QString> &list);
    void creatLogTypeInfo(const QList<LogRecordStruct> &recordList);

private:
    QString logFileName; //日志文件名
    QString logFilePath; //日志文件路径
    QString logFileDir;
    LogFileTypeEnum logType;     //处理日志类型
    QString lastErrorMsg;        //最后一条错误信息
    QList<QString> logLineList;  //从日志文件中读取行数据列表
    TypeRecordMap recordTypeMap; //分日志类型保存信息
};

#endif // ACTIONLOGPROCESSOR_H
