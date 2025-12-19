#include <QMessageBox>
#include <QDebug>
#include "flightwidget.h"
#include "ui_flightwidget.h"
#include "addfoodwindow.h"
#include "../entity/flight.h"
#include "../entity/company.h"
#include "../entity/user.h"
#include "../entity/admin.h"
#include "../model/flightmodel.h"
#include "../model/flightfilterproxymodel.h"
#include "../view/weatherforecastform.h"
#include "../view/setorder.h"  // 订单窗口头文件
#include "../controller/controllerfood.h"
#include "../entity/order.h"  // 订单实体头文件
#include "../model/ordermodel.h"  // 订单模型头文件

extern std::map<int, Company *> companies;
extern User *curUser;

FlightWidget::FlightWidget(QWidget *parent,FlightModel *_flightmodel)
    : QWidget(parent)
    , flightmodel(_flightmodel)
    , ui(new Ui::FlightWidget)
{
    ui->setupUi(this);
    GetCompanies();
    ui->dtmSearchStartTimeFrom->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
    ui->dtmSearchStartTimeTo->setDateTime(QDateTime(QDate(2050, 1, 1), QTime(23, 59, 59)));
    ui->dspbxSearchPriceLowFrom->setValue(00.00);
    ui->dspbxSearchPriceLowTo->setValue(99999.99);
    ui->spxSeatHigh->setMaximum(1000);
    ui->spxSeatLow->setMaximum(1000);
    ui->spxSeatMid->setMaximum(1000);
    ui->dspbxPriceHigh->setMaximum(99999.99);
    ui->dspbxPriceLow->setMaximum(99999.99);
    ui->dspbxPriceMid->setMaximum(99999.99);
    ui->spxSeatLow->setReadOnly(true);
    ui->spxSeatMid->setReadOnly(true);
    ui->spxSeatHigh->setReadOnly(true);

    // 创建controllerflight用于管理航班
    controllerflight = new ControllerFlight(this, flightmodel);
    connect(controllerflight, &ControllerFlight::TableFlightsModified, this, &FlightWidget::refreshDatabase);
    // 创建flightproxymodel
    flightproxymodel = new FlightFilterProxyModel(this);
    flightproxymodel->setSourceModel(flightmodel);
    flightproxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    flightproxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    // tableFlight视图关联flightproxymodel
    ui->tableFlight->setModel(flightproxymodel);
    ui->tableFlight->setSortingEnabled(true);
    ui->tableFlight->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止在表格中直接编辑
    ui->tableFlight->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    ui->tableFlight->setSelectionMode(QAbstractItemView::ExtendedSelection); // 多选
    ui->tableFlight->setColumnHidden(0, true);
    ui->tableFlight->sortByColumn(0, Qt::AscendingOrder);
    // 创建flightmodelmapper绑定flightmodel
    flightmodelmapper = new QDataWidgetMapper(this);
    flightmodelmapper->setModel(flightmodel);
    flightmodelmapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    // 映射模型字段到详细信息控件
    flightmodelmapper->addMapping(ui->txtStartCity, 5);
    flightmodelmapper->addMapping(ui->txtEndCity, 6);
    flightmodelmapper->addMapping(ui->dtmStartTime, 7);
    flightmodelmapper->addMapping(ui->dtmEndTime, 8);
    flightmodelmapper->addMapping(ui->dspbxPriceLow, 10);
    flightmodelmapper->addMapping(ui->dspbxPriceMid, 11);
    flightmodelmapper->addMapping(ui->dspbxPriceHigh, 12);
    flightmodelmapper->addMapping(ui->txtMessage, 16);
    // 绑定表格选择到flightmodelmapper
    connect(ui->tableFlight->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &FlightWidget::onFlightSelectionChanged);
    // 创建刷新定时器
    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(timerefreshMs);
    connect(refreshTimer, &QTimer::timeout, this, &FlightWidget::refreshDatabase);
    refreshTimer->start();
    //设成游客模式
    SetTouristMode();

    // 初始化订单模型
    ordermodel = new OrderModel(this);
    ordermodel->init();
}

