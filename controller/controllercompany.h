#ifndef CONTROLLERCOMPANY_H
#define CONTROLLERCOMPANY_H
#include <QObject>
#include <QString>

class ControllerCompany:public QObject
{
    Q_OBJECT
public:
    explicit ControllerCompany(QObject* parent = nullptr);
    bool RegisterCompany(const QString& cname,const QString& ori_pwd,const QString& reori_pwd,const QString& pubkey,QString& err);
    bool Check(const int& cid,const QString& ori_pwd,QString& err);
signals:
    void companyRegistered();
};

#endif // CONTROLLERCOMPANY_H
