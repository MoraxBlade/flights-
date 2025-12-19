#include "record.h"

Record::Record()
    : m_id(-1), m_adminID(-1), m_companyID(-1), m_targetID(-1)
{
}

Record::Record(int adminID, int companyID, const QString& opType,
               const QString& targetType, int targetID, const QString& opDesc,
               const QDateTime& opTime)
    : m_adminID(adminID),
    m_companyID(companyID),
    m_operationType(opType),
    m_targetType(targetType),
    m_targetID(targetID),
    m_operationDesc(opDesc),
    m_operationTime(opTime)
{}

// Getter实现
int Record::getId() const { return m_id; }
int Record::getAdminID() const { return m_adminID; }
int Record::getCompanyID() const { return m_companyID; }
QString Record::getOperationType() const { return m_operationType; }
QString Record::getTargetType() const { return m_targetType; }
int Record::getTargetID() const { return m_targetID; }
QString Record::getOperationDesc() const { return m_operationDesc; }
QDateTime Record::getOperationTime() const { return m_operationTime; }

// Setter实现
void Record::setId(int id) { m_id = id; }
void Record::setAdminID(int adminID) { m_adminID = adminID; }
void Record::setCompanyID(int companyID) { m_companyID = companyID; }
void Record::setOperationType(const QString& opType) { m_operationType = opType; }
void Record::setTargetType(const QString& targetType) { m_targetType = targetType; }
void Record::setTargetID(int targetID) { m_targetID = targetID; }
void Record::setOperationDesc(const QString& opDesc) { m_operationDesc = opDesc; }
void Record::setOperationTime(const QDateTime& opTime) { m_operationTime = opTime; }
