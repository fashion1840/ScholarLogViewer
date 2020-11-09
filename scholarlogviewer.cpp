#include "scholarlogviewer.h"
#include "ui_scholarlogviewer.h"

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>

ScholarLogViewer::ScholarLogViewer(QWidget *parent) : QWidget(parent), ui(new Ui::ScholarLogViewer), model(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    this->setAcceptDrops(true);

    logRecordList.clear();

    initCustomSearchBox();

    initTableView();
}

ScholarLogViewer::~ScholarLogViewer()
{
    delete ui;
}

void ScholarLogViewer::on_btnMinimize_clicked()
{
    this->showMinimized();
}

void ScholarLogViewer::on_btnMaximize_clicked()
{
    if (this->isMaximized())
        this->showNormal();
    else
        this->showMaximized();
}

void ScholarLogViewer::on_btnClose_clicked()
{
    this->close();
}

void ScholarLogViewer::slot_actionTrigger() {}

///
/// \brief ScholarLogViewer::slot_openLogFile
///
void ScholarLogViewer::slot_openLogFile()
{
    qInfo() << "Open log file path:" << openFilePath;

    QList<QString> fileList;

    QFile fp(openFilePath);
    if (!fp.exists())
    {
        QMessageBox::critical(this, QStringLiteral(""), QStringLiteral(""));
        return;
    }

    if (!fp.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!fp.atEnd())
    {
        QByteArray txt = fp.readLine();
        fileList << txt;
    }

    fp.close();

    createItemRecord(fileList);
}

///
/// brief 鼠标相对于窗体的位置 event->globalPos() - this->pos()
///
void ScholarLogViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bPressed = true;
        m_point = event->pos();
    }
}

///
/// \brief 若鼠标左键被按下，移动窗体位置
///
void ScholarLogViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bPressed)
        move(event->pos() - m_point + pos());
}

///
/// brief 设置鼠标未被按下
///;
void ScholarLogViewer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    m_bPressed = false;
}

void ScholarLogViewer::dropEvent(QDropEvent *event)
{
    //获取MIMEData
    const QMimeData *qm = event->mimeData();
    openFilePath = qm->urls()[0].toString().remove("file:///");

    QMetaObject::invokeMethod(this, "slot_openLogFile", Qt::QueuedConnection);
}

void ScholarLogViewer::dragEnterEvent(QDragEnterEvent *event)
{
    //如果类型是txt或者log才能接受拖动。
    //这里的compare字符串比较函数，相等的时候返回0，所以要取反
    if (!event->mimeData()->urls()[0].fileName().right(3).compare("txt") || !event->mimeData()->urls()[0].fileName().right(3).compare("log"))
        event->acceptProposedAction();
    else
        event->ignore(); //否则不接受鼠标事件
}

void ScholarLogViewer::initCustomSearchBox()
{
    QPushButton *pSearchButton = new QPushButton(this);
    pSearchButton->setFixedSize(24, 24);
    pSearchButton->setStyleSheet("QPushButton{border-image:url(:/images/search.png);}"
                                 "QPushButton:hover{border-image:url(:/images/search_hover.png);}");

    QMargins margins = ui->seacherEdit->textMargins();
    ui->seacherEdit->setTextMargins(margins.left(), margins.top(), pSearchButton->width(), margins.bottom());

    QHBoxLayout *pSearchLayout = new QHBoxLayout();
    pSearchLayout->addStretch();
    pSearchLayout->addWidget(pSearchButton);
    pSearchLayout->setSpacing(0);
    pSearchLayout->setContentsMargins(0, 0, 4, 0);
    ui->seacherEdit->setLayout(pSearchLayout);
    connect(pSearchButton, &QPushButton::clicked, this, &ScholarLogViewer::slot_actionTrigger);

#if 0
    QAction *pSeachAction = new QAction(this);
    pSeachAction->setIcon(QIcon(":/images/search.png"));

    ui->seacherEdit->addAction(pSeachAction, QLineEdit::TrailingPosition);

    connect(pSeachAction, &QAction::trigger, this, &ScholarLogViewer::slot_actionTrigger);
#endif
}

void ScholarLogViewer::initTableView()
{
    //背景网格线设置
    ui->tableView->setShowGrid(false);

    //排序功能
    ui->tableView->setSortingEnabled(false);

    //设置表头
    model = new QStandardItemModel();
    QStringList headList;
    headList << "Time"
             << "Type"
             << "Thread id"
             << "Function"
             << "Line"
             << "Data";
    model->setHorizontalHeaderLabels(headList);
    ui->tableView->setModel(model);

    //设置表格属性
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter); //表头信息显示居中
    ui->tableView->verticalHeader()->hide();                                 //隐藏默认显示的行头
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);      //设置选中时整行选中
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);       //设置表格属性只读，不能编辑

    ui->tableView->setColumnWidth(0, 200); //设定表格第0列宽度
    ui->tableView->setColumnWidth(1, 50);
    ui->tableView->setColumnWidth(2, 100); //设定表格第0列宽度
    ui->tableView->setColumnWidth(3, 200);
    ui->tableView->setColumnWidth(4, 50); //设定表格第0列宽度
    ui->tableView->setColumnWidth(5, 200);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Custom);
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch); //设定第2列表头弹性拉伸
}

void ScholarLogViewer::createItemRecord(const QList<QString> &recordList)
{
    if (recordList.isEmpty())
        return;

    for (int i = 0; i < recordList.size(); i++)
    {
        QStringList list = recordList.at(i).split("-");

        struct LogRecordStruct item;
        item.time = list[0].mid(list[0].indexOf(" ") + 1);
        item.type = list[1];
        item.thread_id = list[2];
        int idx = list[3].lastIndexOf(":");
        item.function = list[3].left(idx);
        item.line_number = list[3].right(list[3].size() - idx - 1);
        item.data = list[4];

        logRecordList.append(item);
    }

    displayLogInfo();
}

void ScholarLogViewer::displayLogInfo()
{
    for (int i = 0; i < logRecordList.count(); i++)
    {
        model->setItem(i, 0, new QStandardItem(logRecordList.at(i).time));
        model->setItem(i, 1, new QStandardItem(logRecordList.at(i).type));
        model->setItem(i, 2, new QStandardItem(logRecordList.at(i).thread_id));
        model->setItem(i, 3, new QStandardItem(logRecordList.at(i).function));
        model->setItem(i, 4, new QStandardItem(logRecordList.at(i).line_number));
        model->setItem(i, 5, new QStandardItem(logRecordList.at(i).data));
    }

    ui->tableView->setModel(model);
}
