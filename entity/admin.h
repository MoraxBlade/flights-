#ifndef ADMIN_H
#define ADMIN_H
#include "user.h"

class Admin:public User{
private:
    int AdminID;
    int CompanyID;
public:
    Admin();
    Admin(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type,int _cid);
    int getCompanyID();
    int getAdminID();
    void setAdminID(int id){AdminID=id;}
    void setCompanyID(int id){CompanyID=id;}
    virtual ~Admin();
};

#endif // ADMIN_H