FlightWidget::~FlightWidget()
{
    delete ui;
}
void FlightWidget::GetCompanies()
{
    ui->cbxCompanyName->clear();
    ui->cbxSearchCompanyName->clear();
    for (const auto &pr : companies) {
        ui->cbxCompanyName->addItem(pr.second->getCompanyName(), QVariant(pr.second->getCompanyID()));
        ui->cbxSearchCompanyName->addItem(pr.second->getCompanyName(), QVariant(pr.second->getCompanyID()));
    }
    ui->cbxCompanyName->setCurrentIndex(-1);
    ui->cbxSearchCompanyName->setCurrentIndex(-1);
}
void FlightWidget::setAdminAccess()
{
    int idx = ui->cbxCompanyName->currentIndex(), cid = -1;
    if (idx != -1) cid = ui->cbxCompanyName->itemData(idx).toInt();

    ui->cbxPlaneName->clear();
    if (cid > 0 && companies.find(cid) != companies.end()) {
        Company *ptr = companies[cid];
        for (const auto &plane : ptr->Planes) {
            ui->cbxPlaneName->addItem(plane.getPlaneName(), QVariant(plane.getPlaneID()));
        }
    }

    if (curUser && curUser->getUserType() == "admin" && dynamic_cast<Admin *>(curUser)->getCompanyID() == cid) {
        ui->cbxPlaneName->setEnabled(true);
        ui->txtStartCity->setReadOnly(false);
        ui->txtEndCity->setReadOnly(false);
        ui->dtmStartTime->setReadOnly(false);
        ui->dtmEndTime->setReadOnly(false);
        ui->dspbxPriceLow->setReadOnly(false);
        ui->dspbxPriceMid->setReadOnly(false);
        ui->dspbxPriceHigh->setReadOnly(false);
        ui->txtMessage->setReadOnly(false);
        ui->btnAddFlight->setEnabled(true);
        ui->btnDeleteFlight->setEnabled(true);
        ui->btnModifyFlight->setEnabled(true);
        ui->btnBatchModify->setEnabled(true);
        ui->btnBatchDelete->setEnabled(true);
    } else {
        ui->cbxPlaneName->setEnabled(false);
        ui->txtStartCity->setReadOnly(true);
        ui->txtEndCity->setReadOnly(true);
        ui->dtmStartTime->setReadOnly(true);
        ui->dtmEndTime->setReadOnly(true);
        ui->dspbxPriceLow->setReadOnly(true);
        ui->dspbxPriceMid->setReadOnly(true);
        ui->dspbxPriceHigh->setReadOnly(true);
        ui->txtMessage->setReadOnly(true);
        ui->btnDeleteFlight->setEnabled(false);
        ui->btnModifyFlight->setEnabled(false);
        ui->btnAddFlight->setEnabled(false);
        ui->btnBatchModify->setEnabled(false);
        ui->btnBatchDelete->setEnabled(false);
    }
}
void FlightWidget::RefreshCompanies()
{
    GetCompanies();
}
void FlightWidget::SetTouristMode()
{
    ui->tableFlight->setColumnHidden(16, true);

    ui->cbxCompanyName->setEnabled(false);
    ui->cbxPlaneName->setEnabled(false);
    ui->txtStartCity->setReadOnly(true);
    ui->txtEndCity->setReadOnly(true);
    ui->dtmStartTime->setReadOnly(true);
    ui->dtmEndTime->setReadOnly(true);

    ui->dspbxPriceLow->setReadOnly(true);
    ui->dspbxPriceMid->setReadOnly(true);
    ui->dspbxPriceHigh->setReadOnly(true);
    ui->rbnLow->hide();
    ui->rbnMid->hide();
    ui->rbnHigh->hide();

    ui->labelFoodName->hide();
    ui->cbxFoodName->hide();
    ui->btnAddFood->hide();

    ui->labelMessage->hide();
    ui->txtMessage->hide();
    ui->btnOrder->hide();

    ui->btnAddFlight->hide();
    ui->btnDeleteFlight->hide();
    ui->btnModifyFlight->hide();

    ui->btnWeather->hide();

    ui->labelTimeDelta->hide();
    ui->dspxTimeDelta->hide();
    ui->labelUpdMess->hide();
    ui->txtUpdMess->hide();
    ui->btnBatchDelete->hide();
    ui->btnBatchModify->hide();
    ui->btnBatchDelete->hide();
    ui->btnBatchModify->hide();
}

