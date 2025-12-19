#ifndef PLANEMODEL_H
#define PLANEMODEL_H
#include <QString>
#include "../entity/plane.h"
class PlaneModel
{
public:
    static bool GetPlanesByCompanyID(int cid,std::vector<Plane>& planes,QString& err);
    static int InsertPlane(const QString& planeName, const QString& planeType,
                           int seatCnt, int companyID, QString& err);//添加飞机，返回PlaneID
    static bool DeletePlane(int planeID, QString& err);// 删除飞机
    static bool GetMaxPlaneID(int& maxID, QString& err);// 获取飞机表最大ID（用于生成新飞机名称）
};

#endif // PLANEMODEL_H
