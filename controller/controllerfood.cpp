#include "controllerfood.h"
#include "../entity/user.h"
#include "../entity/admin.h"
#include <QDateTime>
#include "../tool/recordtool.h"
ControllerFood::ControllerFood(QObject *parent, FoodModel *foodModel, FlightModel *flightModel)
    : QObject(parent)
    , m_foodModel(foodModel)
    , m_flightModel(flightModel)
{
    m_foodService = new FoodService(this, m_flightModel);
}

ControllerFood::~ControllerFood()
{
    delete m_foodService;
}

void ControllerFood::handleAddFood(const QString& flightName, const QString& foodName)// 处理增加餐食业务
{
    QString err;

    if (!checkAdminFlightPermission(flightName, err)) {// 校验是否有权限
        emit signalOperationResult(err, false);
        return;
    }

    Flight flight;
    if (!m_foodService->checkFlightExists(flightName, flight, err)) {// 校验航班是否存在
        emit signalOperationResult(err, false);
        return;
    }

    if (!checkFoodDuplicate(flightName, foodName, err)) {//校验餐食是否重复
        emit signalOperationResult(err, false);
        return;
    }

    Food food;
    food.setFlightId(flight.getFlightID());
    food.setFoodName(foodName);

    if (m_foodModel->insertFood(food, err)) {
        emit signalOperationResult(QString("成功添加餐食「%1」").arg(foodName), true);
        // 记录日志
        int newFoodId = m_foodModel->getLastInsertedFoodId(err);
        extern User* curUser;
        Admin* admin = dynamic_cast<Admin*>(curUser);
        RecordTool::logAdminOperation(
            admin,
            OpType::ADD,
            TargetType::FOOD,
            newFoodId,  // 这里用获取到的真实自增ID
            QString("%4号管理员为航班「%1」添加餐食「%2」（餐食ID：%3）")
                .arg(flightName, foodName).arg(newFoodId).arg(admin->getAdminID())
            );
        //刷新界面
        QList<Food> updatedFoodList;
        m_foodService->queryFoodByFlightName(flightName, updatedFoodList, err);
        emit signalFoodListUpdated(updatedFoodList);
    } else {
        emit signalOperationResult(err, false);
    }
}

void ControllerFood::handleDeleteFood(const QString& flightName, const QString& foodName)// 处理删除餐食业务
{
    QString err;
    if (!checkAdminFlightPermission(flightName, err)) {// 校验是否有权限
        emit signalOperationResult(err, false);
        return;
    }

    int foodId;
    if (!m_foodModel->getFoodIdByFlightNameAndFoodName(flightName, foodName, foodId, err)) {// 获取餐食ID（通过航班号+餐食名唯一确定）
        emit signalOperationResult(err, false);
        return;
    }
    int curFoodId=foodId;

    if (m_foodModel->deleteFood(foodId, err)) {
        emit signalOperationResult(QString("成功删除餐食「%1」").arg(foodName), true);

        // 记录日志
        extern User* curUser;
        Admin* admin = dynamic_cast<Admin*>(curUser);

        RecordTool::logAdminOperation(
            admin,
            OpType::DELETE,
            TargetType::FOOD,
            curFoodId,
            QString("%4号管理员为航班「%1」删除餐食「%2」（餐食ID：%3）")
                .arg(flightName, foodName).arg(foodId).arg(admin->getAdminID())
            );
        //刷新界面
        QList<Food> updatedFoodList;
        m_foodService->queryFoodByFlightName(flightName, updatedFoodList, err);
        emit signalFoodListUpdated(updatedFoodList);
    } else {
        emit signalOperationResult(err, false);
    }
}

void ControllerFood::handleQueryFood(const QString& flightName)// 查询餐食（并更新相关UI）
{
    QString err;
    QList<Food> foodList;

    if (!checkAdminFlightPermission(flightName, err)) {// 校验是否有权限
        emit signalOperationResult(err, false);
        return;
    }

    if (m_foodService->queryFoodByFlightName(flightName, foodList, err)) {
        emit signalFoodListUpdated(foodList);
        if (foodList.isEmpty() && !flightName.isEmpty()) {
            emit signalOperationResult("该航班暂无餐食", true);
        }
    } else {
        emit signalOperationResult(err, false);
        emit signalFoodListUpdated(QList<Food>()); // 清空表格
    }
}
bool ControllerFood::checkFoodDuplicate(const QString& flightName, const QString& foodName, QString& err)
{
    int foodId;
    if (m_foodModel->getFoodIdByFlightNameAndFoodName(flightName, foodName, foodId, err)) {
        err = QString("航班「%1」已存在餐食「%2」，不可重复添加").arg(flightName, foodName);
        return false;
    }
    return true;
}

bool ControllerFood::checkAdminFlightPermission(const QString& flightName, QString& err)
{
    extern User* curUser;
    if (!curUser || curUser->getUserType() != "admin") {
        err = "仅管理员可管理餐食";
        return false;
    }

    Admin* admin = dynamic_cast<Admin*>(curUser);
    int adminCompanyId = admin->getCompanyID();
    if (adminCompanyId <= 0) {
        err = "管理员所属公司信息无效";
        return false;
    }
    Flight flight;
    if (!m_foodService->checkFlightExists(flightName, flight, err)) {
        return false;
    }
    int flightCompanyId=flight.getCompanyID();
    if (flightCompanyId != adminCompanyId) {
        err = "无权限操作其他公司的航班餐食";
        return false;
    }
    return true;
}