void FlightWidget::SetClientMode()
{
    ui->tableFlight->setColumnHidden(16, true);

    ui->cbxCompanyName->setEnabled(false);
    ui->cbxPlaneName->setEnabled(false);
    ui->txtStartCity->setReadOnly(true);
    ui->txtEndCity->setReadOnly(true);
    ui->dtmStartTime->setReadOnly(true);
    ui->dtmEndTime->setReadOnly(true);

    ui->dspbxPriceLow->setReadOnly(true);
    ui->dspbxPriceMid->setReadOnly(true);
    ui->dspbxPriceHigh->setReadOnly(true);
    ui->rbnLow->show();
    ui->rbnMid->show();
    ui->rbnHigh->show();

    ui->labelFoodName->show();
    ui->cbxFoodName->show();
    ui->cbxFoodName->setEnabled(true);
    ui->btnAddFood->hide();

    ui->labelMessage->hide();
    ui->txtMessage->hide();
    ui->btnOrder->show();

    ui->btnAddFlight->hide();
    ui->btnDeleteFlight->hide();
    ui->btnModifyFlight->hide();

    ui->btnWeather->hide();

    ui->labelTimeDelta->hide();
    ui->dspxTimeDelta->hide();
    ui->labelUpdMess->hide();
    ui->txtUpdMess->hide();
    ui->btnBatchDelete->hide();
    ui->btnBatchModify->hide();
}

void FlightWidget::SetAdminMode()
{
    ui->tableFlight->setColumnHidden(16, false);

    ui->cbxCompanyName->setEnabled(true);

    ui->rbnLow->hide();
    ui->rbnMid->hide();
    ui->rbnHigh->hide();

    ui->labelFoodName->show();
    ui->cbxFoodName->hide();
    ui->btnAddFood->show();

    ui->labelMessage->show();
    ui->txtMessage->show();
    ui->btnOrder->hide();

    ui->btnDeleteFlight->show();
    ui->btnModifyFlight->show();
    ui->btnAddFlight->show();

    ui->btnWeather->show();

    ui->labelTimeDelta->show();
    ui->dspxTimeDelta->show();
    ui->labelUpdMess->show();
    ui->txtUpdMess->show();
    ui->btnBatchDelete->show();
    ui->btnBatchModify->show();

    setAdminAccess();
}
void FlightWidget::refreshDatabase()
{
    QModelIndex preViewIdx = ui->tableFlight->currentIndex();
    QModelIndex preSrcIdx = flightproxymodel->mapToSource(preViewIdx);
    int preSrcRow = preSrcIdx.row();

    // 刷新数据表
    flightmodel->select();

    // 恢复选中行（如果依然存在）
    if (preSrcRow >= 0 && preSrcRow < flightmodel->rowCount()) {
        QModelIndex srcIdx = flightmodel->index(preSrcRow, 0);
        QModelIndex viewIdx = flightproxymodel->mapFromSource(srcIdx);
        ui->tableFlight->setCurrentIndex(viewIdx);
    }

    // 重新过滤
    flightproxymodel->refreshFilter();

    qDebug() << "refresh flightwidget!";
}
void FlightWidget::on_btnSearch_clicked()
{
    flightproxymodel->CompanyName = ui->cbxSearchCompanyName->currentText().trimmed();
    flightproxymodel->StartCity = ui->txtSearchStartCity->text().trimmed();
    flightproxymodel->EndCity = ui->txtSearchEndCity->text().trimmed();
    flightproxymodel->StartTimeFrom = ui->dtmSearchStartTimeFrom->dateTime();
    flightproxymodel->StartTimeTo = ui->dtmSearchStartTimeTo->dateTime();
    flightproxymodel->PriceLowFrom = ui->dspbxSearchPriceLowFrom->value();
    flightproxymodel->PriceLowTo = ui->dspbxSearchPriceLowTo->value();
    flightproxymodel->refreshFilter();
}

