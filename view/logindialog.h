#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "../controller/controlleruser.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr,ControllerUser *_controlleruser=nullptr);
    ~LoginDialog();

private slots:
    void on_btnLogin_clicked();

    void on_btnRegister_clicked();

private:
    Ui::LoginDialog *ui;
    ControllerUser *controlleruser;
};

#endif // LOGINDIALOG_H
