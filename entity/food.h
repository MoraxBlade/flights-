#ifndef FOOD_H
#define FOOD_H
#include <QString>
class Food {
private:
    int foodId;     // 自增主键
    int flightId;   // 关联航班ID
    QString foodName; // 餐食名称

public:
    Food();
    Food(int foodID,int flightID,QString& name)
        :foodId(foodID),flightId(flightID),foodName(name){}

    int getFoodId() const;
    int getFlightId() const;
    QString getFoodName() const;

    void setFoodId(int id);
    void setFlightId(int fid);
    void setFoodName(const QString& name);
};

#endif // FOOD_H
