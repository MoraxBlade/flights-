#ifndef COMPANY_H
#define COMPANY_H
#include <QString>
#include "plane.h"

class Company{
private:
    int CompanyID;
    QString CompanyName;
    QString HashPwd;
    QByteArray Salt;//盐
public:
    std::vector<Plane> Planes;
    Company();
    Company(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt);
    virtual ~Company();
    int getCompanyID();
    QString getCompanyName();
    QString getHashPwd();
    QByteArray getSalt();
};

#endif // COMPANY_H