void FlightWidget::on_btnSearchClear_clicked()
{
    ui->cbxSearchCompanyName->setCurrentIndex(-1);
    ui->txtSearchStartCity->clear();
    ui->txtSearchEndCity->clear();
    ui->dtmSearchStartTimeFrom->setDateTime(QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0)));
    ui->dtmSearchStartTimeTo->setDateTime(QDateTime(QDate(2050, 1, 1), QTime(23, 59, 59)));
    ui->dspbxSearchPriceLowFrom->setValue(00.00);
    ui->dspbxSearchPriceLowTo->setValue(99999.99);
}
void FlightWidget::onFlightSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid()) return;

    QModelIndex srcIndex = flightproxymodel->mapToSource(current);
    int row = srcIndex.row();
    QString err;
    Flight flight;
    if (!flightmodel->SelectByRow(row, flight, err)) {
        QMessageBox::warning(this, "警告", err);
        return;
    }

    //航司下拉框回显（阻塞信号避免重复触发）
    ui->cbxCompanyName->blockSignals(true);
    int cbxcid = ui->cbxCompanyName->findData(flight.getCompanyID());
    ui->cbxCompanyName->setCurrentIndex(cbxcid);
    ui->cbxCompanyName->blockSignals(false);

    setAdminAccess();

    //机型下拉框回显
    ui->cbxPlaneName->blockSignals(true);
    int cbxpid = ui->cbxPlaneName->findData(flight.getPlaneID());
    ui->cbxPlaneName->setCurrentIndex(cbxpid);
    ui->cbxPlaneName->blockSignals(false);

    //剩余票数回显
    seatCnt=flight.getSeatCnt();
    int economics=seatCnt*0.8;
    int business=seatCnt*0.1;
    int firstclass=seatCnt-economics-business;
    ui->spxSeatLow->setValue(economics-flight.getSeatLow());
    ui->spxSeatMid->setValue(business-flight.getSeatMid());
    ui->spxSeatHigh->setValue(firstclass-flight.getSeatHigh());

    //选餐下拉框回显
    if(curUser && curUser->getUserType() == "client") {
        loadFoodsToComboBox(flight.getFlightID());
    }

    flightmodelmapper->setCurrentModelIndex(srcIndex);
}
void FlightWidget::on_cbxCompanyName_currentIndexChanged(int index)
{
    setAdminAccess();
}
void FlightWidget::on_cbxPlaneName_currentIndexChanged(int index)
{
    int cidx = ui->cbxCompanyName->currentIndex();
    if (cidx == -1) {
        ui->spxSeatLow->setValue(-1);
        ui->spxSeatMid->setValue(-1);
        ui->spxSeatHigh->setValue(-1);
        return;
    }
    int cid = ui->cbxCompanyName->itemData(cidx).toInt();

    int pidx = ui->cbxPlaneName->currentIndex();
    if(pidx == -1) {
        ui->spxSeatLow->setValue(-1);
        ui->spxSeatMid->setValue(-1);
        ui->spxSeatHigh->setValue(-1);
        return;
    }
    int pid = ui->cbxPlaneName->itemData(pidx).toInt();

    if(companies.find(cid)==companies.end()){
        ui->spxSeatLow->setValue(-1);
        ui->spxSeatMid->setValue(-1);
        ui->spxSeatHigh->setValue(-1);
        return;
    }

    Company *ptr = companies[cid];
    for(const auto &plane:ptr->Planes){
        if(plane.getPlaneID()==pid){
            int totalSeats=plane.getSeatCnt();
            seatCnt=totalSeats;
            int seatLow=totalSeats*0.8;
            int seatMid=totalSeats*0.1;
            int seatHigh=totalSeats-seatLow-seatMid;

            QModelIndex viewIdx = ui->tableFlight->currentIndex();
            if(!viewIdx.isValid()){
                ui->spxSeatLow->setValue(-1);
                ui->spxSeatMid->setValue(-1);
                ui->spxSeatHigh->setValue(-1);
                return;
            }
            QModelIndex srcIdx = flightproxymodel->mapToSource(viewIdx);
            int srcRow = srcIdx.row();
            QString err;
            Flight flight;
            if (!flightmodel->SelectByRow(srcRow, flight, err)) {
                ui->spxSeatLow->setValue(-1);
                ui->spxSeatMid->setValue(-1);
                ui->spxSeatHigh->setValue(-1);
                return;
            }

            ui->spxSeatLow->setValue(seatLow-flight.getSeatLow());
            ui->spxSeatMid->setValue(seatMid-flight.getSeatMid());
            ui->spxSeatHigh->setValue(seatHigh-flight.getSeatHigh());
            return;
        }
    }
    ui->spxSeatLow->setValue(-1);
    ui->spxSeatMid->setValue(-1);
    ui->spxSeatHigh->setValue(-1);
}

