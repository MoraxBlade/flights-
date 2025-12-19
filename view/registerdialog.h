#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "../controller/controlleruser.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr,ControllerUser *_controlleruser=nullptr);
    ~RegisterDialog();

private slots:
    void on_cbxUserType_currentIndexChanged(int index);

    void on_pushButton_clicked();

private:
    Ui::RegisterDialog *ui;
    ControllerUser *controlleruser;
};

#endif // REGISTERDIALOG_H
