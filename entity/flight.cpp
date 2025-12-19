#include "flight.h"

Flight::Flight() {}

Flight::~Flight(){}

QString Flight::toQString(int num,int width){
    QString str;
    while(num){
        int x=num%10;
        str+=char('0'+x);
        num/=10;
    }
    while(str.length()<width) str='0'+str;
    return str;
}

void Flight::setFlightID(int _fid){
    FlightID=_fid;
}
void Flight::setCompanyID(int _cid){
    CompanyID=_cid;
}
void Flight::setPlaneID(int _pid){
    PlaneID=_pid;
}
void Flight::setFlightName(){
    if(FlightID&&CompanyID&&PlaneID) FlightName="F"+toQString(FlightID,5)+"C"+toQString(CompanyID,2)+"P"+toQString(PlaneID,3);
    else FlightName="";
}
void Flight::setSeatCnt(int _seatcnt){
    SeatCnt=_seatcnt;
}
void Flight::setSeatLow(int _seatlow){
    SeatLow=_seatlow;
}
void Flight::setSeatMid(int _seatmid){
    SeatMid=_seatmid;
}
void Flight::setSeatHigh(int _seathigh){
    SeatLow=_seathigh;
}
void Flight::setStartCity(const QString& _cityname){
    StartCity=_cityname;
}
void Flight::setEndCity(const QString& _cityname){
    EndCity=_cityname;
}
void Flight::setStartTime(const QDateTime& _time){
    StartTime=_time;
}
void Flight::setEndTime(const QDateTime& _time){
    EndTime=_time;
}
void Flight::setDuration(double _dur){
    Duration=_dur;
}
void Flight::setPrice(double _pricel,double _pricem,double _priceh){
    PriceLow=_pricel;
    PriceMid=_pricem;
    PriceHigh=_priceh;
}
void Flight::setMessage(const QString& _mes){
    Message=_mes;
}

int Flight::getFlightID()const{
    return FlightID;
}
QString Flight::getFlightName()const{
    return FlightName;
}
int Flight::getCompanyID()const{
    return CompanyID;
}
int Flight::getPlaneID()const{
    return PlaneID;
}
int Flight::getSeatCnt()const{
    return SeatCnt;
}
int Flight::getSeatLow()const{
    return SeatLow;
}
int Flight::getSeatMid()const{
    return SeatMid;
}
int Flight::getSeatHigh()const{
    return SeatHigh;
}
QString Flight::getStartCity()const{
    return StartCity;
}
QString Flight::getEndCity()const{
    return EndCity;
}
QDateTime Flight::getStartTime()const{
    return StartTime;
}
QDateTime Flight::getEndTime()const{
    return EndTime;
}
double Flight::getDuration()const{
    return Duration;
}
double Flight::getPriceLow()const{
    return PriceLow;
}
double Flight::getPriceMid()const{
    return PriceMid;
}
double Flight::getPriceHigh()const{
    return PriceHigh;
}
QString Flight::getMessage()const{
    return Message;
}
