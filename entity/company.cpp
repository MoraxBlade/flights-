#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "company.h"

Company::Company() {}

Company::Company(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt)
    :CompanyID(_id),CompanyName(_name),HashPwd(_hashpwd),Salt(_salt){}

Company::~Company(){}

int Company::getCompanyID(){
    return CompanyID;
}

QString Company::getCompanyName(){
    return CompanyName;
}

QString Company::getHashPwd(){
    return HashPwd;
}

QByteArray Company::getSalt(){
    return Salt;
}
