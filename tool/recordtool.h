#ifndef RECORDTOOL_H
#define RECORDTOOL_H
#include <QString>
#include "../entity/record.h"
#include "../entity/admin.h"

namespace OpType {// 操作类型
const QString ADD = "添加";
const QString DELETE = "删除";
const QString MODIFY = "修改";
}

namespace TargetType {// 操作对象类型
const QString FOOD = "餐食";
const QString FLIGHT = "航班";
const QString USER = "用户";
const QString COMPANY = "公司";
const QString PLANE = "飞机";
}

class RecordTool
{
public:
    static bool logAdminOperation(const Record& record);//记录Record
    static bool logAdminOperation(Admin* admin, const QString& opType,
                                  const QString& targetType, int targetID,
                                  const QString& opDesc);//通过参数构建Record并记录
    static bool GetRecordListByCompanyID(int cid, QList<Record>& records, QString& err);//获取公司的操作记录
private:
    RecordTool() = default;
    ~RecordTool() = default;
    static bool insertRecordToDB(const Record& record);//内部实现：执行数据库插入操作
};

#endif // RECORDTOOL_H
