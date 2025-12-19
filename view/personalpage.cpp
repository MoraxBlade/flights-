// personalpage.cpp
#include "personalpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QHeaderView>
#include <QMessageBox>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "logindialog.h"
#include "companyregisterdialog.h"
#include "flightwidget.h"
#include "adminform.h"
#include "../entity/user.h"
#include "ui_personalpage.h"
#include "passwordchange.h"
#include "../model/ordermodel.h"
#include <QSortFilterProxyModel>
#include <QSqlQuery>

extern User *curUser;

personalpage::personalpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::personalpage)
    , Usr(new QLabel(this))
    , ordermodel(nullptr)
    , orderProxyModel(nullptr)
    , selectedOrderId(-1)
    , flightSeatCount(0)
    , focusedID(-1)
    , currentLuggageWeight(0)
    , currentLuggagePrice(0.0)
    , currentBasePrice(0.0)
    , luggageType(0)
{
    ui->setupUi(this);
    ui->labelUserName_2->setCurrentIndex(0);
    updateUserStatus();

    // 初始化订单表格
    initOrderView();

    // 设置spinbox
    ui->spinBox->setStyleSheet("QSpinBox { color: black; background: white; }");
    ui->spinBox->setEnabled(true);
    ui->spinBox->setReadOnly(false);
    ui->spinBox->setRange(0, 100);
    ui->spinBox->setValue(0);
    ui->spinBox->setMinimumWidth(80);

    // 设置行李类型初始状态
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(false);
    luggageType = 0;
    ui->spinBox->setEnabled(false);  // 默认不托运时禁用

    // 连接spinbox信号
    connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value){
                on_spinBox_textChanged(QString::number(value));
            });

    // 连接刷新按钮
    connect(ui->btnRefreshOrders, &QPushButton::clicked, this, &personalpage::refreshOrders);

    // 初始化图形视图
    if (!ui->graphicsView->scene())
        ui->graphicsView->setScene(new QGraphicsScene(this));

    // 默认显示提示文字
    QGraphicsTextItem *textItem = new QGraphicsTextItem("请先选择订单");
    textItem->setFont(QFont("Arial", 20));
    textItem->setDefaultTextColor(Qt::gray);
    textItem->setPos(50, 50);
    ui->graphicsView->scene()->addItem(textItem);
}

void personalpage::showDefaultMessage(const QString& message)
{
    ui->graphicsView->scene()->clear();
    QGraphicsTextItem *textItem = new QGraphicsTextItem(message);
    textItem->setFont(QFont("Arial", 16));
    textItem->setDefaultTextColor(Qt::gray);

    // 居中显示
    QRectF textRect = textItem->boundingRect();
    textItem->setPos(-textRect.width()/2, -textRect.height()/2);

    ui->graphicsView->scene()->addItem(textItem);
    ui->graphicsView->setSceneRect(-100, -50, 200, 100);
}

void personalpage::focusedIdChanged(int id)
{
    qDebug() << "focusedIdChanged 被调用，id:" << id;

    if (id < 0 || id >= seat.size()) {
        qDebug() << "座位ID超出范围";
        return;
    }

    if (!seat[id]) {
        qDebug() << "座位对象为空";
        return;
    }

    // 检查座位是否被占用
    if (seat[id]->getState() == Occupied) {
        qDebug() << "座位" << id << "已被占用，无法选择";
        return;
    }

    // 保存上一个被选中的座位
    int prevFocusedID = focusedID;

    // 如果点击了同一个座位
    if (focusedID == id) {
        qDebug() << "点击了同一个座位，取消选择";
        seat[id]->setSelected(false);
        seat[id]->setState(Available);
        focusedID = -1;

        if (ui->seat) {
            ui->seat->setText("未选择");
        }
        return;
    }

    // 更新上一个座位的状态
    if (prevFocusedID != -1 && prevFocusedID < seat.size() && seat[prevFocusedID]) {
        seat[prevFocusedID]->setSelected(false);
        if (seat[prevFocusedID]->getState() != Occupied) {
            seat[prevFocusedID]->setState(Available);
        }
    }

    // 更新当前选中的座位
    focusedID = id;
    seat[focusedID]->setSelected(true);
    seat[focusedID]->setState(Selected);

    // 获取座位号显示
    int seatNum = seat[id]->getSeatNumber();

    qDebug() << "选择了座位，id:" << id << "座位号:" << seatNum;

    if (ui->seat) {
        ui->seat->setText(QString("座位: %1").arg(seatNum));
    }
}

personalpage::~personalpage()
{
    delete ui;
    delete Usr;
    for(const auto& ptr:seat) delete ptr;
}

