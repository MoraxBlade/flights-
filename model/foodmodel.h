#ifndef FOODMODEL_H
#define FOODMODEL_H
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

#include "../entity/food.h"
#include "../model/flightmodel.h"
class FoodModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit FoodModel(QObject *parent = nullptr, FlightModel *flightModel = nullptr);
    void init();
    //QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;// 重写 data 方法：实现FoodID->FoodName 映射


    // 重写 headerData：设置列名（仅显示「餐食名称」）
    //QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


    void clearQuery();
    bool getFoodListByFlightId(int flightId, QList<Food>& foodList, QString& err);
    bool loadFoodsByFlightName(const QString &flightName, QString &err);
    bool getFoodIdByFlightNameAndFoodName(const QString& flightNamed, const QString& foodName, int& foodId, QString& err);
    bool checkFoodDuplicate(int flightId, const QString &foodName, QString &err);
    int getLastInsertedFoodId(QString &err);
    bool insertFood(const Food &food, QString &err);
    bool deleteFood(int foodId, QString &err);

private:
    FlightModel *m_flightModel;
};

#endif // FOODMODEL_H
