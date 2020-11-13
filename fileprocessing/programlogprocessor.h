#ifndef PROGRAMLOGPROCESSOR_H
#define PROGRAMLOGPROCESSOR_H

#include "baselogprocessor.h"
#include <QList>

typedef QMap<QString, QList<struct LogRecordStruct>> TypeRecordMap;

class ProgramLogProcessor : public BaseLogProcessor
{
public:
    explicit ProgramLogProcessor();
    ~ProgramLogProcessor() override;

    bool openLogFile(const QString &logName) override;
    QString getFileName() override;
    QString getFilePath() override;
    LogFileTypeEnum getLogType() override;

    QList<QString> &getLogRecordList() override;
    bool getItemRecord(QList<LogRecordStruct> &recordItemsList) override;

    QString getLastError() const override;

    TypeRecordMap &getRecordTypeMap();

private:
    void cleanData(QList<QString> &list);
    void creatLogTypeInfo(const struct LogRecordStruct &record);

private:
    QString logFileName;         //日志文件名
    QString logFilePath;         //日志文件路径
    LogFileTypeEnum logType;     //处理日志类型
    QString lastErrorMsg;        //最后一条错误信息
    QList<QString> logLineList;  //从日志文件中读取行数据列表
    TypeRecordMap recordTypeMap; //分日志类型保存信息
};

#endif // PROGRAMLOGPROCESSOR_H