void personalpage::initOrderView()
{
    // 先获取数据库
    QSqlDatabase db = QSqlDatabase::database();

    // 初始化订单模型
    ordermodel = new OrderModel(this, db);
    if (!ordermodel->init()) {
        qDebug() << "订单模型初始化失败";
        return;
    }

    // 初始化航班模型
    FlightModel *flightModel = new FlightModel(this);
    flightModel->init();

    // 注入到 ordermodel
    ordermodel->flightModel = flightModel;

    // 将模型直接绑定到表格
    ui->orderView->setModel(ordermodel);
    ui->orderView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->orderView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->orderView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->orderView->setSortingEnabled(false);

    QHeaderView* header = ui->orderView->horizontalHeader();

    // 隐藏 ClientID(1) 和 行李ID(4)
    ui->orderView->setColumnHidden(1, true);

    // 设置列宽
    header->resizeSection(0, 100);
    header->resizeSection(2, 120);
    header->resizeSection(4, 100);
    header->resizeSection(15, 100);
    header->resizeSection(16, 100);
    header->resizeSection(6, 120);
    header->resizeSection(7, 150);
    header->resizeSection(8, 150);
    header->resizeSection(9, 80);
    header->resizeSection(10, 100);
    header->resizeSection(11, 100);
    header->resizeSection(3, 80);
    header->resizeSection(5, 80);
    header->resizeSection(12, 100);
    header->resizeSection(13, 100);
    header->resizeSection(14,100);

    header->setStretchLastSection(true);

    // 加载用户订单
    loadUserOrders();
    ui->orderView->setStyleSheet("");
    loadUserOrdersToComboBox();

}


void personalpage::loadUserOrders()
{
    if (!curUser) {
        qDebug() << "用户未登录";
        return;
    }

    int userId = curUser->getUserID();
    if (userId <= 0) {
        qDebug() << "用户ID无效";
        return;
    }

    // 过滤当前用户订单
    ordermodel->setFilter(QString("ClientID = %1").arg(userId));
    ordermodel->select();
}


void personalpage::refreshOrders()
{
    qDebug() << "手动刷新订单";
    loadUserOrders();
}

void personalpage::updateUserStatus()
{
    if (!curUser) {
        qDebug() << "updateUserStatus: curUser为空";
        return;
    }

    qDebug() << "更新用户状态:" << curUser->getUserName();

    if (ui->labelUserName_3) {
        QString welcomeText = QString("欢迎回来，%1！").arg(curUser->getUserName());
        ui->labelUserName_3->setText(welcomeText);
    }
    if (ui->labelUserName) {
        ui->labelUserName->setText(curUser->getUserName());
    }
}

void personalpage::on_pushButton_clicked()
{
    qDebug() << "修改密码按钮被点击";

    Passwordchange *passwordDialog = new Passwordchange(this);
    passwordDialog->setWindowTitle("修改密码 - " + curUser->getUserName());
    passwordDialog->setWindowFlags(Qt::Window);
    passwordDialog->resize(400, 300);
    passwordDialog->show();
}

void personalpage::on_deleteodr_clicked()
{
    // 获取当前选中的行
    QModelIndex currentIndex = ui->orderView->currentIndex();

    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要取消的订单！");
        return;
    }

    // 获取选中的行号
    int row = currentIndex.row();

    // 获取订单号（第0列）
    QModelIndex orderIdIndex = ordermodel->index(row, 0);
    int orderId = ordermodel->data(orderIdIndex, Qt::DisplayRole).toInt();

    // 获取航班号（第2列）
    QModelIndex flightIdIndex = ordermodel->index(row, 2);
    QString flightInfo = ordermodel->data(flightIdIndex, Qt::DisplayRole).toString();

    // 获取起止城市和时间
    QString startCity = ordermodel->data(ordermodel->index(row, 10), Qt::DisplayRole).toString();
    QString endCity = ordermodel->data(ordermodel->index(row, 11), Qt::DisplayRole).toString();
    QString startTime = ordermodel->data(ordermodel->index(row, 7), Qt::DisplayRole).toString();

    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认取消订单",
                                  QString("您确定要取消以下订单吗？\n\n"
                                          "订单号：%1\n"
                                          "航班：%2\n"
                                          "行程：%3 → %4\n"
                                          "起飞时间：%5\n\n"
                                          "取消后订单将不可恢复！")
                                      .arg(orderId)
                                      .arg(flightInfo)
                                      .arg(startCity)
                                      .arg(endCity)
                                      .arg(startTime),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString error;
        bool success = ordermodel->deleteOrder(orderId, error);

        if (success) {
            QMessageBox::information(this, "成功", QString("订单 #%1 已成功取消！").arg(orderId));
            // 刷新订单列表
            loadUserOrders();

        } else {
            QMessageBox::critical(this, "错误", QString("取消订单失败：%1").arg(error));
        }
    }
}

