#ifndef PLANE_H
#define PLANE_H
#include <QString>
class Plane
{
private:
    int PlaneID;
    QString PlaneName;
    int SeatCnt;
    QString planeType;
public:
    Plane();
    Plane(int _pid,const QString& _pname,int _seatcnt);
    Plane(int _pid,const QString& _pname,const QString& _ptype,int _seatcnt);
    virtual ~Plane();
    int getPlaneID()const;
    QString getPlaneName()const;
    int getSeatCnt()const;
    QString getPlaneType()const;
    bool isNarrowBody() const {
        return planeType.contains("窄体", Qt::CaseInsensitive);
    }

    bool isWideBody() const {
        return planeType.contains("宽体", Qt::CaseInsensitive);
    }
};

#endif // PLANE_H
