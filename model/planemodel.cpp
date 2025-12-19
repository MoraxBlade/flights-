#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "planemodel.h"
bool PlaneModel::GetPlanesByCompanyID(int cid,std::vector<Plane>& planes,QString& err){
    QSqlDatabase db=QSqlDatabase::database();
    QSqlQuery qrypla(db);
    qrypla.prepare("SELECT PlaneID,PlaneName,PlaneType,SeatCnt FROM planes WHERE CompanyID=:cid");
    qrypla.bindValue(":cid",cid);
    if(!qrypla.exec()){
        err=qrypla.lastError().text();
        return 0;
    }
    planes.clear();
    while(qrypla.next()){
        planes.push_back(Plane(qrypla.value("PlaneID").toInt(),qrypla.value("PlaneName").toString(),qrypla.value("PlaneType").toString(),qrypla.value("SeatCnt").toInt()));
    }
    err="";
    return 1;
}
int PlaneModel::InsertPlane(const QString& planeName, const QString& planeType,
                            int seatCnt, int companyID, QString& err)// 添加飞机，返回PlaneId
{
    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO planes (PlaneName, PlaneType, SeatCnt, CompanyID)
        VALUES (?, ?, ?, ?)
    )");
    query.addBindValue(planeName);
    query.addBindValue(planeType);
    query.addBindValue(seatCnt);
    query.addBindValue(companyID);
    if (!query.exec()) {
        err = query.lastError().text();
        return -1;
    }
    int id=query.lastInsertId().toInt();
    return id;
}

bool PlaneModel::DeletePlane(int planeID, QString& err)// 删除飞机
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery deleteQuery(db);
    deleteQuery.prepare("DELETE FROM planes WHERE PlaneID = :pid");
    deleteQuery.bindValue(":pid", planeID);
    if (!deleteQuery.exec()) {
        err = deleteQuery.lastError().text();
        return 0;
    }
    return 1;
}

bool PlaneModel::GetMaxPlaneID(int& maxID, QString& err)// 获取飞机表最大ID
{
    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    if (query.exec("SELECT MAX(PlaneID) FROM planes")) {
        if (query.next() && !query.value(0).isNull()) {
            maxID = query.value(0).toInt();
        } else {
            maxID = 0; // 表为空时默认ID从0开始
        }
        return true;
    } else {
        err = query.lastError().text();
        return false;
    }
}
