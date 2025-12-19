#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "usermodel.h"
#include "../entity/user.h"

bool UserModel::SelectByName(const QString& name,User& user,QString& err){
    QSqlDatabase db=QSqlDatabase::database();
    QSqlQuery qryusr(db);
    qryusr.prepare("SELECT * FROM users WHERE UserName=:name");
    qryusr.bindValue(":name",name);
    if(!qryusr.exec()){
        err=qryusr.lastError().text();
        return 0;
    }
    if(qryusr.next()){
        user.setUserID(qryusr.value("UserID").toInt());
        user.setUserName(name);
        user.setHashPwd(qryusr.value("HashPwd").toString());
        user.setSalt(QByteArray::fromHex(qryusr.value("Salt").toString().toUtf8()));
        user.setUserType(qryusr.value("UserType").toString());
        err="";
        return 1;
    }
    else{
        err="用户名不存在！";
        return 0;
    }
}
