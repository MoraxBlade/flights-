#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "logindialog.h"
#include "companyregisterdialog.h"
#include "flightwidget.h"
#include "adminform.h"
#include "../entity/admin.h"
#include "../entity/user.h"
#include"personalpage.h"
#include <QMessageBox>
#include <QVBoxLayout>
extern User *curUser;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QWidget* central=this->centralWidget();
    QVBoxLayout* mainlayout=new QVBoxLayout(central);
    central->setLayout(mainlayout);

    // 创建flightmodel连接所有航班的数据表
    flightmodel = new FlightModel(this);
    flightmodel->init();

    //主页面放航班界面
    FlightWidget* flightWidget = new FlightWidget(central, flightmodel);
    mainlayout->addWidget(flightWidget);

    // 创建一个controllercompany用于注册、注销公司
    controllercompany=new ControllerCompany(this);
    //注册、注销公司与flightwidget的公司combobox更新槽函数连接
    connect(controllercompany,&ControllerCompany::companyRegistered,flightWidget,&FlightWidget::RefreshCompanies);
    // 创建一个controlleruser用于管理注册、登录、登出
    controlleruser=new ControllerUser(this,controllercompany);
    //登录、登出信号与登录状态显示槽函数连接
    connect(controlleruser,&ControllerUser::adminLoggedIn,this,&::MainWindow::updateUserStatus);
    connect(controlleruser,&ControllerUser::clientLoggedIn,this,&::MainWindow::updateUserStatus);
    connect(controlleruser,&ControllerUser::userLoggedOut,this,&MainWindow::updateUserStatus);
    //登录、登出信号与flightwidget的模式切换槽函数连接
    connect(controlleruser,&ControllerUser::adminLoggedIn,flightWidget,&FlightWidget::SetAdminMode);
    connect(controlleruser,&ControllerUser::clientLoggedIn,flightWidget,&FlightWidget::SetClientMode);
    connect(controlleruser,&ControllerUser::userLoggedOut,flightWidget,&FlightWidget::SetTouristMode);
    //状态栏
    lblUsr = new QLabel("游客",this);
    statusBar()->addPermanentWidget(lblUsr);
    //菜单栏
    connect(ui->actionLoginUser,&QAction::triggered,this,&MainWindow::ActivateLoginDialog);
    connect(ui->actionLogoutUser,&QAction::triggered,controlleruser,&ControllerUser::LogoutUser);
    connect(ui->actionHomePage,&QAction::triggered,this,&MainWindow::ActivateUserForm);
    connect(ui->actionRegisterCompany,&QAction::triggered,this,&MainWindow::ActivateCompanyRegisterDialog);
    // 创建用户空闲计时器
    idleTimer=new QTimer(this);//以MainWindow为父对象，父对象被销毁时自动delete
    idleTimer->setInterval(timeoutMs);
    idleTimer->setSingleShot(true);
    connect(idleTimer,&QTimer::timeout,controlleruser,&ControllerUser::LogoutUser);
    // 安装全局事件过滤器，捕获鼠标和键盘操作
    qApp->installEventFilter(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type()==QEvent::MouseMove||event->type()==QEvent::KeyPress||event->type()==QEvent::MouseButtonPress){
        idleTimer->start(); // 用户有操作，重置计时器
    }
    return QMainWindow::eventFilter(watched,event);
}
void MainWindow::ActivateCompanyRegisterDialog(){
    CompanyRegisterDialog cpyregisterdialog(this,controllercompany);
    cpyregisterdialog.exec();
}
void MainWindow::ActivateLoginDialog(){
    LoginDialog logindialog(this,controlleruser);
    if(logindialog.exec()==QDialog::Accepted){
        idleTimer->start();// 登录成功，启动空闲计时器
    }
}
void MainWindow::updateUserStatus(){
    if(curUser==nullptr) lblUsr->setText("游客");
    else if(curUser->getUserType()=="client") lblUsr->setText("客户: "+curUser->getUserName());
    else if(curUser->getUserType()=="admin") lblUsr->setText("管理员: "+curUser->getUserName());
}
void MainWindow::ActivateUserForm(){
    if (curUser == nullptr) {
        QMessageBox::information(this, "提示", "请先登录以访问个人主页！");
        ActivateLoginDialog();  // 自动打开登录对话框
        return;
    }
    // 创建并显示PersonalPage
    if(curUser->getUserType()=="client"){
        personalpage *personalPage = new personalpage(this);
        personalPage->setWindowFlags(Qt::Window);
        personalPage->setWindowTitle("个人主页 - " + curUser->getUserName());
        personalPage->show();
    }
    if(curUser->getUserType()=="admin"){
        extern User* curUser;
        Admin* admin = dynamic_cast<Admin*>(curUser);
        int companyId=admin->getCompanyID();
        AdminForm *w=new AdminForm(this,flightmodel);
        w->setWindowFlags(Qt::Window);
        w->resize(600, 400);
        w->show();
    }
}
