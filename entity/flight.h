#ifndef FLIGHT_H
#define FLIGHT_H
#include <QString>
#include <QDateTime>
class Flight
{
private:
    int FlightID=0;
    QString FlightName;
    int CompanyID=0;
    int PlaneID=0;
    int SeatCnt=0;
    int SeatLow=0;
    int SeatMid=0;
    int SeatHigh=0;
    QString StartCity;
    QString EndCity;
    QDateTime StartTime;
    QDateTime EndTime;
    double Duration=0.0;
    double PriceLow=0.0;
    double PriceMid=0.0;
    double PriceHigh=0.0;
    QString Message;
    QString toQString(int num,int width);
public:
    Flight();
    virtual ~Flight();
    void setFlightID(int _fid);
    void setCompanyID(int _cid);
    void setPlaneID(int _pid);
    void setFlightName();
    void setFlightName(const QString& _fName){FlightName=_fName;}
    void setSeatCnt(int _seatcnt);
    void setSeatLow(int _seatlow);
    void setSeatMid(int _seatmid);
    void setSeatHigh(int _seathigh);
    void setStartCity(const QString& _cityname);
    void setEndCity(const QString& _cityname);
    void setStartTime(const QDateTime& _time);
    void setEndTime(const QDateTime& _time);
    void setDuration(double _dur);
    void setPrice(double _pricel,double _pricem,double _priceh);
    void setMessage(const QString& _mes);
    int getFlightID()const;
    QString getFlightName()const;
    int getCompanyID()const;
    int getPlaneID()const;
    int getSeatCnt()const;
    int getSeatLow()const;
    int getSeatMid()const;
    int getSeatHigh()const;
    QString getStartCity()const;
    QString getEndCity()const;
    QDateTime getStartTime()const;
    QDateTime getEndTime()const;
    double getDuration()const;
    double getPriceLow()const;
    double getPriceMid()const;
    double getPriceHigh()const;
    QString getMessage()const;
};

#endif // FLIGHT_H
