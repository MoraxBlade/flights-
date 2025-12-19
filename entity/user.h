#ifndef USER_H
#define USER_H
#include<QString>
#include"order.h"
#include"../model/ordermodel.h"

class User{
private:
    int UserID;
    OrderModel* orderModel;
    QString UserName;
    QString HashPwd;
    QByteArray Salt;//盐
    QString UserType;
public:
    User();
    User(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type);
    virtual ~User();
    void setUserID(int _id);
    void setUserName(const QString& _name);
    void setHashPwd(const QString& _hashpwd);
    void setSalt(const QByteArray& _salt);
    void setUserType(const QString& _type);
    int getUserID()const;
    QString getUserName()const;
    QString getHashPwd()const;
    QByteArray getSalt()const;
    QString getUserType()const;
    std::vector<Order> getOrders() const;
    int getOrderCount() const;
};

#endif // USER_H
