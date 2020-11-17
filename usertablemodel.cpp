#include "usertablemodel.h"

#include <QColor>

UserTableModel::UserTableModel(const QStringList &hreadList, QObject *parent) : QAbstractTableModel(parent), mRow(0), mColumn(0), headColumnList(hreadList) {}

UserTableModel::~UserTableModel() {}

void UserTableModel::updateData(QList<LogRecordStruct> &dataList)
{
    beginResetModel();
    infoList = dataList;
    mRow = dataList.size();
    mColumn = 5;
    endResetModel();
}

int UserTableModel::rowCount(const QModelIndex &) const
{
    return mRow;
}

int UserTableModel::columnCount(const QModelIndex &) const
{
    return mColumn;
}

QVariant UserTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section < headColumnList.size())
        {
            return headColumnList[section];
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant UserTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
        case Qt::TextColorRole:
            return QColor(Qt::black);
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole:
        {
            if (infoList.size() > index.row())
            {
            }

            return QVariant();
        }
        case Qt::CheckStateRole:
        {
            return QVariant(); // 返回勾选框，如果返回QVariant()就没有勾选框
        }
        default:
            return QVariant();
    }
}
