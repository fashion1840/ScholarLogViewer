#include "scholarlogviewer.h"
#include "ui_scholarlogviewer.h"

#include <QAction>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QListView>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include "CommonHelper.h"
#include "fileprocessing/actionlogprocessor.h"
#include "fileprocessing/androidlogprocessor.h"
#include "fileprocessing/ioslogprocessor.h"
#include "fileprocessing/programlogprocessor.h"
#include "fileprocessing/qcommondelegate.h"

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#else
#include "mac/macminimizewidget.h"
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
    , isFileOpened(false)
    , waitfrm(nullptr)
    , bShowRightContent(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#ifdef Q_OS_MACOS
    MacMinimizeWidget::AllowMinimizeForFramelessWindow(this);
#endif

    this->setAcceptDrops(true);

    //自定义 QComboBox item 样式
    ui->cbxLogType->setView(new QListView());
    ui->cbxInfoType->setView(new QListView());

    //默认隐藏内容显示窗口
    ui->contentWidget->setVisible(false);
    ui->btnCloseContent->setVisible(false);

    ui->labFuncName->adjustSize();

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

void ScholarLogViewer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (waitfrm && waitfrm->isVisible())
        waitfrm->resize(ui->infoFrame->width(), ui->infoFrame->height());
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
    //    this->showMinimized();
    setWindowState(Qt::WindowMinimized);
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
    pSearchButton->setCursor(Qt::PointingHandCursor);

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

    setTabelHeadWithType(currentLogType);

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

void ScholarLogViewer::setIosTabelHead()
{
    QStringList headList;
    headList << "Time"
             << "Type"
             << "Class"
             << "Function"
             << "Line"
             << "Data";
    model->setHorizontalHeaderLabels(headList);
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0, 230);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 120);
    ui->tableView->setColumnWidth(3, 200);
    ui->tableView->setColumnWidth(4, 60);
    ui->tableView->setColumnWidth(5, 200);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); //设定表头列宽不可变
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch); //设定第x列表头弹性拉伸
}

void ScholarLogViewer::setAndroidTabelHead()
{
    QStringList headList;
    headList << "Time"
             << "Type"
             << "Thread id"
             << "Model"
             << "Function"
             << "Data";
    model->setHorizontalHeaderLabels(headList);
    ui->tableView->setModel(model);

    ui->tableView->setColumnWidth(0, 230);
    ui->tableView->setColumnWidth(1, 80);
    ui->tableView->setColumnWidth(2, 100);
    ui->tableView->setColumnWidth(3, 120);
    ui->tableView->setColumnWidth(4, 200);
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

    if (!pCurrentList)
        return;

    displayLogInfo(*pCurrentList);
}

///
/// \brief 打开日志文件
///
void ScholarLogViewer::slot_openLogFile()
{
    qint64 startTimeStamp = QDateTime::currentMSecsSinceEpoch();
    qInfo() << "Open log file path:" << openFilePath;

    if (pLogProcessor)
    {
        delete pLogProcessor;
        pLogProcessor = nullptr;
    }

    setViewerState(false);
    showLoadingWidget(LOADING_MOVIE);

    switch (ui->cbxLogType->currentIndex())
    {
        case LOG_PROGRAM:
            pLogProcessor = new ProgramLogProcessor();
            break;
        case LOG_IOS:
            pLogProcessor = new IosLogProcessor();
            break;
        case LOG_ANDROID:
            pLogProcessor = new AndroidLogProcessor();
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
        closeLoadingWidget();
        setViewerState(true);
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(pLogProcessor->getLastError()));
        return;
    }

    logRecordList.clear();
    recordTypeMap.clear();

    result = pLogProcessor->getItemRecord(logRecordList);
    if (!result)
    {
        closeLoadingWidget();
        setViewerState(true);
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("%1").arg(pLogProcessor->getLastError()));
        return;
    }

    setTitleName(pLogProcessor->getFileName(), pLogProcessor->getFilePath());

    recordTypeMap = pLogProcessor->getRecordTypeMap();

    displayLogInfo(logRecordList);

    pCurrentList = &logRecordList;

    isFileOpened = true;

    QList<QString> typeList = recordTypeMap.keys();
    typeList.insert(0, "All");
    setTypeCombobox(typeList);

    closeLoadingWidget();

    setViewerState(true);

    qint64 endTimeStamp = QDateTime::currentMSecsSinceEpoch();

    qint64 intervalTime = endTimeStamp - startTimeStamp;
    qInfo() << QStringLiteral("总共加载 %1 条记录，耗时：%2 毫秒。").arg(logRecordList.size()).arg(intervalTime);
}

