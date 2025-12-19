#include "ordermodel.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

OrderModel::OrderModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db), flightModel(nullptr)
{
}

bool OrderModel::init()
{
    setTable("orders");
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    if (!select()) {
        qDebug() << "OrderModel init failed:" << lastError().text();
        return false;
    }

    return true;
}

// 强制列数为 1
int OrderModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 17;
}

QVariant OrderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QSqlTableModel::headerData(section, orientation, role);

    switch (section)
    {
    case 0: return "订单号";
    case 1: return "客户ID";
    case 2: return "航班号";
    case 3: return "座位号";
    case 4: return "行李重量";
    case 5: return "舱位";
    case 6: return "飞机";
    case 7: return "起飞时间";
    case 8: return "到达时间";
    case 9: return "飞行时长";
    case 10: return "起点";
    case 11: return "终点";
    case 12: return "飞机餐";
    case 13: return "票价";
    case 14: return "行李费";
    case 15: return "总价";
    case 16: return "天气提示";
    }
    return QVariant();
}

// select 时清缓存并预取本页需要的航班
bool OrderModel::select()
{
    // 先让基类刷新数据
    bool ok = QSqlTableModel::select();

    // 先清空缓存（保证数据一致）
    flightCache.clear();
    planeCache.clear();
    luggagePriceCache.clear();    // 新增：清空行李价格缓存
    luggageWeightCache.clear();

    if (!ok) return ok;

    // 预取当前表中所有订单对应的航班，避免后续滚动时触发大量查询
    prefetchFlights();
    prefetchLuggages();
    return true;
}
Qt::ItemFlags OrderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // 所有列都可选择可显示（但不可编辑）
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant OrderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QSqlTableModel::data(index, role);

    int row = index.row();
    int col = index.column();

    if (col < 6)
    {
        if (col == 3) // 座位号
        {
            int seatId = QSqlTableModel::data(this->index(row, 3)).toInt();
            if (seatId == 0) {
                return "未选座";
            } else {
                // 查询座位编号 (SeatNo)
                QString seatNo = getSeatNo(seatId);
                if (seatNo.isEmpty()) {
                    return QString("座位%1").arg(seatId);
                }
                return seatNo;
            }
        }
        if (col == 2) {
            int flightId = QSqlTableModel::data(this->index(row, 2)).toInt();

            // 先从缓存获取
            if (flightCache.contains(flightId)) {
                QString flightName = flightCache.value(flightId).getFlightName();
                if (!flightName.isEmpty()) {
                    return flightName;
                }
            }

            // 如果缓存中没有，从数据库查询
            QString error;
            Flight f = getFlightInfo(flightId, error);
            if (error.isEmpty() && !f.getFlightName().isEmpty()) {
                return f.getFlightName();
            } else {
                return QString("航班%1").arg(flightId);
            }
        }
        if(col == 4) { // 行李ID
            int luggageId = QSqlTableModel::data(this->index(row, 4)).toInt();
            if (luggageId == 0) {
                return "无托运";
            } else {
                // 获取行李重量
                int weight = getLuggageWeight(luggageId);
                if (weight > 0) {
                    return QString("%1kg").arg(weight);
                } else {
                    return "未知";
                }
            }
        }
        if (col == 5) // 舱位
        {
            int t = QSqlTableModel::data(this->index(row, 5)).toInt();
            if (t == 1) return "经济舱";
            if (t == 2) return "商务舱";
            if (t == 3) return "头等舱";
            return "未知舱位";
        }
        return QSqlTableModel::data(index, role);
    }

    // 扩展列读取 flight 信息
    int flightId = QSqlTableModel::data(this->index(row, 2)).toInt();
    QString error;
    Flight f;

    if (flightCache.contains(flightId))
        f = flightCache.value(flightId);
    else
    {
        f = getFlightInfo(flightId, error);
        flightCache.insert(flightId, f);
    }

    int t = QSqlTableModel::data(this->index(row, 5)).toInt();
    int luggageId = QSqlTableModel::data(this->index(row, 4)).toInt();

    // 计算基础票价
    double basePrice = (t == 1 ? f.getPriceLow() :
                            t == 2 ? f.getPriceMid() :
                            f.getPriceHigh());

    // 获取行李费
    double luggagePrice = 0.0;
    if (luggageId > 0) {
        luggagePrice = getLuggagePrice(luggageId);
    }

    // 计算总价
    double totalPrice = basePrice + luggagePrice;

    switch (col)
    {
    case 6: // 飞机型号
        return getPlaneName(flightId);

    case 7: // 起飞时间
        return f.getStartTime().toString("yyyy-MM-dd HH:mm");

    case 8: // 到达时间
        return f.getEndTime().toString("yyyy-MM-dd HH:mm");

    case 9: // 时长
        return QString("%1小时").arg(QString::number(f.getDuration(), 'f', 1));

    case 10: return f.getStartCity();
    case 11: return f.getEndCity();

    case 12:// 飞机餐
    {
        int orderId = QSqlTableModel::data(this->index(row, 0)).toInt();
        return getFoodInfo(orderId);
    }

    case 13: // 票价
        return QString("¥%1").arg(QString::number(basePrice, 'f', 2));

    case 14: // 行李费
        if (luggageId > 0) {
            return QString("¥%1").arg(QString::number(luggagePrice, 'f', 2));
        } else {
            return "¥0.00";
        }

    case 15: // 总价
        return QString("¥%1").arg(QString::number(totalPrice, 'f', 2));

    case 16: // 天气提示
        return f.getMessage();

    default:
        return QSqlTableModel::data(index, role);
    }
}


