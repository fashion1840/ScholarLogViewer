#ifndef BASELOGPROCESSOR_H
#define BASELOGPROCESSOR_H

#include "UtilityDefine.h"
#include <QString>

class BaseLogProcessor
{
public:
    virtual ~BaseLogProcessor() {}

    virtual bool openLogFile(const QString &logName) = 0;

    virtual QString getFileName() = 0;

    virtual QString getFilePath() = 0;

    virtual QString getFileDir() = 0;

    virtual LogFileTypeEnum getLogType() = 0;

    virtual QList<QString> &getLogRecordList() = 0;

    virtual bool getItemRecord(QList<QStringList> &recordItemsList) = 0;

    virtual TypeRecordMap &getRecordTypeMap() = 0;

    virtual QString getLastError() const = 0;
};

#endif // BASELOGPROCESSOR_H
