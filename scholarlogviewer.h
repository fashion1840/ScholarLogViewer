#ifndef SCHOLARLOGVIEWER_H
#define SCHOLARLOGVIEWER_H

#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ScholarLogViewer;
}
QT_END_NAMESPACE

struct LogRecordStruct
{
    QString time;
    QString type;
    QString thread_id;
    QString function;
    QString line_number;
    QString data;
};

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

    void slot_actionTrigger();

    void slot_openLogFile();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

private:
    void initCustomSearchBox();
    void initTableView();
    void createItemRecord(const QList<QString> &recordList);
    void displayLogInfo();

private:
    Ui::ScholarLogViewer *ui;

    bool m_bPressed;
    QPoint m_point;
    QString openFilePath;
    QStandardItemModel *model;

    QList<struct LogRecordStruct> logRecordList;
};
#endif // SCHOLARLOGVIEWER_H
