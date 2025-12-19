#ifndef COMPANYREGISTERDIALOG_H
#define COMPANYREGISTERDIALOG_H

#include <QDialog>
#include "../controller/controllercompany.h"

namespace Ui {
class CompanyRegisterDialog;
}

class CompanyRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompanyRegisterDialog(QWidget *parent = nullptr,ControllerCompany *_controllercompany=nullptr);
    ~CompanyRegisterDialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CompanyRegisterDialog *ui;
    ControllerCompany *controllercompany;
};

#endif // COMPANYREGISTERDIALOG_H
