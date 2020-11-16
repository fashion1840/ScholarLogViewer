#include "scholarlogviewer.h"
#include "ui_scholarlogviewer.h"

#include <QAction>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QListView>
#include <QMessageBox>
#include <QMimeData>

#include "fileprocessing/actionlogprocessor.h"
#include "fileprocessing/programlogprocessor.h"
#include "fileprocessing/qcommondelegate.h"

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

ScholarLogViewer::ScholarLogViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScholarLogViewer)
    , model(nullptr)
    , pSearchButton(nullptr)
    , pCurrentList(nullptr)
    , currentLogType(LOG_PROGRAM)
    , pLogProcessor(nullptr)
    , pLabelMenu(nullptr)
    , pLabelAction(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    this->setAcceptDrops(true);

    ui->cbxLogType->setView(new QListView());
    ui->cbxInfoType->setView(new QListView());
    ui->contentWidget->setVisible(false);

    //去掉选择中 item 的虚线框
    ui->tableView->setItemDelegate(new QCommonDelegate);
    //初始化搜索框
    initCustomSearchBox();
    //初始化 view 列标头信息
    initTableHead();

    //给控件设置上下文菜单策略
    ui->labFileName->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->labFileName, &QLabel::customContextMenuRequested, this, &ScholarLogViewer::slot_onShowLabelMenu);

    ui->seacherEdit->installEventFilter(this);
    ui->tableView->installEventFilter(this);
}

ScholarLogViewer::~ScholarLogViewer()
{
    if (pLogProcessor)
    {
        delete pLogProcessor;
        pLogProcessor = nullptr;
    }
    delete ui;
}

///
/// \brief 自定义标题栏移动
/// \param event
///
#ifdef Q_OS_WIN
void ScholarLogViewer::mousePressEvent(QMouseEvent *event)
{

    if (ReleaseCapture())
    {
        QWidget *pWindow = this->window();
        if (pWindow->isTopLevel())
        {
            SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
}
#endif

#ifdef Q_OS_MAC
///
/// brief 鼠标相对于窗体的位置 event->globalPos() - this->pos()
///
void ScholarLogViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_point = event->pos();
        //if (m_point.x() <= 0 || m_point.y() <= 0)
        //    return;
        //if (m_point.x() >= ui->titleFrame->height() || m_point.y() >= ui->titleFrame->width())
        //    return;

        m_bPressed = true;
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

#endif

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

bool ScholarLogViewer::eventFilter(QObject *obj, QEvent *event)
{
    //Qt 中的回车键对应的是两个键值： Qt::Key_Enter  以及 Qt::Key_Return
    //所以，要在 keyPressEvent() 获 keyReleaseEvent() 中捕获回车键，应使用：
    if (obj == ui->seacherEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            {
                QMetaObject::invokeMethod(this, "slot_onSearchWithKey", Qt::QueuedConnection, Q_ARG(QString, ui->seacherEdit->text()));
            }
        }
    }
    else if (obj == ui->tableView)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Down)
            {
                int row = ui->tableView->currentIndex().row() + 1;
                if (row < model->rowCount())
                {
                    QString str1 = model->data(model->index(row, 3)).toString();
                    QString str2 = model->data(model->index(row, model->columnCount() - 1)).toString();
                    updateItemContent(str1, str2);
                }
            }
            else if (keyEvent->key() == Qt::Key_Up)
            {
                int row = ui->tableView->currentIndex().row() - 1;
                if (row >= 0)
                {
                    QString str1 = model->data(model->index(row, 3)).toString();
                    QString str2 = model->data(model->index(row, model->columnCount() - 1)).toString();
                    updateItemContent(str1, str2);
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

///
/// \brief 最小化
///
void ScholarLogViewer::on_btnMinimize_clicked()
{
    this->showMinimized();
}

///
/// \brief 最大化
///
void ScholarLogViewer::on_btnMaximize_clicked()
{
    if (this->isMaximized())
    {
        this->showNormal();
        ui->btnMaximize->setStyleSheet("QPushButton{background-image:url(:/images/max_gray.png);}"
                                       "QPushButton:hover{background-image:url(:/images/max_blue.png);}");
    }
    else
    {
        this->showMaximized();
        ui->btnMaximize->setStyleSheet("QPushButton{background-image:url(:/images/normal_gray.png);}"
                                       "QPushButton:hover{background-image:url(:/images/normal_blue.png);}");
    }
}

///
/// \brief 关闭窗口
///
void ScholarLogViewer::on_btnClose_clicked()
{
    this->close();
}

///
/// \brief 搜索
///
void ScholarLogViewer::on_btnCompleted_clicked()
{
    QString search_key = ui->seacherEdit->text();
    qInfo() << "Search key value:" << search_key;
    slot_onSearchWithKey(search_key);
}

///
/// \brief 初始化搜索框
///
void ScholarLogViewer::initCustomSearchBox()
{
    pSearchButton = new QPushButton(this);
    pSearchButton->setFixedSize(16, 16);
    pSearchButton->setStyleSheet("QPushButton{border-image:url(:/images/delete.png);}"
                                 "QPushButton:hover{border-image:url(:/images/delete_hover.png);}");

    QMargins margins = ui->seacherEdit->textMargins();
    ui->seacherEdit->setTextMargins(margins.left(), margins.top(), pSearchButton->width(), margins.bottom());

    QHBoxLayout *pSearchLayout = new QHBoxLayout();
    pSearchLayout->addStretch();
    pSearchLayout->addWidget(pSearchButton);
    pSearchLayout->setSpacing(0);
    pSearchLayout->setContentsMargins(0, 0, 4, 0);
    ui->seacherEdit->setLayout(pSearchLayout);
    connect(pSearchButton, &QPushButton::clicked, this, &ScholarLogViewer::slot_actionTrigger);
    pSearchButton->setVisible(false);

    QAction *pSeachAction = new QAction(this);
    pSeachAction->setIcon(QIcon(":/images/search_leading.png"));
    ui->seacherEdit->addAction(pSeachAction, QLineEdit::LeadingPosition);

    //connect(pSeachAction, &QAction::trigger, this, &ScholarLogViewer::slot_actionTrigger);
}

///
/// \brief 初始化数据表头内容
///
void ScholarLogViewer::initTableHead()
{
    //背景网格线设置
    ui->tableView->setShowGrid(false);
    //排序功能
    ui->tableView->setSortingEnabled(false);
    //设置表头
    model = new QStandardItemModel();

    switch (currentLogType)
    {
        case LOG_PROGRAM:
            setProgromTabelHead();
            break;
        case LOG_ACTION:
            setActonTabelHead();
            break;
        case LOG_EVENT:
            break;
    }

    //设置表格属性
    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter); //表头信息显示居中
    ui->tableView->verticalHeader()->hide();                                 //隐藏默认显示的行头
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);      //设置选中时整行选中
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);       //设置表格属性只读，不能编辑

    //ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color:white; border:none; border-right:1px solid gray;}");
    ui->tableView->horizontalHeader()->setSectionsClickable(false); //水平方向的头不可点击
    //ui->tableView->verticalHeader()->setSectionsClickable(false);   //垂直方向的头不可点击
}

