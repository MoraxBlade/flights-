#include "orderview.h"
#include "ui_orderview.h"
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QDebug>

OrderView::OrderView(QWidget *parent, User *currentUser)
    : QWidget(parent)
    , ui(new Ui::OrderView)
    , currentUser(currentUser)
    , ordermodel(nullptr)
    , orderproxymodel(nullptr)
    , refreshTimer(nullptr)
{
    ui->setupUi(this);

    // 初始化UI
    initUI();

    // 初始化数据模型
    initModel();

    // 初始加载数据
    refreshDatabase();
}

OrderView::~OrderView()
{
    delete ui;
    if (refreshTimer) {
        refreshTimer->stop();
        delete refreshTimer;
    }
}

void OrderView::initUI()
{
    // 设置表格属性
    ui->tableOrder->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableOrder->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableOrder->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableOrder->setSortingEnabled(true);

    // 设置列宽自适应
    ui->tableOrder->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 隐藏编辑、删除按钮，因为我们只需要查看功能
    ui->btnAddOrder->setVisible(false);
    ui->btnDeleteOrder->setVisible(false);
    ui->btnModifyOrder->setVisible(false);
    ui->btnSearch->setVisible(false);
    ui->btnReset->setVisible(false);

    // 隐藏搜索条件区域
    ui->groupSearch->setVisible(false);

    // 设置窗口标题
    this->setWindowTitle("我的订单");
}

void OrderView::initModel()
{
    // 创建订单模型
    ordermodel = new OrderModel(this);
    if (!ordermodel->init()) {
        QMessageBox::critical(this, "错误", "订单数据初始化失败！");
        return;
    }

    // 创建代理模型用于过滤
    orderproxymodel = new QSortFilterProxyModel(this);
    orderproxymodel->setSourceModel(ordermodel);
    orderproxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // 设置表格视图的模型
    ui->tableOrder->setModel(orderproxymodel);

    // 设置列标题
    ui->tableOrder->setColumnHidden(0, false);  // 显示订单号
    ui->tableOrder->setColumnHidden(1, true);   // 隐藏ClientID（不需要显示）

    // 设置列宽
    ui->tableOrder->setColumnWidth(0, 80);      // 订单号
    ui->tableOrder->setColumnWidth(2, 100);     // 航班ID
    ui->tableOrder->setColumnWidth(3, 80);      // 座位ID
    ui->tableOrder->setColumnWidth(4, 80);      // 行李ID
    ui->tableOrder->setColumnWidth(5, 100);     // 舱位类型

    // 创建刷新定时器
    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(timerefreshMs);
    connect(refreshTimer, &QTimer::timeout, this, &OrderView::refreshDatabase);
    refreshTimer->start();

    // 连接表格选择变化信号
    connect(ui->tableOrder->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &OrderView::onOrderSelectionChanged);
}

void OrderView::refreshDatabase()
{
    if (ordermodel && ordermodel->select()) {
        // 只显示当前用户的订单
        filterByCurrentUser();

        // 更新状态显示
        int rowCount = orderproxymodel->rowCount();
        ui->lblStatus->setText(QString("您共有 %1 个订单").arg(rowCount));
    } else if (ordermodel) {
        qDebug() << "刷新订单数据失败：" << ordermodel->lastError().text();
    }
}

void OrderView::filterByCurrentUser()
{
    if (!currentUser || !orderproxymodel) {
        return;
    }

    int clientId = currentUser->getUserID();  // 假设User类有getUserID()方法

    if (clientId > 0) {
        // 根据ClientID过滤，显示该用户的订单
        // 假设ClientID是第1列（索引1）
        orderproxymodel->setFilterKeyColumn(1);
        orderproxymodel->setFilterFixedString(QString::number(clientId));

        qDebug() << "过滤用户ID:" << clientId << "，显示订单数量:" << orderproxymodel->rowCount();
    }
}

void OrderView::onOrderSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if (!current.isValid()) {
        clearDetails();
        return;
    }

    loadOrderDetails(current);
}

void OrderView::loadOrderDetails(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid() || !ordermodel || !orderproxymodel) {
        return;
    }

    // 获取源模型索引
    QModelIndex sourceIndex = orderproxymodel->mapToSource(proxyIndex);

    if (!sourceIndex.isValid()) {
        return;
    }

    // 获取订单信息
    int orderId = ordermodel->data(ordermodel->index(sourceIndex.row(), 0)).toInt();
    int clientId = ordermodel->data(ordermodel->index(sourceIndex.row(), 1)).toInt();
    int flightId = ordermodel->data(ordermodel->index(sourceIndex.row(), 2)).toInt();
    int seatId = ordermodel->data(ordermodel->index(sourceIndex.row(), 3)).toInt();
    int luggageId = ordermodel->data(ordermodel->index(sourceIndex.row(), 4)).toInt();
    int cabinType = ordermodel->data(ordermodel->index(sourceIndex.row(), 5)).toInt();

    // 转换为可读的舱位类型
    QString cabinTypeStr;
    switch (cabinType) {
    case 1: cabinTypeStr = "经济舱"; break;
    case 2: cabinTypeStr = "商务舱"; break;
    case 3: cabinTypeStr = "头等舱"; break;
    default: cabinTypeStr = "未知舱位";
    }

    // 在详细信息区域显示
    ui->txtOrderID->setText(QString::number(orderId));
    ui->txtClientID->setText(QString::number(clientId));
    ui->txtFlightID->setText(QString::number(flightId));
    ui->txtSeatID->setText(QString::number(seatId));
    ui->txtLuggageID->setText(QString::number(luggageId));
    ui->txtCabinType->setText(cabinTypeStr);

    // 这里可以添加更多信息的显示，比如：
    // 1. 根据flightId查询航班详细信息
    // 2. 根据seatId查询座位详细信息
    // 3. 根据luggageId查询行李详细信息
}

void OrderView::clearDetails()
{
    ui->txtOrderID->clear();
    ui->txtClientID->clear();
    ui->txtFlightID->clear();
    ui->txtSeatID->clear();
    ui->txtLuggageID->clear();
    ui->txtCabinType->clear();
}

void OrderView::on_btnRefresh_clicked()
{
    refreshDatabase();
}
