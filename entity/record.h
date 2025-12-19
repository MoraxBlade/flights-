#ifndef RECORD_H
#define RECORD_H

#include <QDateTime>
#include <QString>

class Record
{
private:
    int m_id;                  // 记录ID（自增主键）
    int m_adminID;             // 管理员ID
    int m_companyID;           // 公司ID
    QString m_operationType;   // 操作类型（新增/删除/修改/查询）
    QString m_targetType;      // 操作对象（餐食/航班/用户/公司）
    int m_targetID;            // 操作对象ID
    QString m_operationDesc;   // 操作描述
    QDateTime m_operationTime; // 操作时间

public:
    // 构造函数
    Record();
    Record(int adminID, int companyID, const QString& opType,
           const QString& targetType, int targetID, const QString& opDesc,
           const QDateTime& opTime = QDateTime::currentDateTime());

    // Getter方法
    int getId() const;
    int getAdminID() const;
    int getCompanyID() const;
    QString getOperationType() const;
    QString getTargetType() const;
    int getTargetID() const;
    QString getOperationDesc() const;
    QDateTime getOperationTime() const;

    // Setter方法
    void setId(int id);
    void setAdminID(int adminID);
    void setCompanyID(int companyID);
    void setOperationType(const QString& opType);
    void setTargetType(const QString& targetType);
    void setTargetID(int targetID);
    void setOperationDesc(const QString& opDesc);
    void setOperationTime(const QDateTime& opTime);
};

#endif // RECORD_H