void personalpage::on_comboBox_activated(int index)
{
    if (index < 0) return;

    int orderId = ui->comboBox->itemData(index).toInt();
    if (orderId <= 0) return;

    selectedOrderId = orderId;  // 保存当前选中的订单ID
    qDebug() << "当前选中订单:" << orderId;

    // 获取订单详细信息
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "数据库未连接";
        return;
    }

    // 修改SQL语句，确保表别名正确
    QSqlQuery query(db);
    QString sql = QString(
                      "SELECT "
                      "o.OrderID, "
                      "o.FlightID, "
                      "f.PlaneID, "
                      "f.StartCity, "
                      "f.EndCity, "
                      "f.StartTime, "
                      "p.PlaneName, "
                      "p.SeatCnt, "
                      "o.Cabintype "
                      "FROM orders o "
                      "LEFT JOIN flights f ON o.FlightID = f.FlightID "
                      "LEFT JOIN planes p ON f.PlaneID = p.PlaneID "
                      "WHERE o.OrderID = %1"
                      ).arg(orderId);

    qDebug() << "执行的SQL:" << sql;  // 添加这行来查看SQL语句

    if (!query.exec(sql)) {
        qDebug() << "查询订单详情失败:" << query.lastError().text();
        return;
    }

    if (query.next()) {
        int flightId = query.value(1).toInt();
        QString planeName = query.value(6).toString();
        QString startCity = query.value(3).toString();
        QString endCity = query.value(4).toString();
        QDateTime startTime = query.value(5).toDateTime();
        int seatCnt = query.value(7).toInt();
        int cabinType = query.value(8).toInt();
        flightSeatCount = seatCnt;

        qDebug() << "查询结果:"
                 << "FlightID:" << flightId
                 << "PlaneName:" << planeName
                 << "StartCity:" << startCity
                 << "EndCity:" << endCity
                 << "SeatCnt:" << seatCnt
                 << "CabinType:" << cabinType;

        // 设置各个标签的显示文本
        if (ui->plane) {
            ui->plane->setText(QString("%1").arg(planeName));
        }
        if (ui->from) {
            ui->from->setText(QString("%1").arg(startCity));
        }
        if (ui->dest) {
            ui->dest->setText(QString("%1").arg(endCity));
        }
        if (ui->time) {
            QString timeStr = startTime.toString("yyyy-MM-dd HH:mm");
            ui->time->setText(QString("%1").arg(timeStr));
        }

        // 重置座位显示
        if (ui->seat) {
            ui->seat->setText("未选择");
        }

        // 查询被占用的座位
        QSet<int> occupiedSeats;
        QSqlQuery occupiedQuery(db);
        QString occupiedSql = QString(
                                  "SELECT s.SeatNo FROM seats s "
                                  "WHERE s.FlightID = %1 AND s.SeatAvailable = 0"
                                  ).arg(flightId);

        if (occupiedQuery.exec(occupiedSql)) {
            while (occupiedQuery.next()) {
                QString seatNo = occupiedQuery.value(0).toString();
                qDebug() << "被占用的座位:" << seatNo;
                // 这里可以根据需要处理座位号
            }
        }

        // 初始化座位选择界面，传入舱位类型
        initSeatSelection(seatCnt, occupiedSeats);

    } else {
        qDebug() << "未找到订单" << orderId << "的详细信息";
        showDefaultMessage("订单信息获取失败");
    }
}
void personalpage::initSeatSelection(int totalSeats, const QSet<int>& occupiedSeats)
{
    QSqlDatabase db = QSqlDatabase::database();

    // 1. 获取订单详细信息
    QSqlQuery orderQuery(db);
    QString orderSql = QString(
                           "SELECT o.FlightID, o.ClientID, o.Cabintype FROM orders o "
                           "WHERE o.OrderID = %1"
                           ).arg(selectedOrderId);

    if (!orderQuery.exec(orderSql) || !orderQuery.next()) {
        QMessageBox::critical(this, "错误", "获取订单信息失败！");
        return;
    }
    int flightId = orderQuery.value(0).toInt();
    int clientId = orderQuery.value(1).toInt();
    int cabinType = orderQuery.value(2).toInt();

    // 2. 查询该航班下该舱位已经被占用的座位
    QString seatClassStr;
    switch (cabinType) {
    case 1: seatClassStr = "economy"; break;
    case 2: seatClassStr = "business"; break;
    case 3: seatClassStr = "first"; break;
    default: seatClassStr = "economy"; break;
    }

    QSet<int> occupiedSeatNos;  // 存储被占用的座位号（数字部分）
    QSqlQuery occupiedQuery(db);
    QString occupiedSql = QString(
                              "SELECT s.SeatNo FROM seats s "
                              "WHERE s.FlightID = %1 AND s.SeatClass = '%2' AND s.SeatAvailable = 0"
                              ).arg(flightId).arg(seatClassStr);

    if (occupiedQuery.exec(occupiedSql)) {
        while (occupiedQuery.next()) {
            QString seatNo = occupiedQuery.value(0).toString();
            // 提取数字部分，如 "C12" -> 12
            if (seatNo.length() > 1) {
                QString numStr = seatNo.mid(1);  // 去掉第一个字母
                bool ok;
                int seatNum = numStr.toInt(&ok);
                if (ok && seatNum > 0) {
                    occupiedSeatNos.insert(seatNum);
                }
            }
        }
    }

    for (const auto& ptr : seat) {
        if (ptr) {
            ui->graphicsView->scene()->removeItem(ptr);
            delete ptr;
        }
    }
    seat.clear();

    // 清空场景
    ui->graphicsView->scene()->clear();

    if (totalSeats <= 0) {
        QGraphicsTextItem *textItem = new QGraphicsTextItem("该航班无可用座位");
        textItem->setFont(QFont("Arial", 16));
        textItem->setDefaultTextColor(Qt::red);
        textItem->setPos(30, 50);
        ui->graphicsView->scene()->addItem(textItem);
        return;
    }

    // 根据舱位类型计算座位数量和起始位置
    int firstClassSeats = 0;
    int businessClassSeats = 0;
    int economyClassSeats = 0;

    // 1:1:8 的比例
    firstClassSeats = qMax(1, totalSeats / 10);           // 头等舱
    businessClassSeats = qMax(1, totalSeats / 10);       // 商务舱
    economyClassSeats = totalSeats - firstClassSeats - businessClassSeats;  // 经济舱

    // 调整，确保总和等于总数
    if (economyClassSeats < 0) {
        economyClassSeats = 0;
        firstClassSeats = totalSeats / 2;
        businessClassSeats = totalSeats - firstClassSeats;
    }

    // 根据舱位类型确定显示的座位范围
    int startSeat = 0;
    int seatsToShow = 0;
    QString cabinName = "";
    QString seatPrefix = "";  // 座位前缀：A, B, C

    switch (cabinType) {
    case 1:  // 经济舱
        startSeat = firstClassSeats + businessClassSeats;
        seatsToShow = economyClassSeats;
        cabinName = "经济舱";
        seatPrefix = "C";
        break;
    case 2:  // 商务舱
        startSeat = firstClassSeats;
        seatsToShow = businessClassSeats;
        cabinName = "商务舱";
        seatPrefix = "B";
        break;
    case 3:  // 头等舱
        startSeat = 0;
        seatsToShow = firstClassSeats;
        cabinName = "头等舱";
        seatPrefix = "A";
        break;
    default:
        startSeat = 0;
        seatsToShow = totalSeats;
        cabinName = "未知舱位";
        seatPrefix = "C";
        break;
    }

    // 重新创建座位
    seat.resize(seatsToShow);

    // 计算布局
    int rowCount = 3;  // 固定3行
    int colCount = (seatsToShow + rowCount - 1) / rowCount;  // 计算列数

    // 在 initSeatSelection 函数中修改
    for (int i = 0; i < seatsToShow; i++) {
        int seatNumber = i + 1;  // 相对座位号（从1开始）
        QString fullSeatNo = seatPrefix + QString::number(seatNumber);

        SeatState status = Available;

        // 判断座位是否被占用
        if (occupiedSeatNos.contains(seatNumber)) {
            status = Occupied;
            qDebug() << "座位" << fullSeatNo << "被占用";
        }

        // 创建座位项
        seat[i] = new SeatItem(i, status);
        seat[i]->setSeatNumber(seatNumber);  // 设置实际座位号
        seat[i]->setSeatName(fullSeatNo);

        // 计算位置
        int row = i % rowCount;
        int col = i / rowCount;
        seat[i]->setPos(col * 50, row * 50);

        ui->graphicsView->scene()->addItem(seat[i]);
        connect(seat[i], &SeatItem::clicked, this, &personalpage::focusedIdChanged);
    }

    // 添加行列标签
    for (int row = 0; row < rowCount; row++) {
        QGraphicsTextItem *rowLabel = new QGraphicsTextItem(QString("行%1").arg(row + 1));
        rowLabel->setFont(QFont("Arial", 10));
        rowLabel->setPos(-30, row * 50);
        ui->graphicsView->scene()->addItem(rowLabel);
    }

    for (int col = 0; col < colCount; col++) {
        QGraphicsTextItem *colLabel = new QGraphicsTextItem(QString("列%1").arg(col + 1));
        colLabel->setFont(QFont("Arial", 10));
        colLabel->setPos(col * 50, rowCount * 50);
        ui->graphicsView->scene()->addItem(colLabel);
    }

    // 添加标题
    QString titleText = QString("请选择座位 (%1)").arg(cabinName);
    QGraphicsTextItem *title = new QGraphicsTextItem(titleText);
    title->setFont(QFont("Arial", 12, QFont::Bold));
    title->setPos((colCount * 50) / 2 - 40, -30);
    ui->graphicsView->scene()->addItem(title);

    // 添加座位号范围说明
    QString rangeText = QString("座位号范围: %11 - %2%3")
                            .arg(seatPrefix)
                            .arg(seatPrefix)
                            .arg(seatsToShow);
    QGraphicsTextItem *rangeLabel = new QGraphicsTextItem(rangeText);
    rangeLabel->setFont(QFont("Arial", 9));
    rangeLabel->setPos((colCount * 50) / 2 - 40, rowCount * 50 + 20);
    ui->graphicsView->scene()->addItem(rangeLabel);

    // 添加图例
    QGraphicsTextItem *legendTitle = new QGraphicsTextItem("图例:");
    legendTitle->setFont(QFont("Arial", 9));
    legendTitle->setPos(-40, rowCount * 50 + 50);
    ui->graphicsView->scene()->addItem(legendTitle);

    // 可用座位
    QGraphicsRectItem *availableLegend = new QGraphicsRectItem(-30, rowCount * 50 + 70, 20, 20);
    availableLegend->setBrush(QBrush(Qt::green));
    availableLegend->setPen(QPen(Qt::black));
    ui->graphicsView->scene()->addItem(availableLegend);

    QGraphicsTextItem *availableText = new QGraphicsTextItem("可用");
    availableText->setFont(QFont("Arial", 8));
    availableText->setPos(0, rowCount * 50 + 70);
    ui->graphicsView->scene()->addItem(availableText);

    // 被占座位
    QGraphicsRectItem *occupiedLegend = new QGraphicsRectItem(-30, rowCount * 50 + 95, 20, 20);
    occupiedLegend->setBrush(QBrush(Qt::gray));
    occupiedLegend->setPen(QPen(Qt::black));
    ui->graphicsView->scene()->addItem(occupiedLegend);

    QGraphicsTextItem *occupiedText = new QGraphicsTextItem("被占");
    occupiedText->setFont(QFont("Arial", 8));
    occupiedText->setPos(0, rowCount * 50 + 95);
    ui->graphicsView->scene()->addItem(occupiedText);

    // 当前选中
    QGraphicsRectItem *selectedLegend = new QGraphicsRectItem(-30, rowCount * 50 + 120, 20, 20);
    selectedLegend->setBrush(QBrush(Qt::yellow));
    selectedLegend->setPen(QPen(Qt::black));
    ui->graphicsView->scene()->addItem(selectedLegend);

    QGraphicsTextItem *selectedText = new QGraphicsTextItem("选中");
    selectedText->setFont(QFont("Arial", 8));
    selectedText->setPos(0, rowCount * 50 + 120);
    ui->graphicsView->scene()->addItem(selectedText);

    // 设置场景大小
    ui->graphicsView->setSceneRect(-50, -50, colCount * 50 + 50, rowCount * 50 + 150);

    // 重置选中座位
    focusedID = -1;
    if (ui->seat) {
        ui->seat->setText(QString("座位: 未选择 (%1)").arg(cabinName));
    }
}

