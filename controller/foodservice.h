#ifndef FOODSERVICE_H
#define FOODSERVICE_H
#include <QObject.h>
#include <QList>
#include "../entity/food.h"
#include "../model/flightmodel.h"

class FoodService : public QObject//控制管理员和旅客的共有逻辑
{
    Q_OBJECT
public:
    explicit FoodService(QObject *parent = nullptr, FlightModel *flightModel = nullptr); // 直接依赖FlightModel，无需ControllerFlight

    bool queryFoodByFlightName(const QString& flightName, QList<Food>& foodList, QString& err);//根据航班号查询餐食列表（游客/管理员都需）

    bool checkFlightExists(const QString& flightName, Flight& flight, QString& err);//校验航班是否存在（增加/查询餐食都需）

private:
    FlightModel *m_flightModel;
};
#endif // FOODSERVICE_H