bool OrderModel::addOrder(const Order &order, QString &error)
{
    QSqlQuery query(database());
    int foodId=order.getFoodID();
    // 新增：判断餐食ID是否为空
    bool seatIsNull = (order.getSeatID() == 0);
    bool luggageIsNull = (order.getLuggageID() == 0);
    bool foodIsNull = (foodId == 0); // 新增餐食空值判断

    // 完全沿用原有分支逻辑，仅新增FoodID字段
    if (seatIsNull && luggageIsNull && foodIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, NULL, NULL, :cabin, NULL)"
            );
    } else if (seatIsNull && luggageIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, NULL, NULL, :cabin, :food)"
            );
        query.bindValue(":food", foodId); // 绑定餐食ID
    } else if (seatIsNull && foodIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, NULL, :luggage, :cabin, NULL)"
            );
        query.bindValue(":luggage", order.getLuggageID());
    } else if (luggageIsNull && foodIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, :seat, NULL, :cabin, NULL)"
            );
        query.bindValue(":seat", order.getSeatID());
    } else if (seatIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, NULL, :luggage, :cabin, :food)"
            );
        query.bindValue(":luggage", order.getLuggageID());
        query.bindValue(":food", foodId); // 绑定餐食ID
    } else if (luggageIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, :seat, NULL, :cabin, :food)"
            );
        query.bindValue(":seat", order.getSeatID());
        query.bindValue(":food", foodId); // 绑定餐食ID
    } else if (foodIsNull) {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, :seat, :luggage, :cabin, NULL)"
            );
        query.bindValue(":seat", order.getSeatID());
        query.bindValue(":luggage", order.getLuggageID());
    } else {
        query.prepare(
            "INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID) "
            "VALUES (:client, :flight, :seat, :luggage, :cabin, :food)"
            );
        query.bindValue(":seat", order.getSeatID());
        query.bindValue(":luggage", order.getLuggageID());
        query.bindValue(":food", foodId); // 绑定餐食ID
    }

    // 绑定公共参数（与原有逻辑一致）
    query.bindValue(":client", order.getClientID());
    query.bindValue(":flight", order.getFlightID());

    int cabinType = 1;
    if (order.getCabintype() == "商务舱") cabinType = 2;
    else if (order.getCabintype() == "头等舱") cabinType = 3;
    query.bindValue(":cabin", cabinType);

    // 执行并输出详细日志（便于排查）
    if (!query.exec()) {
        error = query.lastError().text();
        qDebug() << "插入订单（含餐食）失败:" << error;
        qDebug() << "最终执行SQL:" << query.lastQuery();
        qDebug() << "参数明细: "
                 << "client=" << order.getClientID()
                 << " flight=" << order.getFlightID()
                 << " seat=" << order.getSeatID()
                 << " luggage=" << order.getLuggageID()
                 << " cabin=" << cabinType
                 << " foodId=" << foodId; // 新增餐食ID日志
        return false;
    }
    // 刷新模型（与原有逻辑一致）
    QSqlQuery updateSeatQuery(database());
    updateSeatQuery.prepare("UPDATE flights SET SeatCnt = SeatCnt - 1 WHERE FlightID = :flightID");
    updateSeatQuery.bindValue(":flightID", order.getFlightID());

    if (!updateSeatQuery.exec()) {
        error = "更新航班座位数失败: " + updateSeatQuery.lastError().text();
        qDebug() << "更新座位数失败:" << error;
        return false;
    }

    // 手动刷新模型
    if (!select()) {
        error = "刷新订单数据失败: " + lastError().text();
        return false;
    }
    flightCache.clear();
    planeCache.clear();
    luggagePriceCache.clear();
    luggageWeightCache.clear();

    return true;
}


