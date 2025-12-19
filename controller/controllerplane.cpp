#include "controllerplane.h"
#include "../entity/admin.h"
#include "../entity/company.h"
#include "../model/planemodel.h"
#include "../tool/recordtool.h"
extern User* curUser;
extern std::map<int,Company*> companies;

ControllerPlane::ControllerPlane(QObject* parent)
    :QObject(parent){}

bool ControllerPlane::AddPlane(const QString& planeType,int cid,QString& err){
    Admin* admin = dynamic_cast<Admin*>(curUser);
    if (!admin) {
        err="权限错误，仅管理员可添加飞机！";
        return 0;
    }

    int seatCnt = 0;
    if (planeType == "窄体机") {
        seatCnt = 150;
    } else if (planeType == "宽体机") {
        seatCnt = 350;
    } else {
        err="输入错误，无效机型！";
        return 0;
    }

    QString companyAbbr = "UNKNOWN";
    QString companyName=companies[cid]->getCompanyName();

    if (companyName.contains("南方航空")) companyAbbr = "CSN";
    else if (companyName.contains("东方航空")) companyAbbr = "CES";
    else if (companyName.contains("中国国航")) companyAbbr = "CCA";
    else if (companyName.contains("海南航空")) companyAbbr = "CHH";
    else {
        companyAbbr = companyName.length() >= 3 ?
                          companyName.left(3).toUpper() : companyName.toUpper();
    }

    int lastPlaneId = 0;
    QString qerr;
    if (!PlaneModel::GetMaxPlaneID(lastPlaneId, qerr)) {//获取最大飞机ID
        err="查询失败，获取最大飞机ID失败：" + qerr;
        return 0;
    }
    int newPlaneId = lastPlaneId + 1;

    QString planeName = QString("%1%2").arg(companyAbbr).arg(newPlaneId, 4, 10, QChar('0'));//生成飞机名称

    int pid=PlaneModel::InsertPlane(planeName, planeType, seatCnt, cid, qerr);//调用Model添加飞机
    if (pid==-1) {
        err= "添加失败，飞机添加失败：" + qerr;
        return 0;
    }

    companies[cid]->Planes.push_back(Plane(pid,planeName,planeType,seatCnt));

    RecordTool::logAdminOperation(
        admin,
        OpType::ADD,
        TargetType::PLANE,
        newPlaneId,
        QString("%1号管理员添加了%2飞机「%3」（座位数：%4）")
            .arg(admin->getAdminID()).arg(planeType).arg(planeName).arg(seatCnt)
        );// 记录操作日志

    err=QString("飞机添加成功！\n飞机名称：%1\n机型：%2\n座位数：%3")
              .arg(planeName).arg(planeType).arg(seatCnt);
    return 1;
}
bool ControllerPlane::DeletePlane(int pid,const QString& pname,int cid,QString& err){
    Admin* admin = dynamic_cast<Admin*>(curUser);
    if (!admin) {
        err="权限错误，仅管理员可添加飞机！";
        return 0;
    }

    int idx=-1;
    for(int i=0;i<companies[cid]->Planes.size();i++){
        if(companies[cid]->Planes[i].getPlaneID()==pid){idx=i;break;}
    }
    if(idx==-1){
        err = "飞机不存在或已被删除";
        return 0;
    }
    companies[cid]->Planes.erase(companies[cid]->Planes.begin()+idx);

    QString qerr;
    if (!PlaneModel::DeletePlane(pid, qerr)) {// 调用Model删除飞机
        err="飞机删除失败：" + qerr;
        return 0;
    }

    RecordTool::logAdminOperation(
        admin,
        OpType::DELETE,
        TargetType::PLANE,
        pid,
        QString("%1号管理员删除了飞机「%2」（ID：%3）").arg(admin->getAdminID()).arg(pname).arg(pid)
        );// 记录操作日志
    return 1;
}
