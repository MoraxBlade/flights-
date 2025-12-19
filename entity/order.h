#ifndef ORDER_H
#define ORDER_H

#include "food.h"
#include "flight.h"

class Order {
public:
    Order() = default;
    void setOrderId(int id);
    void setClientID(int id);
    void setFlightID(int id);
    void setSeatID(int id);
    void setLuggageID(int id);
    void setFlight(const Flight& f);
    void setCabintype(QString t);

    int getOrderId() const;
    int getClientID() const;
    int getFlightID() const;
    int getSeatID() const;
    int getLuggageID() const;
    QString getCabintype() const;
    Flight getFlight() const;

    int getFoodID() const { return foodID; }
    void setFoodID(int id) { foodID = id; }
    QString getFoodName() const { return foodName; }
    void setFoodName(const QString& name) { foodName = name; }

private:
    int orderId{ 0 };
    int ClientID{ 0 };
    int FlightID{ 0 };
    int foodID{ 0 };
    int SeatID{ 0 };
    int LuggageID{ 0 };
    Flight flight;
    Food food;
    QString cabinType;
    QString foodName;
};


#endif // ORDER_H
