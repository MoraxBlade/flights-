#ifndef PASSWORDCHANGE_H
#define PASSWORDCHANGE_H

#include <QWidget>

namespace Ui {
class Passwordchange;
}

class ControllerUser;  // 前向声明

class Passwordchange : public QWidget
{
    Q_OBJECT

public:
    explicit Passwordchange(QWidget *parent = nullptr, ControllerUser *_controlleruser = nullptr);
    ~Passwordchange();

signals:
    void passwordChanged(bool success);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Passwordchange *ui;
    ControllerUser *controlleruser;
};

#endif // PASSWORDCHANGE_H