void ScholarLogViewer::setProgromTabelHead()
{
    QStringList headList;
    headList << "Time"
             << "Type"
             << "Thread id"
             << "Function"
             << "Line"
             << "Data";
    model->setHorizontalHeaderLabels(headList);
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0, 215);
    ui->tableView->setColumnWidth(1, 60);
    ui->tableView->setColumnWidth(2, 120);
    ui->tableView->setColumnWidth(3, 200);
    ui->tableView->setColumnWidth(4, 60);
    ui->tableView->setColumnWidth(5, 200);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch); //设定第x列表头弹性拉伸
}

void ScholarLogViewer::setActonTabelHead()
{

    QStringList headList;
    headList << "Time"
             << "Type"
             << "Version"
             << "Function"
             << "Data";
    model->setHorizontalHeaderLabels(headList);
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0, 215);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 100);
    ui->tableView->setColumnWidth(3, 200);
    ui->tableView->setColumnWidth(4, 200);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); //设定第x列表头弹性拉伸
}

void ScholarLogViewer::setTitleName(const QString &fileName, const QString &filePath)
{
    ui->labFileName->setText(fileName);
    ui->labFileName->setToolTip(filePath);
}

///
/// \brief 清空搜索框内容
///
void ScholarLogViewer::slot_actionTrigger()
{
    ui->seacherEdit->clear();

    displayLogInfo(*pCurrentList);
}

///
/// \brief 打开日志文件
///
void ScholarLogViewer::slot_openLogFile()
{
    qInfo() << "Open log file path:" << openFilePath;

    if (pLogProcessor)
    {
        delete pLogProcessor;
        pLogProcessor = nullptr;
    }

    switch (ui->cbxLogType->currentIndex())
    {
        case LOG_PROGRAM:
            pLogProcessor = new ProgramLogProcessor();
            break;
        case LOG_ACTION:
            pLogProcessor = new ActionLogProcessor();
            break;
        case LOG_EVENT:
            break;
    }

    bool result = pLogProcessor->openLogFile(openFilePath);
    if (!result)
    {
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(pLogProcessor->getLastError()));
        return;
    }

    logRecordList.clear();
    recordTypeMap.clear();

    result = pLogProcessor->getItemRecord(logRecordList);
    if (!result)
    {
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(pLogProcessor->getLastError()));
        return;
    }

    setTitleName(pLogProcessor->getFileName(), pLogProcessor->getFilePath());

    recordTypeMap = pLogProcessor->getRecordTypeMap();

    displayLogInfo(logRecordList);

    QList<QString> typeList = recordTypeMap.keys();
    typeList.insert(0, "All");
    setTypeCombobox(typeList);

    pCurrentList = &logRecordList;
}

