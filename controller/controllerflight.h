#ifndef CONTROLLERFLIGHT_H
#define CONTROLLERFLIGHT_H
#include <QObject>
#include "../model/flightmodel.h"
class ControllerFlight:public QObject{
    Q_OBJECT
public:
    explicit ControllerFlight(QObject* parent = nullptr,FlightModel *model = nullptr);
    bool AddFlight(Flight& flight,QString& err);
    bool RemoveFlight(int row,QString& err);
    bool ModifyFlight(int row,Flight& flight,QString& err);
    bool BadWeather(const QDateTime& t1,const QDateTime& t2,QString& err);
    bool GoodWeather(const QDateTime& t1,const QDateTime& t2,QString& err);
    bool BatchModify(const std::vector<int>& rows,int cid,double deltahour,const QString& updmess,QString& err);
    bool BatchDelete(const std::vector<int>& rows,int cid,QString& err);
signals:
    void TableFlightsModified();
private:
    FlightModel *flightmodel;
};

#endif // CONTROLLERFLIGHT_H