void FlightWidget::on_btnAddFlight_clicked()
{
    QString err;
    Flight flight;
    flight.setCompanyID(ui->cbxCompanyName->itemData(ui->cbxCompanyName->currentIndex()).toInt());
    flight.setPlaneID(ui->cbxPlaneName->itemData(ui->cbxPlaneName->currentIndex()).toInt());
    flight.setSeatCnt(seatCnt);
    flight.setStartCity(ui->txtStartCity->text().trimmed());
    flight.setEndCity(ui->txtEndCity->text().trimmed());
    QDateTime t1 = ui->dtmStartTime->dateTime();
    QDateTime t2 = ui->dtmEndTime->dateTime();
    double dur = t1.secsTo(t2) / 3600.00;
    flight.setStartTime(t1);
    flight.setEndTime(t2);
    flight.setDuration(dur);
    flight.setPrice(ui->dspbxPriceLow->value(), ui->dspbxPriceMid->value(), ui->dspbxPriceHigh->value());
    flight.setSeatLow(0),flight.setSeatMid(0),flight.setSeatHigh(0);//新增航班，已购票数均为0
    flight.setMessage(ui->txtMessage->text().trimmed());
    if (!controllerflight->AddFlight(flight, err)) {
        QMessageBox::warning(this, "警告", err);
    } else {
        QMessageBox::information(this, "提示", "航班增加成功！");
    }
}
void FlightWidget::on_btnDeleteFlight_clicked()
{
    QString err;
    QModelIndex viewIdx = ui->tableFlight->currentIndex();
    QModelIndex srcIdx = flightproxymodel->mapToSource(viewIdx);
    int srcRow = srcIdx.row();
    if (!controllerflight->RemoveFlight(srcRow, err)) {
        QMessageBox::warning(this, "警告", err);
    } else {
        QMessageBox::information(this, "提示", "航班删除成功！");
    }
}
void FlightWidget::on_btnModifyFlight_clicked()
{
    QString err;
    QModelIndex viewIdx = ui->tableFlight->currentIndex();
    QModelIndex srcIdx = flightproxymodel->mapToSource(viewIdx);
    int srcRow = srcIdx.row();
    Flight flight;
    flight.setCompanyID(ui->cbxCompanyName->itemData(ui->cbxCompanyName->currentIndex()).toInt());
    flight.setPlaneID(ui->cbxPlaneName->itemData(ui->cbxPlaneName->currentIndex()).toInt());
    flight.setSeatCnt(seatCnt);
    flight.setStartCity(ui->txtStartCity->text().trimmed());
    flight.setEndCity(ui->txtEndCity->text().trimmed());
    QDateTime t1 = ui->dtmStartTime->dateTime();
    QDateTime t2 = ui->dtmEndTime->dateTime();
    double dur = t1.secsTo(t2) / 3600.00;
    flight.setStartTime(t1);
    flight.setEndTime(t2);
    flight.setDuration(dur);
    flight.setPrice(ui->dspbxPriceLow->value(), ui->dspbxPriceMid->value(), ui->dspbxPriceHigh->value());
    //已购票数信息不修改
    flight.setMessage(ui->txtMessage->text().trimmed());
    if (!controllerflight->ModifyFlight(srcRow, flight, err)) {
        QMessageBox::warning(this, "警告", err);
    } else {
        QMessageBox::information(this, "提示", "航班修改成功！");
    }
}
void FlightWidget::on_btnBatchModify_clicked()
{
    QString err;
    std::vector<int> rows;
    QModelIndexList selectedIndexes = ui->tableFlight->selectionModel()->selectedRows();
    for (const QModelIndex &ViewIdx : selectedIndexes) {
        QModelIndex SrcIdx = flightproxymodel->mapToSource(ViewIdx);
        int SrcRow = SrcIdx.row();
        rows.push_back(SrcRow);
    }
    if (!controllerflight->BatchModify(rows, dynamic_cast<Admin *>(curUser)->getCompanyID(), ui->dspxTimeDelta->value(), ui->txtUpdMess->text().trimmed(), err)) {
        QMessageBox::warning(this, "警告", err);
    } else {
        QMessageBox::information(this, "提示", "批量修改成功！");
    }
}