bool OrderModel::deleteOrder(int orderId, QString &error)
{
    QSqlDatabase db = database();
    db.transaction();

    try {
        QSqlQuery query(db);

        // 1. 获取订单信息
        query.prepare(
            "SELECT o.FlightID, o.SeatID, o.LuggageID, "
            "(SELECT COUNT(*) FROM orders o2 WHERE o2.SeatID = o.SeatID) as seat_usage_count "
            "FROM orders o "
            "WHERE o.OrderID = :orderID"
            );
        query.bindValue(":orderID", orderId);

        if (!query.exec()) {
            error = "查询订单失败: " + query.lastError().text();
            db.rollback();
            return false;
        }

        if (!query.next()) {
            error = "未找到订单";
            db.rollback();
            return false;
        }

        int flightId = query.value(0).toInt();
        int seatId = query.value(1).toInt();
        int luggageId = query.value(2).toInt();
        int seatUsageCount = query.value(3).toInt();

        qDebug() << "=== 删除订单调试 ===";
        qDebug() << "订单号:" << orderId;
        qDebug() << "航班号:" << flightId;
        qDebug() << "座位号:" << seatId;
        qDebug() << "行李号:" << luggageId;
        qDebug() << "此座位被" << seatUsageCount << "个订单使用";

        // 2. 先删除订单本身
        query.prepare("DELETE FROM orders WHERE OrderID = :orderID");
        query.bindValue(":orderID", orderId);

        if (!query.exec()) {
            error = "删除订单失败: " + query.lastError().text();
            db.rollback();
            return false;
        }

        int ordersDeleted = query.numRowsAffected();
        qDebug() << "删除了" << ordersDeleted << "个订单记录";

        // 3. 处理行李
        if (luggageId > 0) {
            // 先更新当前订单的行李ID为NULL
            query.prepare("UPDATE orders SET LuggageID = NULL WHERE LuggageID = :luggageID");
            query.bindValue(":luggageID", luggageId);
            query.exec();

            // 再检查是否还有订单使用这个行李
            query.prepare("SELECT COUNT(*) FROM orders WHERE LuggageID = :luggageID");
            query.bindValue(":luggageID", luggageId);

            if (query.exec() && query.next()) {
                int remainingUses = query.value(0).toInt();
                qDebug() << "行李" << luggageId << "还被" << remainingUses << "个订单使用";

                if (remainingUses == 0) {
                    // 没有订单使用这个行李了，可以删除
                    query.prepare("DELETE FROM luggages WHERE LuggageID = :luggageID");
                    query.bindValue(":luggageID", luggageId);

                    if (query.exec()) {
                        qDebug() << "删除了行李记录:" << luggageId;
                    }
                }
            }
        }

        // 4. 处理座位 - 关键修改！
        if (seatId > 0) {
            qDebug() << "处理座位" << seatId << "，原使用次数:" << seatUsageCount;

            if (seatUsageCount == 1) {
                // 只有当前订单使用这个座位，标记为可用
                query.prepare("UPDATE seats SET SeatAvailable = 1 WHERE SeatID = :seatID");
                query.bindValue(":seatID", seatId);

                if (query.exec()) {
                    qDebug() << "座位" << seatId << "标记为可用";
                } else {
                    qDebug() << "更新座位状态失败:" << query.lastError().text();
                }

                // 注意：不要删除座位记录！
            } else {
                // 多个订单使用同一个座位，这是不正常的
                qDebug() << "警告：座位" << seatId << "被多个订单使用，这是错误的数据状态";

                // 只标记座位为可用，不删除
                query.prepare("UPDATE seats SET SeatAvailable = 1 WHERE SeatID = :seatID");
                query.bindValue(":seatID", seatId);
                query.exec();
            }

            // 绝对不要执行 DELETE FROM seats！
        }

        // 5. 更新航班座位数
        query.prepare("UPDATE flights SET SeatCnt = SeatCnt + 1 WHERE FlightID = :flightID");
        query.bindValue(":flightID", flightId);

        if (!query.exec()) {
            error = "更新航班座位数失败: " + query.lastError().text();
            db.rollback();
            return false;
        }

        qDebug() << "更新了航班" << flightId << "的座位数";

        // 6. 提交事务
        if (!db.commit()) {
            error = "提交事务失败: " + db.lastError().text();
            return false;
        }

        // 7. 检查结果
        query.prepare("SELECT COUNT(*) FROM orders WHERE FlightID = :flightID");
        query.bindValue(":flightID", flightId);

        if (query.exec() && query.next()) {
            int remainingOrders = query.value(0).toInt();
            qDebug() << "删除后，航班" << flightId << "还有" << remainingOrders << "个订单";
        }

        qDebug() << "=== 删除完成 ===";
        select();
        return true;

    } catch (...) {
        db.rollback();
        error = "取消订单时发生异常";
        return false;
    }
}

