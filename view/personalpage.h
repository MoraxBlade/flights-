#ifndef PERSONALPAGE_H
#define PERSONALPAGE_H
#include "seatitem.h"
#include <QWidget>
#include <QLabel>
#include "../entity/user.h"
#include "../model/ordermodel.h"
#include <QSortFilterProxyModel>  // 添加这个头文件

namespace Ui {
class personalpage;
}

class personalpage : public QWidget
{
    Q_OBJECT

public:
    int selectedOrderId = -1;
    int flightSeatCount = 0;
    explicit personalpage(QWidget *parent = nullptr);
    ~personalpage();
    QVector<SeatItem*> seat;
    // 操作提示
    void showNotify(const QString &text);
    void updateUserStatus();
    void focusedIdChanged(int id);

    void initSeatSelection(int seatCount, const QSet<int>& occupiedSeats);
    void addLegend(int colCount, int rowCount);

private slots:
    void on_pushButton_clicked();
    void refreshOrders();  // 刷新订单

    void on_deleteodr_clicked();

    void on_comboBox_activated(int index);

    void refreshComboBox();
    void showDefaultMessage(const QString& message);



    void on_spinBox_textChanged(const QString &arg1);

    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

    void on_pushButton_2_clicked();

private:
    Ui::personalpage *ui;
    QLabel *Usr;
    int numSeats=20;
    int focusedID=-1;
    int luggageType;
    double currentLuggageWeight;
    double currentLuggagePrice;
    double currentBasePrice;

    // 订单相关成员变量
    OrderModel *ordermodel;
    QSortFilterProxyModel *orderProxyModel;  // 用于过滤和排序
    void select_luggage();
    // 初始化订单表格
    void initOrderView();
    // 加载当前用户的订单
    void loadUserOrders();

    // 如果需要在点击时显示详情，添加这个槽函数
    void onOrderClicked(const QModelIndex &index);

    void loadUserOrdersToComboBox();
    void updateSelection();

};

#endif // PERSONALPAGE_H
