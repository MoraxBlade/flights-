#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "adminmodel.h"

bool AdminModel::GetCompanyID(int id,int& cid,QString& err){
    QSqlDatabase db=QSqlDatabase::database();
    QSqlQuery qryadm(db);
    qryadm.prepare("SELECT c.CompanyID FROM admins a JOIN companies c ON a.CompanyID=c.CompanyID WHERE a.AdminID=:id");
    qryadm.bindValue(":id",id);
    if(!qryadm.exec()){
        err=qryadm.lastError().text();
        return 0;
    }
    if(qryadm.next()){
        cid=qryadm.value("CompanyID").toInt();
        err="";
        return 1;
    }
    else{
        err="找不到用户所在的航司！";
        return 0;
    }
}

bool AdminModel::InsertAdmin(const QString& name,const QString& hashpwd,const QByteArray& salt,const int& cid,QString& err){
    QSqlDatabase db=QSqlDatabase::database();
    if(!db.transaction()){
        err="开启事务失败: "+db.lastError().text();
        return 0;
    }

    QSqlQuery qryusr(db);
    qryusr.prepare("INSERT INTO users (UserName,HashPwd,Salt,UserType) VALUES(:name,:hashpwd,:salt,:type)");
    qryusr.bindValue(":name",name);
    qryusr.bindValue(":hashpwd",hashpwd);
    qryusr.bindValue(":salt",salt.toHex());
    qryusr.bindValue(":type","admin");
    if(!qryusr.exec()){
        err=qryusr.lastError().text();
        db.rollback();
        return 0;
    }
    int id=qryusr.lastInsertId().toInt();
    qryusr.finish();

    QSqlQuery qryadm(db);
    qryadm.prepare("INSERT INTO admins (AdminID,CompanyID) VALUES(:id,:cid)");
    qryadm.bindValue(":id",id);
    qryadm.bindValue(":cid",cid);
    if(!qryadm.exec()){
        err=qryadm.lastError().text();
        db.rollback();
        return 0;
    }
    qryadm.finish();

    if(!db.commit()){
        err="提交失败："+db.lastError().text();
        db.rollback();
        return 0;
    }
    err="";
    return 1;
}
int AdminModel::getLastInsertedAdminId(QString &err) {
    // 复用FoodModel绑定的数据库连接（和insertFood用同一个连接）
    QSqlQuery query(QSqlDatabase::database());
    // MySQL 专用：获取最后一次插入的自增ID（仅当前连接有效，精准）
    if (query.exec("SELECT LAST_INSERT_ID()")) {
        if (query.next()) {
            return query.value(0).toInt(); // 返回新插入的FoodID
        }
    }
    // 失败时返回错误信息
    err = "获取新餐食ID失败：" + query.lastError().text();
    return -1;
}
