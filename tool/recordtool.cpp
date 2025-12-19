#include "recordtool.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include "../entity/admin.h"

bool RecordTool::logAdminOperation(const Record& record) {
    if (record.getAdminID() <= 0 || record.getCompanyID() <= 0) {// 基本参数校验
        qDebug() << "[RecordTool] 错误：管理员ID或公司ID无效";
        return false;
    }
    return insertRecordToDB(record);
}
bool RecordTool::logAdminOperation(Admin* admin, const QString& opType,
                                   const QString& targetType, int targetID,
                                   const QString& opDesc) {
    if (!admin) {// 入参校验
        qDebug() << "[RecordTool] 错误：管理员对象为空";
        return false;
    }

    Record record(
        admin->getAdminID(),
        admin->getCompanyID(),
        opType,
        targetType,
        targetID,
        opDesc,
        QDateTime::currentDateTime()
        );// 构建Record对象

    return insertRecordToDB(record);
}
bool RecordTool::insertRecordToDB(const Record& record) {
    QSqlDatabase db=QSqlDatabase::database();

    QSqlQuery query(db);
    query.prepare("INSERT INTO records (AdminID, CompanyID, OperationType, TargetType,TargetID, OperationDesc, OperationTime) "
                  "VALUES (:aid,:cid,:opty,:tgty,:tid,:desc,:tim)");
    query.bindValue(":aid",record.getAdminID());
    query.bindValue(":cid",record.getCompanyID());
    query.bindValue(":opty",record.getOperationType());
    query.bindValue(":tgty",record.getTargetType());
    query.bindValue(":tid",record.getTargetID() == -1 ? QVariant(QVariant::Int) : record.getTargetID());
    query.bindValue(":desc",record.getOperationDesc());
    query.bindValue(":tim",record.getOperationTime());

    if (!query.exec()) {
        qDebug() << "[RecordTool] 操作记录失败：" << query.lastError().text();
        return false;
    }
    qDebug() << "[RecordTool] 操作记录成功：" << record.getOperationType();
    return true;
}
bool RecordTool::GetRecordListByCompanyID(int cid, QList<Record>& records, QString& err)
{
    records.clear(); // 清空原有数据
    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    query.prepare("SELECT RecordID, AdminID, CompanyID, OperationType, TargetType, TargetID, OperationDesc, OperationTime "
                  "FROM records WHERE CompanyID = :cid ORDER BY OperationTime DESC");
    query.bindValue(":cid", cid);

    if (!query.exec()) {
        err = "查询操作记录失败：" + query.lastError().text();
        return false;
    }

    while (query.next()) {
        Record record;
        record.setId(query.value("RecordID").toInt());
        record.setAdminID(query.value("AdminID").toInt());
        record.setCompanyID(query.value("CompanyID").toInt());
        record.setOperationType(query.value("OperationType").toString());
        record.setTargetType(query.value("TargetType").toString());
        record.setTargetID(query.value("TargetID").toInt()); // 空值会自动转0，可在显示时处理
        record.setOperationDesc(query.value("OperationDesc").toString());
        record.setOperationTime(query.value("OperationTime").toDateTime());
        records.append(record);
    }
    return true;
}