void ScholarLogViewer::displayLogInfo(const QList<QStringList> &recordlist)
{
    //清空列表内容
    model->removeRows(0, model->rowCount());

    if (recordlist.size() == 0)
        return;

    for (int i = 0; i < recordlist.size(); i++)
    {
        QStringList itemList = recordlist.at(i);
        QList<QStandardItem *> list;
        list << new QStandardItem(itemList.at(0));
        QStandardItem *item = new QStandardItem(itemList.at(1));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        list << item;
        item = new QStandardItem(itemList.at(2));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        list << item;
        //list << new QStandardItem(itemList.thread_id);
        list << new QStandardItem(itemList.at(3));
        if (currentLogType == LOG_PROGRAM || currentLogType == LOG_IOS || currentLogType == LOG_ANDROID)
        {
            item = new QStandardItem(itemList.at(itemList.size() - 2));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            list << item;
        }
        //list << new QStandardItem(itemList.line_number);
        list << new QStandardItem(itemList.at(itemList.size() - 1));

        model->insertRow(i, list);

        //TODO:防止界面疆死
        QCoreApplication::processEvents();
    }

    ui->tableView->setModel(model);
}

void ScholarLogViewer::on_seacherEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
    {
        pSearchButton->setVisible(false);
        slot_actionTrigger();
    }
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
        if (!recordTypeMap.isEmpty() && recordTypeMap.contains(arg1))
        {
            displayLogInfo(recordTypeMap[arg1]);
            pCurrentList = &recordTypeMap[arg1];
        }
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
    if (key.isEmpty() || pCurrentList == nullptr || pCurrentList->isEmpty())
        return;

    //判断是否是过滤搜索的关键字内容
    bool isExcluds = false;
    QString key2;
    if (key.startsWith("! "))
    {
        isExcluds = true;
        key2 = key.section("! ", 1);
    }

    QList<QStringList> resultList;
    for (auto item : *pCurrentList)
    {
        if (!isExcluds)
        {
            if (item.at(item.size() - 2).contains(key, Qt::CaseInsensitive) || item.at(item.size() - 1).contains(key, Qt::CaseInsensitive))
            {
                resultList.append(item);
            }
        }
        else
        {
            if (!item.at(item.size() - 2).contains(key2, Qt::CaseInsensitive) && !item.at(item.size() - 1).contains(key2, Qt::CaseInsensitive))
                resultList.append(item);
        }
    }

    displayLogInfo(resultList);
}

void ScholarLogViewer::slot_onSwitchLogType(LogFileTypeEnum type)
{
    resetWidget();

    setTabelHeadWithType(type);
}

void ScholarLogViewer::on_tableView_clicked(const QModelIndex &index)
{
    if (!ui->contentWidget->isVisible())
        return;

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
    if (!isFileOpened)
        pLabelAction->setEnabled(false);

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

void ScholarLogViewer::showLoadingWidget(WaitTipsEnum type)
{
    waitfrm = new WaitingWidget(ui->infoFrame);
    waitfrm->setAttribute(Qt::WA_ShowModal, true);
    waitfrm->setAttribute(Qt::WA_DeleteOnClose, true);
    waitfrm->loading(type);
    waitfrm->resize(ui->infoFrame->width(), ui->infoFrame->height());
    waitfrm->show();
}

void ScholarLogViewer::closeLoadingWidget()
{
    if (!waitfrm)
        return;

    waitfrm->close();
    delete waitfrm;
    waitfrm = nullptr;
}

void ScholarLogViewer::setViewerState(bool state)
{
    ui->tableView->setVisible(state);
    if (ui->contentWidget->isVisible())
        ui->contentWidget->setVisible(false);
}

void ScholarLogViewer::setTabelHeadWithType(LogFileTypeEnum type)
{
    switch (type)
    {
        case LOG_PROGRAM:
            setProgromTabelHead();
            break;
        case LOG_IOS:
            setIosTabelHead();
            break;
        case LOG_ANDROID:
            setAndroidTabelHead();
            break;
        case LOG_ACTION:
            setActonTabelHead();
            break;
        case LOG_EVENT:
            break;
    }
}

void ScholarLogViewer::on_btnExpandContent_clicked()
{
    bShowRightContent = !bShowRightContent;
    ui->contentWidget->setVisible(bShowRightContent);

    QModelIndexList list = ui->tableView->selectionModel()->selectedRows();
    if (list.count() <= 0)
        return;

    auto index = ui->tableView->selectionModel()->currentIndex();
    ui->labFuncName->setText(model->data(model->index(index.row(), 3)).toString());
    ui->itemContent->setText(model->data(model->index(index.row(), model->columnCount() - 1)).toString());

    /*
     QString styleStr;
    if (bShowRightContent)
    {
        styleStr = "QPushButton{border-image: url(:/images/expand.png);}QPushButton:hover{border-image: url(:/images/expand_hover.png);}";
    }
    else
    {
        styleStr = "QPushButton{border-image: url(:/images/gather.png);}QPushButton:hover{border-image: url(:/images/gather_hover.png);}";
    }

    ui->btnExpandContent->setStyleSheet(styleStr);
    */
}

void ScholarLogViewer::on_btnFormat_clicked()
{
    QString jsonStr = "";
    QString str = ui->itemContent->textCursor().selectedText().trimmed();
    if (str.isEmpty() || str.isNull())
        return;
    bool bRet = CommonHelper::stringToJsonFormat(str, jsonStr);
    if (!bRet)
        return;

    ui->itemContent->setText(jsonStr);
}
