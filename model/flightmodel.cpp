#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include "flightmodel.h"
#include "../model/planemodel.h"
#include "../entity/company.h"
extern std::map<int, Company*> companies;
FlightModel::FlightModel(QObject *parent)
    : QSqlTableModel(parent) {}
void FlightModel::init() {
    setTable("flights");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setHeaderData(0, Qt::Horizontal, "航班ID");
    setHeaderData(1, Qt::Horizontal, "航班号");
    setHeaderData(2, Qt::Horizontal, "航司");
    setHeaderData(3, Qt::Horizontal, "飞机");
    setHeaderData(4, Qt::Horizontal, "总座位数");
    setHeaderData(5, Qt::Horizontal, "起点");
    setHeaderData(6, Qt::Horizontal, "终点");
    setHeaderData(7, Qt::Horizontal, "起飞时间");
    setHeaderData(8, Qt::Horizontal, "到达时间");
    setHeaderData(9, Qt::Horizontal, "时长");
    setHeaderData(10, Qt::Horizontal, "经济舱票价");
    setHeaderData(11, Qt::Horizontal, "商务舱票价");
    setHeaderData(12, Qt::Horizontal, "头等舱票价");
    setHeaderData(13, Qt::Horizontal, "经济舱剩余票数");
    setHeaderData(14, Qt::Horizontal, "商务舱剩余票数");
    setHeaderData(15, Qt::Horizontal, "头等舱剩余票数");
    setHeaderData(16, Qt::Horizontal, "提示信息");
    select();
}
QVariant FlightModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.column() == 2) { // CompanyID映射成CompanyName
            int cid = QSqlTableModel::data(index, Qt::EditRole).toInt();
            if (companies.find(cid) != companies.end())
                return companies[cid]->getCompanyName();
            else
                return QString();
        } else if (index.column() == 3) { // PlaneID映射成PlaneName
            QModelIndex companyIndex = index.sibling(index.row(), 2);
            int cid = QSqlTableModel::data(companyIndex, Qt::EditRole).toInt();
            int pid = QSqlTableModel::data(index, Qt::EditRole).toInt();
            if (companies.find(cid) == companies.end())
                return QString();
            Company* ptr = companies[cid];
            for (const auto& plane : ptr->Planes) {
                if (plane.getPlaneID() == pid)
                    return plane.getPlaneName();
            }
            return QString();
        } else if (index.column()>=13&&index.column()<=15){//由购买票数映射回剩余票数
            QModelIndex seatIndex = index.sibling(index.row(), 4);
            int totalseat = QSqlTableModel::data(seatIndex, Qt::EditRole).toInt();
            int bought=QSqlTableModel::data(index, Qt::EditRole).toInt();
            int seatcnt[3];
            seatcnt[0]=totalseat*0.8;
            seatcnt[1]=totalseat*0.1;
            seatcnt[2]=totalseat-seatcnt[0]-seatcnt[1];
            return seatcnt[index.column()-13]-bought;
        }
        return QSqlTableModel::data(index, role); // 其他情况用默认实现
    }
    return QSqlTableModel::data(index, role); // 其他情况用默认实现
}
bool FlightModel::SelectByRow(int row, Flight& flight, QString& err) {
    if (row < 0 || row >= this->rowCount()) {
        err = QString("Invalid row index %1").arg(row);
        return false;
    }
    QSqlRecord rec = this->record(row);
    flight.setFlightID(rec.value("FlightID").toInt());
    flight.setCompanyID(rec.value("CompanyID").toInt());
    flight.setPlaneID(rec.value("PlaneID").toInt());
    flight.setFlightName();
    flight.setSeatCnt(rec.value("SeatCnt").toInt());
    flight.setStartCity(rec.value("StartCity").toString());
    flight.setEndCity(rec.value("EndCity").toString());
    flight.setStartTime(rec.value("StartTime").toDateTime());
    flight.setEndTime(rec.value("EndTime").toDateTime());
    flight.setDuration(rec.value("Duration").toDouble());
    flight.setPrice(rec.value("PriceLow").toDouble(),
                    rec.value("PriceMid").toDouble(),
                    rec.value("PriceHigh").toDouble());
    flight.setSeatLow(rec.value("SeatLow").toInt());
    flight.setSeatMid(rec.value("SeatMid").toInt());
    flight.setSeatHigh(rec.value("SeatHigh").toInt());
    flight.setMessage(rec.value("Message").toString());
    err = "";
    return true;
}
bool FlightModel::AppendRow(Flight& flight, QString& err) {
    int row = this->rowCount();
    QSqlRecord lstrec = this->record(row - 1);
    flight.setFlightID(lstrec.value("FlightID").toInt() + 1);
    flight.setFlightName();
    this->insertRow(row);
    this->setData(this->index(row, 1), flight.getFlightName());
    this->setData(this->index(row, 2), flight.getCompanyID());
    this->setData(this->index(row, 3), flight.getPlaneID());
    this->setData(this->index(row, 4), flight.getSeatCnt());
    this->setData(this->index(row, 5), flight.getStartCity());
    this->setData(this->index(row, 6), flight.getEndCity());
    this->setData(this->index(row, 7), flight.getStartTime());
    this->setData(this->index(row, 8), flight.getEndTime());
    this->setData(this->index(row, 9), flight.getDuration());
    this->setData(this->index(row, 10), flight.getPriceLow());
    this->setData(this->index(row, 11), flight.getPriceMid());
    this->setData(this->index(row, 12), flight.getPriceHigh());
    this->setData(this->index(row, 13), flight.getSeatLow());
    this->setData(this->index(row, 14), flight.getSeatMid());
    this->setData(this->index(row, 15), flight.getSeatHigh());
    this->setData(this->index(row, 16), flight.getMessage());
    if (!this->submitAll()) {
        err = this->lastError().text();
        this->revertAll();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::RemoveRow(int row, QString& err) {
    if (!this->removeRow(row)) {
        err = this->lastError().text();
        this->revertAll();
        return false;
    }
    if (!this->submitAll()) {
        err = this->lastError().text();
        this->revertAll();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::ModifyRow(int row, Flight& flight, QString& err) {
    QSqlRecord rec = this->record(row);
    flight.setFlightID(rec.value("FlightID").toInt());
    flight.setFlightName();
    this->setData(this->index(row, 1), flight.getFlightName());
    this->setData(this->index(row, 2), flight.getCompanyID());
    this->setData(this->index(row, 3), flight.getPlaneID());
    this->setData(this->index(row, 4), flight.getSeatCnt());
    this->setData(this->index(row, 5), flight.getStartCity());
    this->setData(this->index(row, 6), flight.getEndCity());
    this->setData(this->index(row, 7), flight.getStartTime());
    this->setData(this->index(row, 8), flight.getEndTime());
    this->setData(this->index(row, 9), flight.getDuration());
    this->setData(this->index(row, 10), flight.getPriceLow());
    this->setData(this->index(row, 11), flight.getPriceMid());
    this->setData(this->index(row, 12), flight.getPriceHigh());
    //已购票数信息不变更
    this->setData(this->index(row, 16), flight.getMessage());
    if (!this->submitAll()) {
        err = this->lastError().text();
        this->revertAll();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::CheckAddConflict(const Flight& flight, QString& err) const {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery qryfli(db);
    qryfli.prepare("SELECT StartTime,EndTime FROM flights WHERE PlaneID=:pid AND (StartTime<=:etim AND EndTime>=:stim) LIMIT 1");
    qryfli.bindValue(":pid", flight.getPlaneID());
    qryfli.bindValue(":etim", flight.getEndTime());
    qryfli.bindValue(":stim", flight.getStartTime());
    if (!qryfli.exec()) {
        err = "查询冲突航班失败：" + qryfli.lastError().text();
        return false;
    }
    if (qryfli.next()) {
        err = QString("飞机在 %1 - %2 时间段有其他飞行任务")
                  .arg(qryfli.value("StartTime").toDateTime().toString("yyyy-MM-dd hh:mm")
                       , qryfli.value("EndTime").toDateTime().toString("yyyy-MM-dd hh:mm"));
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::CheckUpdConflict(int row, Flight& flight, QString& err) const {
    QSqlRecord rec = this->record(row);
    flight.setFlightID(rec.value("FlightID").toInt());
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery qryfli(db);
    qryfli.prepare("SELECT StartTime,EndTime FROM flights WHERE PlaneID=:pid AND (StartTime<=:etim AND EndTime>=:stim) AND FlightID!=:fid LIMIT 1");
    qryfli.bindValue(":pid", flight.getPlaneID());
    qryfli.bindValue(":etim", flight.getEndTime());
    qryfli.bindValue(":stim", flight.getStartTime());
    qryfli.bindValue(":fid", flight.getFlightID());
    if (!qryfli.exec()) {
        err = "查询冲突航班失败：" + qryfli.lastError().text();
        return false;
    }
    if (qryfli.next()) {
        err = QString("飞机在 %1 - %2 时间段有其他飞行任务")
                  .arg(qryfli.value("StartTime").toDateTime().toString("yyyy-MM-dd hh:mm")
                       , qryfli.value("EndTime").toDateTime().toString("yyyy-MM-dd hh:mm"));
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::AddWeatherWarning(const QDateTime& t1, const QDateTime& t2, QString& err) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery qryfli(db);
    qryfli.prepare("UPDATE flights SET Message=:str WHERE (StartTime BETWEEN :t1 AND :t2) OR (EndTime BETWEEN :t1 AND :t2)");
    qryfli.bindValue(":str", "遇恶劣天气，建议延误");
    qryfli.bindValue(":t1", t1);
    qryfli.bindValue(":t2", t2);
    if (!qryfli.exec()) {
        err = qryfli.lastError().text();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::DelWeatherWarning(const QDateTime& t1, const QDateTime& t2, QString& err) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery qryfli(db);
    qryfli.prepare("UPDATE flights SET Message=:str WHERE (StartTime BETWEEN :t1 AND :t2) OR (EndTime BETWEEN :t1 AND :t2)");
    qryfli.bindValue(":str", "");
    qryfli.bindValue(":t1", t1);
    qryfli.bindValue(":t2", t2);
    if (!qryfli.exec()) {
        err = qryfli.lastError().text();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::CompanyBatchModify(const std::vector<int>& rows, int cid, double deltahour, const QString& updmess, QString& err) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        err = "开启事务失败: " + db.lastError().text();
        return false;
    }
    for (int row : rows) {
        Flight flight;
        if (!SelectByRow(row, flight, err)) {
            db.rollback();
            return false;
        }
        if (flight.getCompanyID() != cid) continue;
        QDateTime updStartTime = flight.getStartTime().addMSecs(static_cast<qint64>(deltahour * 3600 * 1000));
        QDateTime updEndTime = flight.getEndTime().addMSecs(static_cast<qint64>(deltahour * 3600 * 1000));
        flight.setStartTime(updStartTime);
        flight.setEndTime(updEndTime);
        if(!CheckUpdConflict(row, flight,err)){
            db.rollback();
            return false;
        }
        QSqlQuery qryfli(db);
        qryfli.prepare("UPDATE flights SET StartTime=:updstim,EndTime=:updetim,Message=:str WHERE FlightID=:fid");
        qryfli.bindValue(":updstim", updStartTime);
        qryfli.bindValue(":updetim", updEndTime);
        qryfli.bindValue(":str", updmess);
        qryfli.bindValue(":fid", flight.getFlightID());
        if (!qryfli.exec()) {
            err = qryfli.lastError().text();
            db.rollback();
            return false;
        }
        qryfli.finish();
    }
    if (!db.commit()) {
        err = "提交失败：" + db.lastError().text();
        db.rollback();
        return false;
    }
    err = "";
    return true;
}
bool FlightModel::CompanyBatchDelete(const std::vector<int>& rows, int cid, QString& err) {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        err = "开启事务失败: " + db.lastError().text();
        return false;
    }
    QSqlQuery qryfli(db);
    qryfli.prepare("DELETE FROM flights WHERE FlightID=:fid");
    for (int row : rows) {
        Flight flight;
        if (!SelectByRow(row, flight, err)) {
            db.rollback();
            return false;
        }
        if (flight.getCompanyID() != cid) continue;
        qryfli.bindValue(":fid", flight.getFlightID());
        if (!qryfli.exec()) {
            err = qryfli.lastError().text();
            db.rollback();
            return false;
        }
        qryfli.finish();
    }
    if (!db.commit()) {
        err = "提交失败：" + db.lastError().text();
        db.rollback();
        return false;
    }
    err = "";
    return true;
}

bool FlightModel::getFlightByFlightName(const QString& flightName, Flight& flight, QString& err) {
    QString trimmedName = flightName.trimmed();
    if (trimmedName.isEmpty()) {
        err = "航班号不能为空";
        return false;
    }
    setFilter(QString("FlightName = '%1'").arg(trimmedName));
    if (!select()) {
        err = "查询航班失败：" + lastError().text();
        setFilter("");
        return false;
    }
    if (rowCount() == 0) {
        err = QString("航班号「%1」不存在").arg(trimmedName);
        setFilter("");
        return false;
    }
    flight.setFlightID(record(0).value("FlightID").toInt());
    flight.setCompanyID(record(0).value("CompanyID").toInt());
    setFilter(""); // 清除筛选，恢复全量数据
    select();
    err.clear();
    return true;
}

int FlightModel::getLastInsertedFlightId(QString &err) {// 返回最后插入的FlightID
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    if (query.exec("SELECT LAST_INSERT_ID()")) {// 获取最后一次插入的自增ID
        if (query.next()) {
            return query.value(0).toInt(); // 返回新插入的FlightID
        }
    }
    err = "获取新航班ID失败：" + query.lastError().text();
    return -1;
}
