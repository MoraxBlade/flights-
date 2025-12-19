#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "companymodel.h"
#include "planemodel.h"
#include "../entity/company.h"
extern std::map<int,Company*> companies;

void CompanyModel::LoadCompanies(){
    QSqlDatabase db=QSqlDatabase::database();
    QSqlQuery qrycpy(db);
    if(!qrycpy.exec("SELECT * FROM companies")) qDebug()<<qrycpy.lastError().text();
    while(qrycpy.next()){
        int cid=qrycpy.value("CompanyID").toInt();
        QString cname=qrycpy.value("CompanyName").toString();
        QString hashpwd=qrycpy.value("HashPwd").toString();
        QByteArray salt=QByteArray::fromHex(qrycpy.value("Salt").toString().toUtf8());
        Company* ptr=new Company(cid,cname,hashpwd,salt);
        QString err;
        if(!PlaneModel::GetPlanesByCompanyID(cid,ptr->Planes,err)) qDebug()<<err;
        companies[cid]=ptr;
    }
}
void CompanyModel::ReleaseCompanies(){
    for(auto& pr:companies) delete pr.second;
    companies.clear();
}
bool CompanyModel::InsertCompany(const QString& cname,const QString& hashpwd,const QByteArray& salt,QString& err){
    QSqlDatabase db=QSqlDatabase::database();
    QSqlQuery qrycpy(db);
    qrycpy.prepare("INSERT INTO companies (CompanyName,HashPwd,Salt) VALUES(:cname,:hashpwd,:salt)");
    qrycpy.bindValue(":cname",cname);
    qrycpy.bindValue(":hashpwd",hashpwd);
    qrycpy.bindValue(":salt",salt.toHex());
    if(!qrycpy.exec()){
        err=qrycpy.lastError().text();
        return 0;
    }
    int cid=qrycpy.lastInsertId().toInt();
    Company* ptr=new Company(cid,cname,hashpwd,salt);
    if(!PlaneModel::GetPlanesByCompanyID(cid,ptr->Planes,err)) qDebug()<<err;
    companies[cid]=ptr;
    err="";
    return 1;
}
 QString CompanyModel::getCompanyNameByCompanyId(int companyId){
    auto it = companies.find(companyId);
    if (it != companies.end()) {
        // 找到公司返回名称
        return it->second->getCompanyName();
    }
    // 未找到返回空字符串或其他标识
    return QString("未找到公司名");
}