void FlightWidget::on_btnBatchDelete_clicked()
{
    QString err;
    std::vector<int> rows;
    QModelIndexList selectedIndexes = ui->tableFlight->selectionModel()->selectedRows();
    for (const QModelIndex &ViewIdx : selectedIndexes) {
        QModelIndex SrcIdx = flightproxymodel->mapToSource(ViewIdx);
        int SrcRow = SrcIdx.row();
        rows.push_back(SrcRow);
    }
    if (!controllerflight->BatchDelete(rows, dynamic_cast<Admin *>(curUser)->getCompanyID(), err)) {
        QMessageBox::warning(this, "警告", err);
    } else {
        QMessageBox::information(this, "提示", "批量删除成功！");
    }
}
void FlightWidget::on_btnWeather_clicked()
{
    WeatherForecastForm *weatherforecastform = new WeatherForecastForm(nullptr, controllerflight);
    weatherforecastform->setAttribute(Qt::WA_DeleteOnClose);
    weatherforecastform->show();
}

void FlightWidget::on_btnAddFood_clicked()
{
    FoodModel *foodModel = new FoodModel(this, flightmodel); // 传入已有的 flightmodel
    foodModel->init();

    // 创建业务控制器（关联模型）
    ControllerFood *foodController = new ControllerFood(this, foodModel, flightmodel);

    // 创建UI窗口
    AddFoodWindow *addFoodWin = new AddFoodWindow(nullptr);
    addFoodWin->setAttribute(Qt::WA_DeleteOnClose);
    // 建立UI与Controller的信号槽关联（UI触发事件→Controller处理业务）
    connect(addFoodWin, &AddFoodWindow::signalAddFood,
            foodController, &ControllerFood::handleAddFood);
    connect(addFoodWin, &AddFoodWindow::signalDeleteFood,
            foodController, &ControllerFood::handleDeleteFood);
    connect(addFoodWin, &AddFoodWindow::signalQueryFood,
            foodController, &ControllerFood::handleQueryFood);

    // 建立Controller与UI的信号槽关联（Controller返回结果→UI更新）
    connect(foodController, &ControllerFood::signalOperationResult,
            addFoodWin, &AddFoodWindow::slotShowResult);
    connect(foodController, &ControllerFood::signalFoodListUpdated,
            addFoodWin, &AddFoodWindow::slotUpdateFoodList);

    // 显示UI窗口
    addFoodWin->show();

    // 确保Controller和Model随UI窗口释放（避免内存泄漏）
    connect(addFoodWin, &QWidget::destroyed, [=]() {
        foodController->deleteLater();
        foodModel->deleteLater();
    });
}

void FlightWidget::on_tableFlight_activated(const QModelIndex &index)
{
    // 表格激活事件的实现
    qDebug() << "航班表格行被激活：" << index.row();
}

