#include <map>
#include "controllercompany.h"
#include "../model/companymodel.h"
#include "../entity/company.h"
#include "../tool/encoder.h"
#include"../entity/admin.h"
#include"../tool/recordtool.h"
extern std::map<int,Company*> companies;

ControllerCompany::ControllerCompany(QObject* parent)
    : QObject(parent){}

bool ControllerCompany::RegisterCompany(const QString& cname,const QString& ori_pwd,const QString& reori_pwd,const QString& pubkey,QString& err){
    if(cname==""){
        err="公司名不能为空！";
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
    if(pubkey==""){
        err="权限密钥不能为空！";
        return 0;
    }
    if(Encoder::Hash(pubkey,Encoder::pubSalt)!=Encoder::pubHash){
        err="权限密钥输入错误！";
        return 0;
    }
    QByteArray salt=Encoder::GenerateSalt();
    QString encry_pwd=Encoder::Hash(ori_pwd,salt);
    if(!CompanyModel::InsertCompany(cname,encry_pwd,salt,err)) return 0;

    emit companyRegistered();
    err="";
    return 1;
}
bool ControllerCompany::Check(const int& cid,const QString& ori_pwd,QString& err){
    if(ori_pwd==""){
        err="公司密码不能为空！";
        return 0;
    }
    for(const auto& pr:companies){
        if(pr.first==cid){
            Company *ptr=pr.second;
            QString encry_pwd=Encoder::Hash(ori_pwd,ptr->getSalt());
            if(ptr->getHashPwd()!=encry_pwd){
                err="公司密码错误！";
                return 0;
            }
            else{
                err="";
                return 1;
            }
        }
    }
    err="找不到指定公司！";
    return 0;
}