void ScholarLogViewer::displayLogInfo(const QList<LogRecordStruct> &recordlist)
{
    //清空列表内容
    model->removeRows(0, model->rowCount());

    if (recordlist.size() == 0)
        return;

    for (int i = 0; i < recordlist.count(); i++)
    {
        QList<QStandardItem *> list;
        list << new QStandardItem(recordlist.at(i).time);
        QStandardItem *item = new QStandardItem(recordlist.at(i).type);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        list << item;
        item = new QStandardItem(recordlist.at(i).id);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        list << item;
        //list << new QStandardItem(recordlist.at(i).thread_id);
        list << new QStandardItem(recordlist.at(i).name);
        if (currentLogType == LOG_PROGRAM)
        {
            item = new QStandardItem(recordlist.at(i).number);
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            list << item;
        }
        //list << new QStandardItem(recordlist.at(i).line_number);
        list << new QStandardItem(recordlist.at(i).data);

        model->insertRow(i, list);
    }

    ui->tableView->setModel(model);
}

void ScholarLogViewer::on_seacherEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        pSearchButton->setVisible(false);
    else
        pSearchButton->setVisible(true);
}

void ScholarLogViewer::on_cbxInfoType_currentIndexChanged(const QString &arg1)
{
    if (arg1 == "All")
    {
        displayLogInfo(logRecordList);
        pCurrentList = &logRecordList;
    }
    else
    {
        displayLogInfo(recordTypeMap[arg1]);
        pCurrentList = &recordTypeMap[arg1];
    }
}

void ScholarLogViewer::on_btnCloseContent_clicked()
{
    ui->contentWidget->setVisible(false);
}

void ScholarLogViewer::on_cbxLogType_currentIndexChanged(int index)
{
    currentLogType = static_cast<LogFileTypeEnum>(index);

    // QMetaObject::invokeMethod(this, "slot_onSwitchLogType", Qt::QueuedConnection, Q_ARG(int, index));
    slot_onSwitchLogType(currentLogType);
}

void ScholarLogViewer::slot_onSearchWithKey(const QString &key)
{
    if (pCurrentList->isEmpty() || key.isEmpty())
        return;

    //判断是否是过滤搜索的关键字内容
    bool isExcluds = false;
    QString key2;
    if (key.startsWith("! "))
    {
        isExcluds = true;
        key2 = key.section("! ", 1);
    }

    QList<struct LogRecordStruct> resultList;
    for (auto item : *pCurrentList)
    {
        if (!isExcluds)
        {
            if (item.name.contains(key) || item.data.contains(key))
            {
                resultList.append(item);
            }
        }
        else
        {
            if (!item.name.contains(key2) && !item.data.contains(key2))
                resultList.append(item);
        }
    }

    /*
    if (currentLogType == LOG_ACTION)
    {
        for (auto item : *pCurrentList)
        {
            if (item.name.contains(key))
            {
                resultList.append(item);
            }
        }
    }
    else
    {
        for (auto item : *pCurrentList)
        {
            if (item.data.contains(key))
            {
                resultList.append(item);
            }
        }
    }
    */

    displayLogInfo(resultList);
}

void ScholarLogViewer::slot_onSwitchLogType(LogFileTypeEnum type)
{
    resetWidget();

    switch (type)
    {
        case LOG_PROGRAM:
            setProgromTabelHead();
            //ui->cbxInfoType->setVisible(true);

            break;
        case LOG_ACTION:
            setActonTabelHead();
            //ui->cbxInfoType->setVisible(false);
            break;
        case LOG_EVENT:
            break;
    }
}

void ScholarLogViewer::on_tableView_clicked(const QModelIndex &index)
{
    if (!ui->contentWidget->isVisible())
        ui->contentWidget->setVisible(true);

    ui->labFuncName->setText(model->data(model->index(index.row(), 3)).toString());

    ui->itemContent->setText(model->data(model->index(index.row(), model->columnCount() - 1)).toString());
}

void ScholarLogViewer::slot_onLabelTrigger()
{
    QDesktopServices::openUrl(QUrl("file:" + pLogProcessor->getFileDir(), QUrl::TolerantMode));
}

void ScholarLogViewer::slot_onShowLabelMenu(const QPoint &pos)
{
    Q_UNUSED(pos)

    pLabelMenu = new QMenu(ui->labFileName);
    pLabelAction = new QAction(QStringLiteral("打开文件所在的目录"), this);
    pLabelMenu->addAction(pLabelAction);

    connect(pLabelAction, &QAction::triggered, this, &ScholarLogViewer::slot_onLabelTrigger);

    pLabelMenu->exec(QCursor::pos());
}

void ScholarLogViewer::updateItemContent(const QString &str1, const QString &str2)
{
    ui->labFuncName->setText(str1);

    ui->itemContent->setText(str2);
}

void ScholarLogViewer::setTypeCombobox(const QStringList &typelist)
{
    ui->cbxInfoType->clear();
    ui->cbxInfoType->addItems(typelist);
}

void ScholarLogViewer::resetWidget()
{
    model->clear();
    logRecordList.clear();
    recordTypeMap.clear();
    pCurrentList = nullptr;
    ui->cbxInfoType->clear();
    ui->labFileName->setText("ScholarLogViewer");
    if (ui->contentWidget->isVisible())
        ui->contentWidget->setVisible(false);
}