void FlightWidget::on_btnOrder_clicked()
{
    QModelIndex currentIndex = ui->tableFlight->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "警告", "请先选择一个航班！");
        return;
    }
    QModelIndex srcIndex = flightproxymodel->mapToSource(currentIndex);
    int row = srcIndex.row();
    QString err;
    Flight flight;
    if (!flightmodel->SelectByRow(row, flight, err)) {
        QMessageBox::warning(this, "警告", "获取航班信息失败：" + err);
        return;
    }

    QString cabinType;
    double price = 0.0;
    if(ui->rbnLow->isChecked()){
        cabinType = "经济舱";
        price=flight.getPriceLow();
    }
    else if(ui->rbnMid->isChecked()){
        cabinType = "商务舱";
        price=flight.getPriceMid();
    }
    else if(ui->rbnHigh->isChecked()){
        cabinType = "头等舱";
        price=flight.getPriceHigh();
    }
    else{
        QMessageBox::warning(this, "警告", "请选择舱位！");
        return;
    }

    QString foodName = ui->cbxFoodName->currentText().trimmed();

    QString confirmMessage = QString(
                                 "请确认订票信息：\n\n"
                                 "航班号：%1\n"
                                 "航线：%2 → %3\n"
                                 "起飞时间：%4\n"
                                 "到达时间：%5\n"
                                 "舱位：%6\n"
                                 "餐食：%7\n"
                                 "价格：¥%8\n\n"
                                 "确认订票吗？"
                                 ).arg(flight.getFlightName())
                                 .arg(flight.getStartCity())
                                 .arg(flight.getEndCity())
                                 .arg(flight.getStartTime().toString("yyyy-MM-dd hh:mm"))
                                 .arg(flight.getEndTime().toString("yyyy-MM-dd hh:mm"))
                                 .arg(cabinType)
                                 .arg(foodName)
                                 .arg(price, 0, 'f', 2);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认订票", confirmMessage,
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 创建订单对象
        Order newOrder;
        newOrder.setClientID(curUser->getUserID());
        newOrder.setFlightID(flight.getFlightID());
        newOrder.setCabintype(cabinType);
        newOrder.setSeatID(0);
        newOrder.setLuggageID(0);  // 默认无行李
        newOrder.setFoodID(currentFoodId); // 关联选中的餐食ID

        // 使用类成员的订单模型
        QString error;
        bool success = ordermodel->addOrder(newOrder, error);

        if (!success) {
            QMessageBox::warning(this, "订票失败", "订票失败：" + error);
        }else{
            // 获取新添加的订单ID
            std::vector<Order> orders = ordermodel->getOrdersByClientId(curUser->getUserID(), error);
            int latestOrderId = 0;
            for (const auto &ord : orders) {
                if (ord.getOrderId() > latestOrderId) {
                    latestOrderId = ord.getOrderId();
                }
            }

            QMessageBox::information(this, "订票成功",
                                     QString("订票成功！\n\n"
                                             "订单号：%1\n"
                                             "航班号：%2\n"
                                             "舱位：%3\n"
                                             "价格：¥%4\n\n"
                                             "您可以在个人页面的\"我的订单\"中查看和管理订单。")
                                         .arg(latestOrderId)
                                         .arg(flight.getFlightName())
                                         .arg(cabinType)
                                         .arg(price, 0, 'f', 2));
        }
    }
}

void FlightWidget::loadFoodsToComboBox(int flightId)
{
    ui->cbxFoodName->blockSignals(true);
    ui->cbxFoodName->clear();

    // 添加"无餐食"选项（默认）
    ui->cbxFoodName->addItem("无餐食", QVariant(0));

    // 从FoodModel查询当前航班的餐食
    QList<Food> foodList;
    QString err;
    FoodModel foodModel(this, flightmodel);
    foodModel.init();
    if (foodModel.getFoodListByFlightId(flightId, foodList, err)) {
        // 餐食添加到下拉框（text=餐食名称，data=餐食ID）
        for (const auto &food : foodList) {
            ui->cbxFoodName->addItem(food.getFoodName(), QVariant(food.getFoodId()));
        }
    } else {
        qDebug() << "加载餐食失败：" << err;
    }

    ui->cbxFoodName->setCurrentIndex(0); // 默认选中"无餐食"
    currentFoodId = 0; // 重置选中餐食ID
    ui->cbxFoodName->blockSignals(false);
}

void FlightWidget::on_cbxFoodName_currentIndexChanged(int index)
{
    if (index < 0) return;
    currentFoodId = ui->cbxFoodName->itemData(index).toInt();
    qDebug() << "当前选中餐食ID：" << currentFoodId;
}
