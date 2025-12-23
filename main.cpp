#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "view/mainwindow.h"
#include "model/companymodel.h"
int main(int argc, char *argv[])
{
QApplication a(argc, argv);
qDebug() << QSqlDatabase::drivers();
//连接数据库
QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
QString connectionString = QString("DRIVER={MySQL ODBC 9.5 Unicode Driver};"
"SERVER=127.0.0.1;"
"PORT=33306;"
"DATABASE=ticketsys;"
"USER=ticket;"
"PASSWORD=@Ticket123;");
db.setDatabaseName(connectionString); // 重要：这里设置的是整个连接字符串！
if(!db.open()){
qDebug()<<"数据库连接失败："<<db.lastError().text();
return -1;
}
//载入现有的全部航司
CompanyModel::LoadCompanies();
MainWindow w;
w.show();
int ret=a.exec();
//释放航司对象
CompanyModel::ReleaseCompanies();
//关闭数据库连接
db.close();
return ret;
}