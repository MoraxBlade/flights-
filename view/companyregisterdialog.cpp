#include <QMessageBox>
#include "companyregisterdialog.h"
#include "ui_companyregisterdialog.h"
#include "../controller/controllercompany.h"

CompanyRegisterDialog::CompanyRegisterDialog(QWidget *parent,ControllerCompany *_controllercompany)
    : QDialog(parent)
    ,controllercompany(_controllercompany)
    , ui(new Ui::CompanyRegisterDialog)
{
    ui->setupUi(this);
}

CompanyRegisterDialog::~CompanyRegisterDialog()
{
    delete ui;
}

void CompanyRegisterDialog::on_pushButton_clicked()
{
    QString err;
    if(!controllercompany->RegisterCompany(ui->txtCompanyName->text().trimmed(),ui->txtPassword->text().trimmed(),ui->txtRePassword->text().trimmed(),ui->txtPubkey->text().trimmed(),err)){
        QMessageBox::warning(this, "警告", err);
    }
    else QMessageBox::information(this, "提示", "公司注册成功！");
}
