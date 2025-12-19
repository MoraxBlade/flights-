#include "flightfilterproxymodel.h"

FlightFilterProxyModel::FlightFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent){
    StartTimeFrom=(QDateTime(QDate(2000,1,1), QTime(0,0,0)));
    StartTimeTo=(QDateTime(QDate(2050,1,1), QTime(23,59,59)));
    PriceLowFrom=00.00;
    PriceLowTo=99999.99;
}

bool FlightFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const{
    QAbstractItemModel* m = sourceModel();

    QString cname=m->index(row, 2, parent).data().toString();
    QString startcity=m->index(row, 5, parent).data().toString();
    QString endcity=m->index(row, 6, parent).data().toString();
    QDateTime stim=m->index(row, 7, parent).data().toDateTime();
    double plow=m->index(row, 10, parent).data().toDouble();

    // 过滤：航司（模糊搜索）
    if(!CompanyName.isEmpty()&&!cname.contains(CompanyName, Qt::CaseInsensitive)) return false;

    // 过滤：出发城市（模糊搜索）
    if(!StartCity.isEmpty() && !startcity.contains(StartCity, Qt::CaseInsensitive)) return false;

    // 过滤：到达城市（模糊搜索）
    if(!EndCity.isEmpty() && !endcity.contains(EndCity, Qt::CaseInsensitive)) return false;

    // 过滤：时间范围（出发时间应在指定范围）
    if(StartTimeFrom.isValid() && stim<StartTimeFrom) return false;
    if(StartTimeTo.isValid() && stim>StartTimeTo) return false;

    // 过滤：票价
    if(plow<PriceLowFrom||plow>PriceLowTo) return false;
    return true;
}

void FlightFilterProxyModel::refreshFilter(){
    invalidateFilter();
}
