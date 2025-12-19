#ifndef FLIGHTFILTERPROXYMODEL_H
#define FLIGHTFILTERPROXYMODEL_H
#include <QSortFilterProxyModel>
#include <QDateTime>

class FlightFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FlightFilterProxyModel(QObject *parent = nullptr);
    // 搜索条件
    QString CompanyName;
    QString StartCity;
    QString EndCity;
    QDateTime StartTimeFrom;
    QDateTime StartTimeTo;
    double PriceLowFrom;
    double PriceLowTo;
    //刷新过滤
    void refreshFilter();
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};
#endif // FLIGHTFILTERPROXYMODEL_H
