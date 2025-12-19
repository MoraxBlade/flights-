#include <QMessageBox>
#include "logindialog.h"
#include "ui_logindialog.h"
#include "registerdialog.h"

LoginDialog::LoginDialog(QWidget *parent, ControllerUser *_controlleruser)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , controlleruser(_controlleruser)
{
    ui->setupUi(this);
    ui->txtPassword->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_btnLogin_clicked()
{
    QString err;
    if(!controlleruser->LoginUser(ui->txtUserName->text().trimmed(),ui->txtPassword->text().trimmed(),err)){
        QMessageBox::warning(this, "警告", err);
    }
    else{
        QMessageBox::information(this, "提示", "登录成功！");
        accept();
    }
}


void LoginDialog::on_btnRegister_clicked()
{
    RegisterDialog registerdialog(this,controlleruser);
    registerdialog.exec();
}

