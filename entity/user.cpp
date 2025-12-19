#include "user.h"

User::User() {}

User::User(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type)
    :UserID(_id),UserName(_name),HashPwd(_hashpwd),Salt(_salt),UserType(_type){}

User::~User(){}

void User::setUserID(int _id){
    UserID=_id;
}

void User::setUserName(const QString& _name){
    UserName=_name;
}

void User::setHashPwd(const QString& _hashpwd){
    HashPwd=_hashpwd;
}

void User::setSalt(const QByteArray& _salt){
    Salt=_salt;
}

void User::setUserType(const QString& _type){
    UserType=_type;
}

int User::getUserID()const{
    return UserID;
}

QString User::getUserName()const{
    return UserName;
}

QString User::getHashPwd()const{
    return HashPwd;
}

QByteArray User::getSalt()const{
    return Salt;
}

QString User::getUserType()const{
    return UserType;
}

std::vector<Order> User::getOrders() const {
    QString error;
    auto orders = orderModel->getOrdersByClientId(UserID, error);
    if (!error.isEmpty()) {
        qDebug() << "获取订单失败:" << error;
        return {};
    }
    return orders;
}
