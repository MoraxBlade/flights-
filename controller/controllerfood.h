#ifndef CONTROLLERFOOD_H
#define CONTROLLERFOOD_H
#include <QObject>
#include <QList>
#include "../model/foodmodel.h"
#include "../model/flightmodel.h"
#include "../controller/foodservice.h"
#include "../entity/food.h"

// 业务控制器类：处理餐食相关业务逻辑，与UI解耦
class ControllerFood : public QObject
{
    Q_OBJECT

public:
    explicit ControllerFood(QObject *parent = nullptr, FoodModel *foodModel = nullptr, FlightModel *flightModel = nullptr);
    ~ControllerFood();

public slots:
    // 处理「增加餐食」请求（接收UI信号）
    void handleAddFood(const QString& flightName, const QString& foodName);
    // 处理「删除餐食」请求（接收UI信号）
    void handleDeleteFood(const QString& flightName, const QString& foodName);
    // 处理「查询餐食」请求（接收UI信号）
    void handleQueryFood(const QString& flightName);

signals:
    // 向UI发送餐食列表数据
    void signalFoodListUpdated(const QList<Food>& foodList);
    // 向UI发送操作结果
    void signalOperationResult(const QString& msg, bool isSuccess);

private:
    FoodModel *m_foodModel;       // 餐食数据模型（操作数据库）
    FlightModel *m_flightModel;   // 航班数据模型（依赖航班查询）
    FoodService *m_foodService;   // 公共业务服务（复用查询/校验逻辑）

    // 校验同一航班下是否存在重复餐食
    bool checkFoodDuplicate(const QString& flightName, const QString& foodName, QString& err);
    //校验当前管理员是否有修改该航班的权限
    bool checkAdminFlightPermission(const QString& flightName, QString& err);
};

#endif // CONTROLLERFOOD_H
