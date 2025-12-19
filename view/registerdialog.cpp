#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "../entity/company.h"
extern std::map<int,Company*> companies;


RegisterDialog::RegisterDialog(QWidget *parent,ControllerUser *_controlleruser)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , controlleruser(_controlleruser)
{
    ui->setupUi(this);
    for(const auto& pr:companies){
        ui->cbxCompanyName->addItem(pr.second->getCompanyName(),QVariant(pr.second->getCompanyID()));
    }
    ui->labelCompanyName->hide();
    ui->cbxCompanyName->hide();
    ui->labelCompanyPwd->hide();
    ui->txtCompanyPwd->hide();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_cbxUserType_currentIndexChanged(int index)
{
    if(ui->cbxUserType->currentIndex()==0){
        ui->labelCompanyName->hide();
        ui->cbxCompanyName->hide();
        ui->labelCompanyPwd->hide();
        ui->txtCompanyPwd->hide();
    }
    else if(ui->cbxUserType->currentIndex()==1){
        ui->labelCompanyName->show();
        ui->cbxCompanyName->show();
        ui->labelCompanyPwd->show();
        ui->txtCompanyPwd->show();
    }
}


void RegisterDialog::on_pushButton_clicked()
{
    QString err;
    if(ui->cbxUserType->currentIndex()==0){
        if(!controlleruser->RegisterUser(ui->txtUserName->text().trimmed(),ui->txtPassword->text().trimmed(),ui->txtRePassword->text().trimmed(),err)){
            QMessageBox::warning(this, "警告", err);
        }
        else QMessageBox::information(this, "提示", "注册成功！");
    }
    else if(ui->cbxUserType->currentIndex()==1){
        int cidx=ui->cbxCompanyName->currentIndex();
        if(cidx==-1){
            QMessageBox::warning(this, "警告", "航司不能为空！");
        }
        int cid=ui->cbxCompanyName->itemData(cidx).toInt();
        if(!controlleruser->RegisterUser(ui->txtUserName->text().trimmed(),ui->txtPassword->text().trimmed(),ui->txtRePassword->text().trimmed(),cid,ui->txtCompanyPwd->text().trimmed(),err)){
            QMessageBox::warning(this, "警告", err);
        }
        else QMessageBox::information(this, "提示", "注册成功！");
    }
}

