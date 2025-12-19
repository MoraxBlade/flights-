#include "food.h"

Food::Food() : foodId(0), flightId(0) {}


int Food::getFoodId() const {
    return foodId;
}
int Food::getFlightId() const {
    return flightId;
}
QString Food::getFoodName() const {
    return foodName;
}

void Food::setFoodId(int id) {
    foodId = id;
}
void Food::setFlightId(int fid) {
    flightId = fid;
}
void Food::setFoodName(const QString& name) {
    foodName = name;
}
