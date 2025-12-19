#include "foodmodel.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlField>  // 新增：包含QSqlField类定义
#include <QSqlRecord> // 新增：包含QSqlRecord类定义
FoodModel::FoodModel(QObject *parent, FlightModel *flightModel)
    :QSqlTableModel(parent),m_flightModel(flightModel){}

void FoodModel::init(){
    setTable("foods");
    setFilter("");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setHeaderData(0, Qt::Horizontal, "餐食ID");
    setHeaderData(1, Qt::Horizontal, "航班ID");
    setHeaderData(2, Qt::Horizontal, "餐食名称");
    select();
}
/*
QVariant FoodModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return "餐食名称"; // 列名显示为「餐食名称」
    }
    return QVariant();
}*/
/*
QVariant FoodModel::data(const QModelIndex &index, int role)const{
    if (!index.isValid() || index.row() >= this->rowCount()) {
        return QVariant();
    }
    // 仅处理「显示角色」，且只返回第2列（FoodName）的数据
    if (role == Qt::DisplayRole && index.column() == 0) {
        // 定位到模型中当前行的FoodName列（数据库表的第2列）
        QModelIndex foodNameIdx = this->index(index.row(), 2);
        QString foodName = QSqlTableModel::data(foodNameIdx, Qt::DisplayRole).toString().trimmed();
        return foodName;
    }
    return QSqlTableModel::data(index, role);
}
*/


void FoodModel::clearQuery() {// 清空查询结果（视图显示空列表）
    setFilter("1=0"); // 筛选条件恒假，无数据显示
    select();
}

bool FoodModel::checkFoodDuplicate(int flightId, const QString &foodName, QString &err)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT FoodID FROM foods WHERE FlightID = :flightId AND FoodName = :foodName");
    query.bindValue(":flightId", flightId);
    query.bindValue(":foodName", foodName.trimmed());
    if (!query.exec()) {
        err = "校验重复餐食失败：" + query.lastError().text();
        return false;
    }
    if (query.next()) {
        err = "餐食已存在";
        return false;
    }
    return true;
}

bool FoodModel::getFoodListByFlightId(int flightId, QList<Food>& foodList, QString& err)
{
    foodList.clear();
    if (flightId <= 0) {
        err = "无效的航班ID";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    const QString sql = "SELECT FoodID, FoodName FROM foods WHERE FlightID = :flightId";
    query.prepare(sql);
    query.bindValue(":flightId", flightId);
    if (!query.exec()) {
        err = "查询餐食失败：" + query.lastError().text();
        return false;
    }
    while (query.next()) {
        Food food;
        food.setFoodId(query.value("FoodID").toInt());
        food.setFlightId(flightId);
        food.setFoodName(query.value("FoodName").toString().trimmed());
        foodList.append(food);
    }
    return true;
}

bool FoodModel::getFoodIdByFlightNameAndFoodName(const QString& flightName, const QString& foodName, int& foodId, QString& err){

    if (!m_flightModel) {
        err = "航班模型未初始化，无法查询";
        return false;
    }
    QString trimmedFlightName = flightName.trimmed();
    QString trimmedFoodName = foodName.trimmed();
    if (trimmedFlightName.isEmpty()) {
        clearQuery();
        err ="错误，航班号不可为空";
        return false;
    }
    if(trimmedFoodName.isEmpty()){
        clearQuery();
        err = "错误：食品名称不能为空";
        return false;
    }

    Flight flight;
    if(!m_flightModel->getFlightByFlightName(trimmedFlightName, flight, err)){
        clearQuery();
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    QString sql = "SELECT FoodID FROM foods WHERE FlightID = :flightId AND FoodName = :foodName";
    query.prepare(sql);
    query.bindValue(":flightId", flight.getFlightID());
    query.bindValue(":foodName", trimmedFoodName);
    if (!query.exec()) {
        clearQuery();
        err = QString("错误：查询食品失败：%1").arg(query.lastError().text());
        return false;
    }
    if (!query.next()) {
        clearQuery();
        err = QString("航班「%1」下无餐食「%2」").arg(flightName).arg(trimmedFoodName);
        return false;
    }
    QVariant foodIdVar = query.value("FoodID");
    if (foodIdVar.isNull() || !foodIdVar.canConvert<int>()) {
        clearQuery();
        err = QString("餐食ID无效：%1").arg(foodIdVar.toString());
        return false;
    }

    foodId = foodIdVar.toInt();
    err.clear(); // 清空错误信息
    return true;
}
bool FoodModel::insertFood(const Food &food, QString &err){
    if(food.getFlightId()<=0||food.getFoodName().trimmed().isEmpty()){
        err="航班号无效或餐食名称为空";
        return false;
    }
    int newRow=rowCount();
    if(!insertRow(newRow)){
        err = "插入失败："+lastError().text();
        return false;
    }
    setData(index(newRow,1),food.getFlightId());
    setData(index(newRow,2),food.getFoodName().trimmed());
    if(!submitAll()){
        err="保存失败："+lastError().text();
        revertAll();
        return false;
    }
    select(); // 刷新数据
    err.clear();
    return true;
}
bool FoodModel::deleteFood(int foodId,QString &err){
    if(foodId<=0){
        err = "无效的餐食ID";
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM foods WHERE FoodID = :foodId");
    query.bindValue(":foodId", foodId);
    if (!query.exec()) {
        err = "删除餐食失败：" + query.lastError().text();
        return false;
    }
    select();// 刷新模型数据
    err.clear();
    return true;
}
bool FoodModel::loadFoodsByFlightName(const QString& flightName, QString& err)
{
    if (!m_flightModel) {
        err = "航班模型未初始化";
        return false;
    }
    QString trimmedName = flightName.trimmed();
    if (trimmedName.isEmpty()) {
        clearQuery(); // 航班号为空时清空列表
        return true;
    }
    Flight flight;
    if (!m_flightModel->getFlightByFlightName(trimmedName, flight, err)) {
        clearQuery();
        return false;
    }
    setFilter(QString("FlightID = %1").arg(flight.getFlightID()));
    if (!select()) {
        err = "加载餐食失败：" + lastError().text();
        setFilter("");
        return false;
    }
    err.clear();
    return true;
}
int FoodModel::getLastInsertedFoodId(QString &err) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    if (query.exec("SELECT LAST_INSERT_ID()")) {//获取最后一次插入的自增ID
        if (query.next()) {
            return query.value(0).toInt(); // 返回新插入的FoodID
        }
    }
    err = "获取新餐食ID失败：" + query.lastError().text();
    return -1;
}
