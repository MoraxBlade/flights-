#include "passwordchange.h"
#include "ui_passwordchange.h"
#include "../controller/controlleruser.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QLineEdit>
#include <QDialog>

Passwordchange::Passwordchange(QWidget *parent, ControllerUser *_controlleruser)
    : QWidget(parent)
    , ui(new Ui::Passwordchange)
    , controlleruser(_controlleruser)  // 初始化成员变量
{
    ui->setupUi(this);
    ui->lineEdit->setEchoMode(QLineEdit::Password);
    ui->lineEdit_2->setEchoMode(QLineEdit::Password);
    ui->lineEdit_3->setEchoMode(QLineEdit::Password);
}

Passwordchange::~Passwordchange()
{
    delete ui;
}

void Passwordchange::on_pushButton_clicked()
{
    // 获取输入
    QString oldPassword = ui->lineEdit->text().trimmed();
    QString newPassword = ui->lineEdit_2->text().trimmed();
    QString confirmPassword = ui->lineEdit_3->text().trimmed();

    QString err;
    if(confirmPassword!=newPassword){
        QMessageBox::warning(this, "警告", "两次输入密码不同！");
        return;
    }
    if(!controlleruser->ChangePassword(oldPassword,newPassword,err)){
        QMessageBox::warning(this, "警告", err);
    }
    else{
        QMessageBox::information(this, "提示", "密码修改成功！");
        this->close();
    }
}

