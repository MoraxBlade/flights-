#ifndef CONTROLLERUSER_H
#define CONTROLLERUSER_H
#include <QObject>
#include <QString>
#include "controllercompany.h"

class ControllerUser:public QObject
{
    Q_OBJECT
public:
    explicit ControllerUser(QObject* parent = nullptr, ControllerCompany *_controllercompany = nullptr);
    bool RegisterUser(const QString& name,const QString& ori_pwd,const QString& reori_pwd,QString& err);
    bool RegisterUser(const QString& name,const QString& ori_pwd,const QString& reori_pwd,const int& cid,const QString& cpwd,QString& err);
    bool LoginUser(const QString& name,const QString& ori_pwd, QString& err);
    void LogoutUser();
    bool ChangePassword(const QString& name,const QString& ori_pwd, QString& err);
signals:
    void clientLoggedIn();
    void adminLoggedIn();
    void userLoggedOut();
private:
    ControllerCompany *controllercompany;
};
#endif // CONTROLLERUSER_H
