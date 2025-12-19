#ifndef FLIGHTMODEL_H
#define FLIGHTMODEL_H
#include <QSqlTableModel>
#include <vector>
#include "../entity/flight.h"
class FlightModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit FlightModel(QObject *parent = nullptr);
    void init();
    // 基础行操作
    bool SelectByRow(int row, Flight& flight, QString& err);
    bool AppendRow(Flight& flight, QString& err);
    bool RemoveRow(int row, QString& err);
    bool ModifyRow(int row, Flight& flight, QString& err);
    // 批量操作
    bool CompanyBatchModify(const std::vector<int>& rows, int cid, double deltahour, const QString& updmess, QString& err);
    bool CompanyBatchDelete(const std::vector<int>& rows, int cid, QString& err);

    // 冲突检查
    bool CheckAddConflict(const Flight& flight, QString& err) const;
    bool CheckUpdConflict(int row, Flight& flight, QString& err) const;
    // 天气预警
    bool AddWeatherWarning(const QDateTime& t1, const QDateTime& t2, QString& err);
    bool DelWeatherWarning(const QDateTime& t1, const QDateTime& t2, QString& err);

    bool getFlightByFlightName(const QString& flightName, Flight& flight, QString& err);//严格来说是通过FlightName求FlightID,CompanyID
    int getLastInsertedFlightId(QString &err);

protected:
    // 遵循Qt规范：重写data()放在protected
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};
#endif // FLIGHTMODEL_H
