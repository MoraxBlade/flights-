#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "clientmodel.h"

bool ClientModel::InsertClient(const QString& name,const QString& hashpwd,const QByteArray& salt,QString& err){
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
    qryusr.bindValue(":type","client");
    if(!qryusr.exec()){
        err=qryusr.lastError().text();
        db.rollback();
        return 0;
    }
    int id=qryusr.lastInsertId().toInt();
    qryusr.finish();


    QSqlQuery qrycli(db);
    qrycli.prepare("INSERT INTO clients (ClientID) VALUES(:id)");
    qrycli.bindValue(":id",id);
    if(!qrycli.exec()){
        err=qrycli.lastError().text();
        db.rollback();
        return 0;
    }
    qrycli.finish();

    if(!db.commit()){
        err="提交失败："+db.lastError().text();
        db.rollback();
        return 0;
    }
    err="";
    return 1;
}
