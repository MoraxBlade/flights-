#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QHash>
#include "../entity/order.h"
#include "flightmodel.h"

class OrderModel : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit OrderModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    bool init();
    bool addOrder(const Order &order, QString &error);
    bool deleteOrder(int orderId, QString &error);
    bool updateOrder(const Order &order, QString &error);

    Order getOrderById(int orderId, QString &error) const;
    std::vector<Order> getOrdersByClientId(int clientId, QString &error) const;
    std::vector<Order> getAllOrders(QString &error) const;

    Flight getFlightInfo(int flightId, QString &error) const;
    QString getFlightName(int flightId) const;
    QString getCompanyName(int flightId) const;
    QString getPlaneName(int flightId) const;
    double calculateTotalPrice(const Order &order) const;
    QString getFoodInfo(int orderId) const;

    // 列数：0..5 原生订单字段 + 6..13 扩展字段 = 14 列
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 重新加载数据时清除缓存
    bool select() override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 航班模型（由外部注入）
    mutable FlightModel *flightModel;

    bool addOrderWithFood(const Order &order, int foodId, QString &error);


private:
    Order orderFromRecord(const QSqlRecord &record) const;
    QString getSeatNo(int seatId) const;

    mutable QHash<int, double> luggagePriceCache;  // LuggageID -> 行李费
    mutable QHash<int, int> luggageWeightCache;    // LuggageID -> 行李重量

    // 添加行李信息获取函数
    double getLuggagePrice(int luggageId) const;
    int getLuggageWeight(int luggageId) const;
    void prefetchLuggages();

    // flight 缓存：flightId → Flight
    mutable QHash<int, Flight> flightCache;

    // plane 缓存：planeId → planeName
    mutable QHash<int, QString> planeCache;

    // 预取当前模型所需的航班信息（在 select() 后调用）
    void prefetchFlights();

};

#endif // ORDERMODEL_H