void personalpage::loadUserOrdersToComboBox()
{

    qDebug() << "comboBox userId =" << curUser->getUserID();

    // 获取用户的所有订单
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "数据库未连接";
        return;
    }

    QSqlQuery query(db);
    int userId = curUser->getUserID();
    QString sql = QString(
                      "SELECT OrderID, FlightID FROM orders WHERE ClientID = %1 "
                      "AND (SeatID = 0 OR SeatID IS NULL) "
                      "ORDER BY OrderID"
                      ).arg(userId);

    if (!query.exec(sql)) {
        qDebug() << "查询订单失败:" << query.lastError().text();
        return;
    }

    int orderCount = 0;
    while (query.next()) {
        int orderId = query.value(0).toInt();
        QString flightId = query.value(1).toString();

        // 格式化显示：订单号 - 航班号
        QString displayText = QString("订单 %1").arg(orderId);

        ui->comboBox->addItem(displayText, orderId);
        orderCount++;
    }

    if (orderCount == 0) {
        ui->comboBox->addItem("暂无订单");
    }

    qDebug() << "已加载" << orderCount << "个订单到comboBox";

}
void personalpage::refreshComboBox()
{
    loadUserOrdersToComboBox();

    // 清空座位选择界面
    ui->graphicsView->scene()->clear();

    // 重置状态
    selectedOrderId = -1;
    flightSeatCount = 0;

    // 显示默认提示
    QGraphicsTextItem *textItem = new QGraphicsTextItem("请先选择订单");
    textItem->setFont(QFont("Arial", 20));
    textItem->setDefaultTextColor(Qt::gray);
    textItem->setPos(50, 50);
    ui->graphicsView->scene()->addItem(textItem);

    if (ui->seat) {
        ui->seat->setText("未选择");
    }

    // 清空其他显示
    if (ui->plane) ui->plane->setText("");
    if (ui->from) ui->from->setText("");
    if (ui->dest) ui->dest->setText("");
    if (ui->time) ui->time->setText("");
}


