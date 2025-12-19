#include "admin.h"

Admin::Admin():User() {}

Admin::Admin(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type,int _cid)
    :User(_id,_name,_hashpwd,_salt,_type), AdminID(_id), CompanyID(_cid) {}

Admin::~Admin(){}

int Admin::getCompanyID(){
    return CompanyID;
}

int Admin::getAdminID(){
    return AdminID;
}