bool OrderModel::updateOrder(const Order &order, QString &error)
{
    setFilter(QString("OrderID=%1").arg(order.getOrderId()));
    select();

    if (rowCount() == 0)
    {
        error = "订单不存在";
        return false;
    }

    QSqlRecord rec = record(0);
    rec.setValue("ClientID", order.getClientID());
    rec.setValue("FlightID", order.getFlightID());
    rec.setValue("SeatID", order.getSeatID());
    rec.setValue("LuggageID", order.getLuggageID());

    int type = 1;
    if (order.getCabintype() == "商务舱") type = 2;
    else if (order.getCabintype() == "头等舱") type = 3;
    rec.setValue("Cabintype", type);

    if (!setRecord(0, rec) || !submitAll())
    {
        error = lastError().text();
        return false;
    }

    setFilter("");
    select();
    flightCache.clear();
    planeCache.clear();
    luggagePriceCache.clear();
    luggageWeightCache.clear();

    return true;
}

// ------------------ 航班查询 ------------------

Flight OrderModel::getFlightInfo(int flightId, QString &error) const
{
    QSqlQuery q(database());
    q.prepare("SELECT * FROM flights WHERE FlightID = :id");
    q.bindValue(":id", flightId);

    if (!q.exec() || !q.next())
    {
        error = "航班不存在";
        return Flight();
    }

    Flight f;
    QSqlRecord r = q.record();
    f.setFlightID(r.value("FlightID").toInt());
    f.setSeatCnt(r.value("SeatCnt").toInt());
    f.setPlaneID(r.value("PlaneID").toInt());
    f.setCompanyID(r.value("CompanyID").toInt());
    f.setStartCity(r.value("StartCity").toString());
    f.setEndCity(r.value("EndCity").toString());
    f.setStartTime(r.value("StartTime").toDateTime());
    f.setEndTime(r.value("EndTime").toDateTime());
    f.setDuration(r.value("Duration").toDouble());
    f.setPrice(
        r.value("PriceLow").toDouble(),
        r.value("PriceMid").toDouble(),
        r.value("PriceHigh").toDouble());
    f.setMessage(r.value("Message").toString());
    f.setFlightName();  // 这里需要正确设置 FlightName
    return f;
}

QString OrderModel::getPlaneName(int flightId) const
{
    // 先看 flightCache 是否有对应 Flight（通常 prefetch 已经填好）
    Flight f;
    if (flightCache.contains(flightId)) {
        f = flightCache.value(flightId);
    } else {
        QString ferr;
        f = getFlightInfo(flightId, ferr);
        // 把它放到缓存，避免重复查询
        flightCache.insert(flightId, f);
    }

    int planeId = f.getPlaneID();
    if (planeId <= 0) return QStringLiteral("未知机型");

    // 查 planeName 缓存
    if (planeCache.contains(planeId)) {
        return planeCache.value(planeId);
    }

    QSqlQuery q(database());
    q.prepare("SELECT PlaneName FROM planes WHERE PlaneID = :id");
    q.bindValue(":id", planeId);
    if (q.exec() && q.next()) {
        QString name = q.value(0).toString();
        planeCache.insert(planeId, name);
        return name;
    }

    planeCache.insert(planeId, QStringLiteral("未知机型"));
    return QStringLiteral("未知机型");
}


QString OrderModel::getFlightName(int flightId) const
{
    if (flightCache.contains(flightId)) {
        // 如果 Flight 对象中包含航班名称字段，优先返回
        const Flight &f = flightCache.value(flightId);
        QString fn = f.getFlightName();
        if (!fn.isEmpty()) return fn;
        // 否则按需返回组合或"航班#id"
    }

    QString err;
    Flight f = getFlightInfo(flightId, err);
    if (!err.isEmpty()) return QStringLiteral("未知航班");
    // 如果 flight.getFlightName() 有值就返回
    if (!f.getFlightName().isEmpty()) return f.getFlightName();
    // fallback 显示 FlightID
    return QStringLiteral("航班%1").arg(f.getFlightID());
}

