#include "controllerflight.h"
#include "controllerflight.h"
#include"../tool/recordtool.h"
#include"../entity/admin.h"
ControllerFlight::ControllerFlight(QObject* parent,FlightModel *model)
    : QObject(parent),flightmodel(model){}

bool ControllerFlight::AddFlight(Flight& flight,QString& err){
    if(flight.getDuration()<=0){
        err="到达时间应晚于出发时间！";
        return 0;
    }
    if(!flightmodel->CheckAddConflict(flight,err)) return 0;
    if(!flightmodel->AppendRow(flight,err)) return 0;


    // 记录日志
    int newFlightId=flightmodel->getLastInsertedFlightId(err);
    extern User* curUser;
    Admin* admin = dynamic_cast<Admin*>(curUser);
    RecordTool::logAdminOperation(
        admin,
        OpType::ADD,
        TargetType::FLIGHT,
        newFlightId,
        QString("%1号管理员添加了航班%2").arg(admin->getAdminID()).arg(newFlightId)
        );
    err="";
    return 1;
}
bool ControllerFlight::RemoveFlight(int row,QString& err){
    Flight flight;
    if(!flightmodel->SelectByRow(row,flight,err)) return 0;
    int delFlightId=flight.getFlightID();
    //在删除操作前获得
    if(!flightmodel->RemoveRow(row,err)) return 0;

    // 记录日志
    extern User* curUser;
    Admin* admin = dynamic_cast<Admin*>(curUser);

    RecordTool::logAdminOperation(
        admin,
        OpType::DELETE,
        TargetType::FLIGHT,
        delFlightId,
        QString("%1号管理员删除了航班%2").arg(admin->getAdminID()).arg(delFlightId)
        );

    err="";
    return 1;
}
bool ControllerFlight::ModifyFlight(int row,Flight& flight,QString& err){
    if(flight.getDuration()<=0){
        err="到达时间应晚于出发时间！";
        return 0;
    }

    if(!flightmodel->CheckUpdConflict(row,flight,err)) return 0;
    if(!flightmodel->ModifyRow(row,flight,err)) return 0;

    // 记录日志
    extern User* curUser;
    Admin* admin = dynamic_cast<Admin*>(curUser);

    RecordTool::logAdminOperation(
        admin,
        OpType::MODIFY,
        TargetType::FLIGHT,
        flight.getFlightID(),
        QString("%1号管理员修改了航班%2").arg(admin->getAdminID()).arg(flight.getFlightID())
        );
    err="";
    return 1;
}
bool ControllerFlight::BadWeather(const QDateTime& t1,const QDateTime& t2,QString& err){
    double dur=t1.secsTo(t2)/3600.00;
    if(dur<=0){
        err="开始时间应晚于结束时间！";
        return 0;
    }
    if(!flightmodel->AddWeatherWarning(t1,t2,err)) return 0;


    err="";
    emit TableFlightsModified();
    return 1;
}
bool ControllerFlight::GoodWeather(const QDateTime& t1,const QDateTime& t2,QString& err){
    double dur=t1.secsTo(t2)/3600.00;
    if(dur<=0){
        err="开始时间应晚于结束时间！";
        return 0;
    }
    if(!flightmodel->DelWeatherWarning(t1,t2,err)) return 0;
    err="";
    emit TableFlightsModified();
    return 1;
}
bool ControllerFlight::BatchModify(const std::vector<int>& rows,int cid,double deltahour,const QString& updmess,QString& err){
    // 批量修改前先收集要修改的航班ID
    std::vector<int> modifiedFlightIds;
    for(int row : rows){
        Flight flight;
        if(!flightmodel->SelectByRow(row, flight, err)){
            return 0;
        }
        if(flight.getCompanyID() == cid){ // 只记录目标公司的航班
            modifiedFlightIds.push_back(flight.getFlightID());
        }
    }

    if(!flightmodel->CompanyBatchModify(rows,cid,deltahour,updmess,err)) return 0;

    // 记录批量修改日志
    extern User* curUser;
    Admin* admin = dynamic_cast<Admin*>(curUser);
    for(int flightId : modifiedFlightIds){
        RecordTool::logAdminOperation(
            admin,
            OpType::MODIFY,
            TargetType::FLIGHT,
            flightId,
            QString("%1号管理员批量修改了航班%2").arg(admin->getAdminID()).arg(flightId)
            );
    }

    err="";
    emit TableFlightsModified();
    return 1;
}
bool ControllerFlight::BatchDelete(const std::vector<int>& rows,int cid,QString& err){
    // 批量删除前先收集要删除的航班ID
    std::vector<int> deletedFlightIds;
    for(int row : rows){
        Flight flight;
        if(!flightmodel->SelectByRow(row, flight, err)){
            return 0;
        }
        if(flight.getCompanyID() == cid){ // 只记录目标公司的航班
            deletedFlightIds.push_back(flight.getFlightID());
        }
    }

    if(!flightmodel->CompanyBatchDelete(rows,cid,err)) return 0;

    // 记录批量删除日志
    extern User* curUser;
    Admin* admin = dynamic_cast<Admin*>(curUser);
    for(int flightId : deletedFlightIds){
        RecordTool::logAdminOperation(
            admin,
            OpType::DELETE,
            TargetType::FLIGHT,
            flightId,
            QString("%1号管理员批量删除了航班%2").arg(admin->getAdminID()).arg(flightId)
            );
    }

    err="";
    emit TableFlightsModified();
    return 1;
}
