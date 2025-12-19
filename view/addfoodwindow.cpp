// addfoodwindow.cpp 头部添加必要头文件
#include "../view/addfoodwindow.h"
#include "ui_addfoodwindow.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QHeaderView>

AddFoodWindow::AddFoodWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddFoodWindow)
    , m_foodTableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // 初始化表格：设置列名、禁止编辑、列宽自适应
    m_foodTableModel->setColumnCount(3);
    m_foodTableModel->setHorizontalHeaderLabels({"FoodID", "FlightID", "餐食名称"});
    ui->foodList->setModel(m_foodTableModel);
    ui->foodList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->foodList->setColumnHidden(0, true);
    ui->foodList->setColumnHidden(1, true);
    ui->foodList->horizontalHeader()->setStretchLastSection(true);
    ui->foodList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

AddFoodWindow::~AddFoodWindow()
{
    delete ui;
}

void AddFoodWindow::on_addFood_clicked()
{
    QString flightName = ui->txtFlightID->text().trimmed();
    QString foodName = ui->txtAddFood->text().trimmed();
    // 基础输入校验
    if (flightName.isEmpty()) {
        slotShowResult("航班号不能为空！", false);
        return;
    }
    if (foodName.isEmpty()) {
        slotShowResult("餐食名称不能为空！", false);
        return;
    }

    emit signalAddFood(flightName, foodName);// 发送信号给Controller处理业务
}

void AddFoodWindow::on_deleteFood_clicked()
{
    QString flightName = ui->txtFlightID->text().trimmed();
    QString foodName = ui->txtAddFood->text().trimmed();
    if (flightName.isEmpty() || foodName.isEmpty()) {
        slotShowResult("航班号和餐食名称均不能为空！", false);
        return;
    }

    emit signalDeleteFood(flightName, foodName);// 发送信号给Controller处理业务
}



void AddFoodWindow::slotUpdateFoodList(const QList<Food>& foodList)// 接收Controller的餐食列表，更新表格
{
    m_foodTableModel->removeRows(
        0,
        m_foodTableModel->rowCount()
        );
    for (const Food& food : foodList) {
        QList<QStandardItem*> row;
        row << new QStandardItem(food.getFoodId());
        row << new QStandardItem(food.getFlightId());
        row << new QStandardItem(food.getFoodName().trimmed());
        m_foodTableModel->appendRow(row);
    }
}

void AddFoodWindow::slotShowResult(const QString &msg, bool isSuccess)// 显示操作结果提示
{
    if (isSuccess) {
        QMessageBox::information(this, "操作成功", msg);
        clearInputs();
    } else {
        QMessageBox::warning(this, "操作失败", msg);
    }
}

void AddFoodWindow::clearInputs()// 清空输入框
{
    ui->txtAddFood->clear();
}
void AddFoodWindow::on_txtFlightID_returnPressed()
{
    QString flightName = ui->txtFlightID->text().trimmed();
    emit signalQueryFood(flightName);
}
