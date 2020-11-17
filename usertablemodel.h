#ifndef USERTABLEMODEL_H
#define USERTABLEMODEL_H

#include "UtilityDefine.h"
#include <QAbstractTableModel>

class UserTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit UserTableModel(const QStringList &hreadList, QObject *parent = nullptr);
    ~UserTableModel() override;

    void updateData(QList<LogRecordStruct> &dataList);

    int rowCount(const QModelIndex & = QModelIndex()) const override;

    int columnCount(const QModelIndex & = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    int mRow;
    int mColumn;
    QStringList headColumnList;
    QList<LogRecordStruct> infoList;
};

#endif // USERTABLEMODEL_H
