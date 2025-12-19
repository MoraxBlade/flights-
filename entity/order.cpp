#include "order.h"

void Order::setOrderId(int id) { orderId = id; }
void Order::setClientID(int id) { ClientID = id; }
void Order::setFlightID(int id) { FlightID = id; }
void Order::setSeatID(int id) { SeatID = id; }
void Order::setLuggageID(int id) { LuggageID = id; }
void Order::setFlight(const Flight& f) { flight = f; }
void Order::setCabintype(QString t) {cabinType=t;}

int Order::getOrderId() const { return orderId; }
int Order::getClientID() const { return ClientID; }
int Order::getFlightID() const { return FlightID; }
int Order::getSeatID() const { return SeatID; }
int Order::getLuggageID() const { return LuggageID; }
QString Order::getCabintype() const {return cabinType;}
Flight Order::getFlight() const { return flight; }
