#include "controlleruser.h"
#include "controllercompany.h"
#include "../entity/user.h"
#include "../entity/client.h"
#include "../entity/admin.h"
#include "../model/usermodel.h"
#include "../model/clientmodel.h"
#include "../model/adminmodel.h"
#include "../tool/encoder.h"
#include"../tool/recordtool.h"
#include <QDebug>
#include<QSqlQuery>
#include<QSqlError>
extern User *curUser;

ControllerUser::ControllerUser(QObject* parent,ControllerCompany *_controllercompany)
    : QObject(parent),controllercompany(_controllercompany){}

bool ControllerUser::LoginUser(const QString& name,const QString& ori_pwd,QString& err){
    if(name==""){
        err="用户名不能为空！";
        return 0;
    }
    if(ori_pwd==""){
        err="密码不能为空！";
        return 0;
    }
    User usr;
    if(!UserModel::SelectByName(name,usr,err)) return 0;
    QString encry_pwd=Encoder::Hash(ori_pwd,usr.getSalt());
    if(usr.getHashPwd()!=encry_pwd){
        err="密码错误！";
        return 0;
    }
    if(usr.getUserType()=="client"){
        if(!curUser) delete curUser;
        curUser=new Client(usr.getUserID(),name,encry_pwd,usr.getSalt(),"client");
        emit clientLoggedIn();
    }
    else if(usr.getUserType()=="admin"){
        int cid=-1;
        if(!AdminModel::GetCompanyID(usr.getUserID(),cid,err)) return 0;
        if(!curUser) delete curUser;
        curUser=new Admin(usr.getUserID(),name,encry_pwd,usr.getSalt(),"admin",cid);
        Admin* tempAdmin = dynamic_cast<Admin*>(curUser);
        emit adminLoggedIn();
    }
    err="";
    return 1;
}

bool ControllerUser::ChangePassword(const QString &oldPassword,
                                    const QString &newPassword,
                                    QString &errorMessage)
{
    QString encry_pwd = Encoder::Hash(oldPassword, curUser->getSalt());
    QString new_pwd = Encoder::Hash(newPassword, curUser->getSalt());

    if (curUser->getHashPwd() != encry_pwd) {
        errorMessage = "原密码错误！";
        return false;
    }

    if (curUser->getHashPwd() == new_pwd) {
        errorMessage = "新密码不能和原密码一样！";
        return false;
    }

    // 生成新密码
    QByteArray newSalt = Encoder::GenerateSalt();
    QString newHash = Encoder::Hash(newPassword, newSalt);
    QString newSaltHex = newSalt.toHex();

    // 直接更新
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE users SET HashPwd = ?, Salt = ? WHERE UserName = ?");
    updateQuery.addBindValue(newHash);
    updateQuery.addBindValue(newSaltHex);
    updateQuery.addBindValue(curUser->getUserName());

    if (!updateQuery.exec()) {
        errorMessage = "更新失败：" + updateQuery.lastError().text();
        return false;
    }


    if (updateQuery.numRowsAffected() <= 0) {
        errorMessage = "密码更新失败，未找到用户记录！";
        return false;
    }

    // 🔥 关键修复：更新内存中的用户对象
    curUser->setHashPwd(newHash);
    curUser->setSalt(newSalt);

    errorMessage = "密码修改成功！";
    return true;
}

bool ControllerUser::RegisterUser(const QString& name,const QString& ori_pwd,const QString& reori_pwd,QString& err){
    if(name==""){
        err="用户名不能为空！";
        return 0;
    }
    if(ori_pwd==""){
        err="密码不能为空！";
        return 0;
    }
    if(ori_pwd!=reori_pwd){
        err="两次输入密码不一致！";
        return 0;
    }
    QByteArray salt=Encoder::GenerateSalt();
    QString encry_pwd=Encoder::Hash(ori_pwd,salt);
    if(!ClientModel::InsertClient(name,encry_pwd,salt,err)) return 0;
    err="";
    return 1;
}
bool ControllerUser::RegisterUser(const QString& name,const QString& ori_pwd,const QString& reori_pwd,const int& cid,const QString& cpwd,QString& err){
    if(name==""){
        err="用户名不能为空！";
        return 0;
    }
    if(ori_pwd==""){
        err="密码不能为空！";
        return 0;
    }
    if(ori_pwd!=reori_pwd){
        err="两次输入密码不一致！";
        return 0;
    }
    if(!controllercompany->Check(cid,cpwd,err)) return 0;
    QByteArray salt=Encoder::GenerateSalt();
    QString encry_pwd=Encoder::Hash(ori_pwd,salt);
    if(!AdminModel::InsertAdmin(name,encry_pwd,salt,cid,err)) return 0;

    // 记录日志

    int newAdminId = AdminModel::getLastInsertedAdminId(err);
    if (newAdminId <= 0) {
        qWarning() << "[注册管理员] 获取新管理员ID失败，日志记录跳过";
        err = ""; // 不影响注册成功，仅跳过日志
        return 1;
    }

    // 2. 构造新管理员的Admin对象（无需依赖curUser）
    Admin* newAdmin = new Admin();
    newAdmin->setAdminID(newAdminId);       // 新管理员的ID
    newAdmin->setCompanyID(cid);            // 注册时指定的公司ID


    // 3. 记录日志：操作人是新注册的管理员自身
    bool logSuccess = RecordTool::logAdminOperation(
        newAdmin,
        OpType::ADD,
        TargetType::USER,
        newAdminId,
        QString("注册新管理员「%1」（管理员ID：%2，所属公司ID：%3）").arg(name).arg(newAdminId).arg(cid)
        );

    // 4. 释放临时对象，避免内存泄漏
    delete newAdmin;
    err="";
    return 1;
}

void ControllerUser::LogoutUser(){
    curUser=nullptr;
    emit userLoggedOut();
}