QString OrderModel::getCompanyName(int flightId) const
{
    return "默认航司";
}

double OrderModel::calculateTotalPrice(const Order &order) const
{
    QString err;
    Flight f = getFlightInfo(order.getFlightID(), err);
    if (!err.isEmpty()) return 0;

    if (order.getCabintype() == "经济舱") return f.getPriceLow();
    if (order.getCabintype() == "商务舱") return f.getPriceMid();
    return f.getPriceHigh();
}

QString OrderModel::getFoodInfo(int orderId) const
{
    QSqlQuery query(database());
    query.prepare(
        "SELECT f.FoodName "
        "FROM orders o "
        "LEFT JOIN foods f ON o.FoodID = f.FoodID "
        "WHERE o.OrderID = :orderId"
        );
    query.bindValue(":orderId", orderId);

    if (query.exec() && query.next()) {
        QString foodName = query.value(0).toString();
        if (foodName.isEmpty()) {
            return "无餐食";
        }
        return foodName;
    }

    return "无餐食";
}

std::vector<Order> OrderModel::getOrdersByClientId(int clientId, QString &error) const
{
    std::vector<Order> orders;
    QSqlQuery q(database());

    q.prepare("SELECT * FROM orders WHERE ClientID = :cid");
    q.bindValue(":cid", clientId);

    if (!q.exec()) {
        error = q.lastError().text();
        return orders;
    }

    while (q.next()) {
        QSqlRecord rec = q.record();
        Order order;

        order.setOrderId(rec.value("OrderID").toInt());
        order.setClientID(rec.value("ClientID").toInt());
        order.setFlightID(rec.value("FlightID").toInt());
        order.setSeatID(rec.value("SeatID").toInt());
        order.setLuggageID(rec.value("LuggageID").toInt());

        int t = rec.value("Cabintype").toInt();
        if (t == 1) order.setCabintype("经济舱");
        else if (t == 2) order.setCabintype("商务舱");
        else if (t == 3) order.setCabintype("头等舱");
        else order.setCabintype("未知舱位");

        // 航班信息
        QString ferr;
        Flight f = getFlightInfo(order.getFlightID(), ferr);
        if (ferr.isEmpty())
            order.setFlight(f);

        if (order.getLuggageID() > 0) {
            int weight = getLuggageWeight(order.getLuggageID());
            double price = getLuggagePrice(order.getLuggageID());
        }
        orders.push_back(order);
    }

    error.clear();
    return orders;
}

void OrderModel::prefetchFlights()
{
    QSet<int> need; // 要查询的 flightId 集合
    int rows = QSqlTableModel::rowCount();
    for (int r = 0; r < rows; ++r) {
        bool ok;
        int fid = QSqlTableModel::data(this->index(r, 2), Qt::DisplayRole).toInt(&ok);
        if (ok && fid > 0) need.insert(fid);
    }

    // 批量查询每个 flightId，一次写入缓存
    QSqlQuery q(database());
    q.prepare("SELECT * FROM flights WHERE FlightID = :id");
    for (int fid : need) {
        q.bindValue(":id", fid);
        if (!q.exec()) {
            q.clear();
            continue;
        }
        if (!q.next()) { q.clear(); continue; }
        QSqlRecord r = q.record();
        Flight f;
        f.setFlightID(r.value("FlightID").toInt());
        f.setSeatCnt(r.value("SeatCnt").toInt());
        f.setPlaneID(r.value("PlaneID").toInt());
        f.setCompanyID(r.value("CompanyID").toInt());
        f.setStartCity(r.value("StartCity").toString());
        f.setEndCity(r.value("EndCity").toString());
        f.setStartTime(r.value("StartTime").toDateTime());
        f.setEndTime(r.value("EndTime").toDateTime());
        f.setDuration(r.value("Duration").toDouble());
        f.setPrice(r.value("PriceLow").toDouble(),
                   r.value("PriceMid").toDouble(),
                   r.value("PriceHigh").toDouble());
        f.setMessage(r.value("Message").toString());
        f.setFlightName();

        flightCache.insert(fid, f);
        q.clear();
    }
}

