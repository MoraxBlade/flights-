#ifndef COMPANYMODEL_H
#define COMPANYMODEL_H
#include<QString>

class CompanyModel{
public:
    static void LoadCompanies();
    static void ReleaseCompanies();
    static bool InsertCompany(const QString& cname,const QString& hashpwd,const QByteArray& salt,QString& err);
    QString getCompanyNameByCompanyId(int companyId);
};

#endif // COMPANYMODEL_H
