#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "../controller/controlleruser.h"
#include "../controller/controllercompany.h"
#include "../model/flightmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void ActivateLoginDialog();
    void updateUserStatus();
    void ActivateUserForm();
    void ActivateCompanyRegisterDialog();
private:
    Ui::MainWindow *ui;
    QLabel* lblUsr;
    ControllerUser *controlleruser;
    ControllerCompany *controllercompany;

    FlightModel *flightmodel;

    QTimer *idleTimer;
    const int timeoutMs = 60 * 60 * 1000;
};
#endif // MAINWINDOW_H
