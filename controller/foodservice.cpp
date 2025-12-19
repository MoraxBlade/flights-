#include "foodservice.h"
#include "../model/foodmodel.h"
FoodService::FoodService(QObject *parent, FlightModel *flightModel)
    : QObject(parent)
    , m_flightModel(flightModel)
{}

bool FoodService::queryFoodByFlightName(const QString& flightName, QList<Food>& foodList, QString& err)//根据航班号查询餐食列表
{
    foodList.clear();
    if (flightName.isEmpty()) {
        return true;
    }

    Flight flight;
    if (!checkFlightExists(flightName, flight, err)) {
        return false;
    }

    FoodModel foodModel(nullptr, m_flightModel);
    foodModel.init();
    if (!foodModel.loadFoodsByFlightName(flightName, err)) {
        return false;
    }
    for (int i = 0; i < foodModel.rowCount(); ++i) {
        int foodID = foodModel.data(foodModel.index(i, 0)).toInt();
        QString foodName = foodModel.data(foodModel.index(i, 2)).toString().trimmed();
        Food food;
        food.setFoodId(foodID);
        food.setFlightId(flight.getFlightID());
        food.setFoodName(foodName);
        foodList.append(food);
    }
    return true;
}

bool FoodService::checkFlightExists(const QString& flightName, Flight& flight, QString& err)//校验航班是否存在
{
    if (!m_flightModel) {
        err = "航班模型未初始化";
        return 0;
    }
    return m_flightModel->getFlightByFlightName(flightName, flight, err);
}