bool OrderModel::addOrderWithFood(const Order &order, int foodId, QString &error)
{
    QSqlQuery query(database());
    const QString sql = R"(
        INSERT INTO orders (ClientID, FlightID, SeatID, LuggageID, Cabintype, FoodID)
        VALUES (:client, :flight, :seat, :luggage, :cabin, :food)
    )";

    query.prepare(sql);
    // 绑定所有参数（空值直接传0，数据库会自动存为NULL）
    query.bindValue(":client", order.getClientID());
    query.bindValue(":flight", order.getFlightID());
    query.bindValue(":seat", order.getSeatID());
    query.bindValue(":luggage", order.getLuggageID());
    // 舱位类型转换（保持原有逻辑）
    int cabinType = 1;
    if (order.getCabintype() == "商务舱") cabinType = 2;
    else if (order.getCabintype() == "头等舱") cabinType = 3;
    query.bindValue(":cabin", cabinType);
    // 绑定餐食ID（0对应数据库NULL）
    query.bindValue(":food", (foodId == 0) ? QVariant(QVariant::Int) : QVariant(foodId));

    // 执行并检查错误
    if (!query.exec()) {
        error = "插入订单失败: " + query.lastError().text();
        qDebug() << error;
        return false;
    }

    // 刷新模型（保持原有逻辑）
    if (!select()) {
        error = "刷新订单数据失败: " + lastError().text();
        return false;
    }
    flightCache.clear();
    planeCache.clear();
    return true;
}

double OrderModel::getLuggagePrice(int luggageId) const
{
    if (luggageId <= 0) return 0.0;

    // 先检查缓存
    if (luggagePriceCache.contains(luggageId)) {
        return luggagePriceCache.value(luggageId);
    }

    // 从数据库查询
    QSqlQuery query(database());
    QString sql = QString("SELECT Price FROM luggages WHERE LuggageID = %1").arg(luggageId);

    if (query.exec(sql) && query.next()) {
        double price = query.value(0).toDouble();
        luggagePriceCache.insert(luggageId, price);
        return price;
    }

    luggagePriceCache.insert(luggageId, 0.0);
    return 0.0;
}

int OrderModel::getLuggageWeight(int luggageId) const
{
    if (luggageId <= 0) return 0;

    // 先检查缓存
    if (luggageWeightCache.contains(luggageId)) {
        return luggageWeightCache.value(luggageId);
    }

    // 从数据库查询
    QSqlQuery query(database());
    QString sql = QString("SELECT Weight FROM luggages WHERE LuggageID = %1").arg(luggageId);

    if (query.exec(sql) && query.next()) {
        int weight = query.value(0).toInt();
        luggageWeightCache.insert(luggageId, weight);
        return weight;
    }

    luggageWeightCache.insert(luggageId, 0);
    return 0;
}

void OrderModel::prefetchLuggages()
{
    QSet<int> luggageIds;
    int rows = QSqlTableModel::rowCount();

    // 收集所有行李ID
    for (int r = 0; r < rows; ++r) {
        int luggageId = QSqlTableModel::data(this->index(r, 4)).toInt();
        if (luggageId > 0) {
            luggageIds.insert(luggageId);
        }
    }

    if (luggageIds.isEmpty()) return;

    // 批量查询行李信息
    QSqlQuery query(database());
    QString idList;
    for (int id : luggageIds) {
        if (!idList.isEmpty()) idList += ",";
        idList += QString::number(id);
    }

    QString sql = QString("SELECT LuggageID, Weight, Price FROM luggages WHERE LuggageID IN (%1)").arg(idList);

    if (query.exec(sql)) {
        while (query.next()) {
            int id = query.value(0).toInt();
            int weight = query.value(1).toInt();
            double price = query.value(2).toDouble();

            luggageWeightCache.insert(id, weight);
            luggagePriceCache.insert(id, price);
        }
    }
}

QString OrderModel::getSeatNo(int seatId) const
{
    if (seatId <= 0) return QString();

    // 这里可以添加缓存机制来提高性能
    static QHash<int, QString> seatNoCache;

    if (seatNoCache.contains(seatId)) {
        return seatNoCache.value(seatId);
    }

    QSqlQuery query(database());
    query.prepare("SELECT SeatNo FROM seats WHERE SeatID = :seatId");
    query.bindValue(":seatId", seatId);

    if (query.exec() && query.next()) {
        QString seatNo = query.value(0).toString();
        seatNoCache.insert(seatId, seatNo);
        return seatNo;
    }

    seatNoCache.insert(seatId, QString());
    return QString();
}