void personalpage::updateSelection(){
    if(ui->checkBox->isChecked()){
        luggageType=0;
    }else if(ui->checkBox_2->isChecked()){
        luggageType=1;
    }
}

void personalpage::on_spinBox_textChanged(const QString &arg1)
{
    int weight = arg1.toInt();

    if (selectedOrderId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择订单！");
        return;
    }

    if (weight <= 0) {
        if (ui->price) {
            ui->price->setText("行李费: ¥0.00");
        }
        return;
    }

    // 获取订单的舱位信息
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "数据库未连接";
        return;
    }

    QSqlQuery query(db);
    QString sql = QString(
                      "SELECT o.Cabintype, o.FlightID, f.PriceLow, f.PriceMid, f.PriceHigh "
                      "FROM orders o "
                      "LEFT JOIN flights f ON o.FlightID = f.FlightID "
                      "WHERE o.OrderID = %1"
                      ).arg(selectedOrderId);

    if (!query.exec(sql) || !query.next()) {
        qDebug() << "获取订单信息失败";
        return;
    }

    int cabinType = query.value(0).toInt();  // 1:经济舱, 2:商务舱, 3:头等舱
    QString flightId = query.value(1).toString();
    double basePrice = 0.0;


    // 不同舱位的行李规则
    int freeWeight = 20;  // 默认经济舱免费额
    double pricePerKg = 20.0;

    switch (cabinType) {
    case 1:  // 经济舱
        freeWeight = 20;
        pricePerKg = 20.0;
        break;
    case 2:  // 商务舱
        freeWeight = 30;
        pricePerKg = 15.0;
        break;
    case 3:  // 头等舱
        freeWeight = 40;
        pricePerKg = 10.0;
        break;
    }

    // 计算行李费
    double luggagePrice = 0.0;
    if (weight > freeWeight) {
        luggagePrice = (weight - freeWeight) * pricePerKg;
    }

    // 更新界面显示
    if (ui->price) {
        ui->price->setText(QString("行李费: ¥%1").arg(QString::number(luggagePrice, 'f', 2)));
    }


    // 保存当前行李重量，用于后续确认
    currentLuggageWeight = weight;
    currentLuggagePrice = luggagePrice;
}

