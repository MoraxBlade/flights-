#ifndef ORDERVIEW_H
#define ORDERVIEW_H

#include <QWidget>
#include <QTimer>
#include <QDataWidgetMapper>
#include "../model/ordermodel.h"
#include "../model/flightmodel.h"
#include "../entity/user.h"

namespace Ui {
class OrderView;
}

class OrderView : public QWidget
{
    Q_OBJECT

public:
    explicit OrderView(QWidget *parent = nullptr, User *currentUser = nullptr);
    ~OrderView();

private slots:
    // 刷新数据
    void refreshDatabase();

    // 表格行选择变化
    void onOrderSelectionChanged(const QModelIndex &current, const QModelIndex &previous);

    // 按钮点击事件
    void on_btnRefresh_clicked();

private:
    Ui::OrderView *ui;

    // 数据模型
    OrderModel *ordermodel;

    // 当前登录用户
    User *currentUser;

    // 代理模型，用于过滤
    QSortFilterProxyModel *orderproxymodel;

    // 刷新定时器
    QTimer *refreshTimer;
    const int timerefreshMs = 30000;  // 30秒刷新一次

    // 初始化方法
    void initUI();
    void initModel();

    // 加载订单详细信息
    void loadOrderDetails(const QModelIndex &index);

    // 清空详细信息
    void clearDetails();

    // 设置过滤条件，只显示当前用户的订单
    void filterByCurrentUser();
};

#endif // ORDERVIEW_H
