#include "plane.h"

Plane::Plane() {}

Plane::Plane(int _pid,const QString& _pname,int _seatcnt)
    :PlaneID(_pid),PlaneName(_pname),SeatCnt(_seatcnt){planeType="";}

Plane::Plane(int _pid, const QString& _pname, const QString &_ptype, int _seatcnt)
    :PlaneID(_pid), PlaneName(_pname), SeatCnt(_seatcnt), planeType(_ptype) {}
Plane::~Plane(){}

int Plane::getPlaneID()const{
    return PlaneID;
}
QString Plane::getPlaneName()const{
    return PlaneName;
}
QString Plane::getPlaneType()const{
    return planeType;
}
int Plane::getSeatCnt()const{
    return SeatCnt;
}
