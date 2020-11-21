#ifndef SCHOLARLOGVIEWER_H
#define SCHOLARLOGVIEWER_H

#include <QAction>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QStandardItemModel>
#include <QWidget>

#include "UtilityDefine.h"
#include "fileprocessing/baselogprocessor.h"
#include "waitingwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ScholarLogViewer;
}
QT_END_NAMESPACE

class ScholarLogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ScholarLogViewer(QWidget *parent = nullptr);
    ~ScholarLogViewer() override;

private slots:
    void on_btnMinimize_clicked();

    void on_btnMaximize_clicked();

    void on_btnClose_clicked();

    void on_btnCompleted_clicked();

    void on_seacherEdit_textChanged(const QString &arg1);

    void on_cbxInfoType_currentIndexChanged(const QString &arg1);

    void on_btnCloseContent_clicked();

    void on_cbxLogType_currentIndexChanged(int index);

    void slot_actionTrigger();

    void slot_openLogFile();

    void slot_onSearchWithKey(const QString &key);

    void slot_onSwitchLogType(LogFileTypeEnum type);

    void on_tableView_clicked(const QModelIndex &index);

    void slot_onLabelTrigger();

    void slot_onShowLabelMenu(const QPoint &pos);

    void on_btnExpandContent_clicked();

    void on_btnFormat_clicked();

protected:
#ifdef Q_OS_MAC
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
#endif
#ifdef Q_OS_WIN
    // 进行鼠界面的拖动
    void mousePressEvent(QMouseEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;
#endif

    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initCustomSearchBox();
    void initTableHead();

    void displayLogInfo(const QList<struct LogRecordStruct> &recordlist);

    void setProgromTabelHead();
    void setActonTabelHead();
    void setIosTabelHead();
    void setAndroidTabelHead();

    inline void setTitleName(const QString &fileName, const QString &filePath);

    void updateItemContent(const QString &str1, const QString &str2);

    void setTypeCombobox(const QStringList &typelist);

    void resetWidget();

    void showLoadingWidget(WaitTipsEnum type);
    void closeLoadingWidget();

    inline void setViewerState(bool state);

    void setTabelHeadWithType(LogFileTypeEnum type);

private:
    Ui::ScholarLogViewer *ui;

    bool m_bPressed;
    QPoint m_point;
    QString openFilePath;
    QStandardItemModel *model;
    QPushButton *pSearchButton;

    QList<struct LogRecordStruct> logRecordList;
    QMap<QString, QList<struct LogRecordStruct>> recordTypeMap;
    QList<struct LogRecordStruct> *pCurrentList;
    LogFileTypeEnum currentLogType;

    BaseLogProcessor *pLogProcessor;

    QMenu *pLabelMenu;
    QAction *pLabelAction;

    bool isFileOpened;

    WaitingWidget *waitfrm;

    bool bShowRightContent;
};
#endif // SCHOLARLOGVIEWER_H