void personalpage::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) {
        ui->checkBox_2->setChecked(false);
    }
    ui->spinBox->setEnabled(false);
    updateSelection();
}

void personalpage::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::Checked) {
        ui->checkBox->setChecked(false);
    }
    ui->spinBox->setEnabled(true);
    updateSelection();
}

void personalpage::on_pushButton_2_clicked()
{
    if (selectedOrderId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择订单！");
        return;
    }

    // 检查是否有选择座位
    if (focusedID < 0) {
        QMessageBox::warning(this, "提示", "请先选择座位！");
        return;
    }

    int seatNumber = focusedID + 1;  // 座位号从1开始

    // 获取订单信息用于验证
    QSqlDatabase db = QSqlDatabase::database();

    // 1. 获取订单详细信息
    QSqlQuery orderQuery(db);
    QString orderSql = QString(
                           "SELECT o.FlightID, o.ClientID, o.Cabintype FROM orders o "
                           "WHERE o.OrderID = %1"
                           ).arg(selectedOrderId);

    if (!orderQuery.exec(orderSql) || !orderQuery.next()) {
        QMessageBox::critical(this, "错误", "获取订单信息失败！");
        return;
    }

    int flightId = orderQuery.value(0).toInt();
    int clientId = orderQuery.value(1).toInt();
    int cabinType = orderQuery.value(2).toInt();  // 舱位类型：1经济，2商务，3头等

    // 2. 将座位号转换为 SeatNo（如经济舱C1，商务舱B1，头等舱A1）
    QString seatNo;
    switch (cabinType) {
    case 1:  // 经济舱
        seatNo = "C" + QString::number(seatNumber);
        break;
    case 2:  // 商务舱
        seatNo = "B" + QString::number(seatNumber);
        break;
    case 3:  // 头等舱
        seatNo = "A" + QString::number(seatNumber);
        break;
    default:
        seatNo = "C" + QString::number(seatNumber);
        break;
    }

    // 3. 检查座位是否已被占用
    QSqlQuery checkQuery(db);
    QString checkSql = QString(
                           "SELECT COUNT(*) FROM orders o "
                           "JOIN seats s ON o.SeatID = s.SeatID "
                           "WHERE o.FlightID = %1 AND s.SeatNo = '%2' AND o.OrderID != %3"
                           ).arg(flightId)
                           .arg(seatNo)
                           .arg(selectedOrderId);

    if (!checkQuery.exec(checkSql)) {
        QMessageBox::critical(this, "错误", "检查座位状态失败：" + checkQuery.lastError().text());
        return;
    }

    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "提示", QString("座位 %1 已被占用，请选择其他座位！").arg(seatNo));
        return;
    }

    bool hasLuggage = (luggageType == 1);
    int luggageWeight = hasLuggage ? currentLuggageWeight : 0;
    double luggagePrice = hasLuggage ? currentLuggagePrice : 0.0;

    if (hasLuggage && luggageWeight <= 0) {
        QMessageBox::warning(this, "提示", "请设置托运行李重量！");
        return;
    }

    // 4. 开始事务
    db.transaction();

    try {
        // 5. 查找或创建座位记录
        QString seatClassStr;
        switch (cabinType) {
        case 1: seatClassStr = "economy"; break;
        case 2: seatClassStr = "business"; break;
        case 3: seatClassStr = "first"; break;
        default: seatClassStr = "economy"; break;
        }

        // 检查座位是否已存在
        QString findSeatSql = QString(
                                  "SELECT SeatID, SeatAvailable FROM seats "
                                  "WHERE FlightID = %1 AND SeatNo = '%2'"
                                  ).arg(flightId).arg(seatNo);

        QSqlQuery findSeatQuery(db);
        int seatId = 0;
        bool seatExists = false;
        bool seatAvailable = true;

        if (findSeatQuery.exec(findSeatSql) && findSeatQuery.next()) {
            seatId = findSeatQuery.value(0).toInt();
            seatAvailable = (findSeatQuery.value(1).toInt() == 1);
            seatExists = true;
            qDebug() << "找到已存在的座位记录，SeatID:" << seatId << "SeatNo:" << seatNo << "是否可用:" << seatAvailable;
        }

        if (!seatExists) {
            // 插入新座位记录
            QString insertSeatSql = QString(
                                        "INSERT INTO seats (FlightID, SeatNo, SeatClass, SeatAvailable) "
                                        "VALUES (%1, '%2', '%3', 0)"
                                        ).arg(flightId)
                                        .arg(seatNo)
                                        .arg(seatClassStr);

            QSqlQuery insertSeatQuery(db);
            if (!insertSeatQuery.exec(insertSeatSql)) {
                throw QString("创建座位记录失败：") + insertSeatQuery.lastError().text();
            }

            seatId = insertSeatQuery.lastInsertId().toInt();
            qDebug() << "创建新座位记录成功，SeatID:" << seatId << "SeatNo:" << seatNo;

        } else {
            // 检查座位是否可用
            if (!seatAvailable) {
                // 检查是否是同一个订单占用了这个座位
                QString checkSameOrderSql = QString(
                                                "SELECT COUNT(*) FROM orders "
                                                "WHERE OrderID = %1 AND SeatID = %2"
                                                ).arg(selectedOrderId).arg(seatId);

                QSqlQuery sameOrderQuery(db);
                if (sameOrderQuery.exec(checkSameOrderSql) && sameOrderQuery.next()) {
                    int sameOrderCount = sameOrderQuery.value(0).toInt();
                    if (sameOrderCount == 0) {
                        throw QString("座位 %1 已被其他订单占用！").arg(seatNo);
                    } else {
                        // 如果是同一个订单，允许继续操作
                        qDebug() << "订单" << selectedOrderId << "已占用座位" << seatNo << "，允许重新选择";
                    }
                } else {
                    throw QString("座位 %1 已被占用！").arg(seatNo);
                }
            }
        }

        // 6. 更新座位状态为不可用
        QString updateSeatSql = QString(
                                    "UPDATE seats SET SeatAvailable = 0 "
                                    "WHERE SeatID = %1"
                                    ).arg(seatId);
        QSqlQuery updateSeatQuery(db);
        if (!updateSeatQuery.exec(updateSeatSql)) {
            throw QString("更新座位状态失败：") + updateSeatQuery.lastError().text();
        }

        // 7. 更新或创建行李记录 - 改为每个订单独立
        int luggageId = 0;
        if (hasLuggage) {
            // 先检查这个订单是否已经有行李记录
            QString getOrderLuggageSql = QString(
                                             "SELECT LuggageID FROM orders WHERE OrderID = %1"
                                             ).arg(selectedOrderId);

            QSqlQuery getLuggageQuery(db);
            if (getLuggageQuery.exec(getOrderLuggageSql) && getLuggageQuery.next()) {
                luggageId = getLuggageQuery.value(0).toInt();
            }

            if (luggageId > 0) {
                // 更新这个订单现有的行李记录
                QString updateLuggageSql = QString(
                                               "UPDATE luggages SET Weight = %1, Price = %2 "
                                               "WHERE LuggageID = %3"
                                               ).arg(luggageWeight)
                                               .arg(luggagePrice)
                                               .arg(luggageId);

                QSqlQuery updateQuery(db);
                if (!updateQuery.exec(updateLuggageSql)) {
                    throw QString("更新行李信息失败：") + updateQuery.lastError().text();
                }
                qDebug() << "更新了订单" << selectedOrderId << "的行李记录，LuggageID:" << luggageId;
            } else {
                // 为这个订单创建新的独立行李记录
                QString insertLuggageSql = QString(
                                               "INSERT INTO luggages (FlightID, ClientID, Weight, Price) "
                                               "VALUES (%1, %2, %3, %4)"
                                               ).arg(flightId)
                                               .arg(clientId)
                                               .arg(luggageWeight)
                                               .arg(luggagePrice);

                QSqlQuery insertQuery(db);
                if (!insertQuery.exec(insertLuggageSql)) {
                    throw QString("创建行李信息失败：") + insertQuery.lastError().text();
                }

                luggageId = insertQuery.lastInsertId().toInt();
                qDebug() << "为订单" << selectedOrderId << "创建了新行李记录，LuggageID:" << luggageId;
            }
        } else {
            // 不托运，删除这个订单的行李记录
            // 先获取这个订单当前的行李ID
            QString getCurrentLuggageSql = QString(
                                               "SELECT LuggageID FROM orders WHERE OrderID = %1"
                                               ).arg(selectedOrderId);

            QSqlQuery getQuery(db);
            if (getQuery.exec(getCurrentLuggageSql) && getQuery.next()) {
                int currentLuggageId = getQuery.value(0).toInt();
                if (currentLuggageId > 0) {
                    // 只删除这个行李记录
                    QString deleteLuggageSql = QString(
                                                   "DELETE FROM luggages WHERE LuggageID = %1"
                                                   ).arg(currentLuggageId);

                    QSqlQuery deleteQuery(db);
                    if (!deleteQuery.exec(deleteLuggageSql)) {
                        qDebug() << "删除行李记录失败:" << deleteQuery.lastError().text();
                        // 这里不抛出异常，因为可能已经被其他逻辑删除了
                    } else {
                        qDebug() << "删除了订单" << selectedOrderId << "的行李记录，LuggageID:" << currentLuggageId;
                    }
                }
            }
            luggageId = 0;  // 设为0表示无行李
        }

        // 8. 更新订单的座位ID和行李ID
        QString updateOrderSql;
        if (hasLuggage && luggageId > 0) {
            updateOrderSql = QString(
                                 "UPDATE orders SET SeatID = %1, LuggageID = %2 "
                                 "WHERE OrderID = %3"
                                 ).arg(seatId)  // 存储 seats 表的 SeatID
                                 .arg(luggageId)
                                 .arg(selectedOrderId);
        } else {
            updateOrderSql = QString(
                                 "UPDATE orders SET SeatID = %1, LuggageID = NULL "
                                 "WHERE OrderID = %2"
                                 ).arg(seatId)  // 存储 seats 表的 SeatID
                                 .arg(selectedOrderId);
        }

        QSqlQuery orderUpdateQuery(db);
        if (!orderUpdateQuery.exec(updateOrderSql)) {
            throw QString("更新订单信息失败：") + orderUpdateQuery.lastError().text();
        }

        // 9. 提交事务
        if (!db.commit()) {
            throw QString("提交事务失败：") + db.lastError().text();
        }

        // 10. 成功提示
        QString successMessage = QString("值机成功！\n\n"
                                         "订单号: %1\n"
                                         "航班号: %2\n"
                                         "座位号: %3\n"
                                         "座位类型: %4")
                                     .arg(selectedOrderId)
                                     .arg(flightId)
                                     .arg(seatNo)  // 显示 SeatNo
                                     .arg(seatClassStr);

        if (hasLuggage) {
            successMessage += QString("\n托运行李: %1kg\n"
                                      "行李费用: ¥%2")
                                  .arg(luggageWeight)
                                  .arg(QString::number(luggagePrice, 'f', 2));
        } else {
            successMessage += "\n托运行李: 无";
        }

        QMessageBox::information(this, "成功", successMessage);

        // 11. 刷新界面
        loadUserOrders();  // 刷新订单列表
        loadUserOrdersToComboBox();  // 刷新下拉框

        if (ordermodel) {
            ordermodel->select();  // 刷新订单模型
        }

        // 12. 清空座位选择界面
        ui->graphicsView->scene()->clear();
        QGraphicsTextItem *textItem = new QGraphicsTextItem("请先选择订单");
        textItem->setFont(QFont("Arial", 20));
        textItem->setDefaultTextColor(Qt::gray);
        textItem->setPos(50, 50);
        ui->graphicsView->scene()->addItem(textItem);

        // 13. 重置所有状态
        selectedOrderId = -1;
        flightSeatCount = 0;
        focusedID = -1;

        if (ui->seat) ui->seat->setText("已确认");
        if (ui->plane) ui->plane->setText("");
        if (ui->from) ui->from->setText("");
        if (ui->dest) ui->dest->setText("");
        if (ui->time) ui->time->setText("");
        if (ui->price) ui->price->setText("");

        // 14. 重置行李选择
        ui->checkBox->setChecked(true);
        ui->checkBox_2->setChecked(false);
        luggageType = 0;
        currentLuggageWeight = 0;
        currentLuggagePrice = 0.0;
        ui->spinBox->setValue(0);

        qDebug() << "值机成功完成，订单:" << selectedOrderId
                 << "SeatID:" << seatId
                 << "SeatNo:" << seatNo;

    } catch (const QString &error) {
        // 回滚事务
        db.rollback();
        QMessageBox::critical(this, "错误", error);
    }
}
