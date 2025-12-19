#ifndef ADDFOODWINDOW_H
#define ADDFOODWINDOW_H
#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include "../entity/food.h"

namespace Ui {
class AddFoodWindow; // 仅声明UI类
}

// 类定义在全局命名空间，不嵌套在Ui内
class AddFoodWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AddFoodWindow(QWidget *parent = nullptr);
    ~AddFoodWindow();

signals:
    void signalAddFood(const QString& flightName, const QString& foodName);
    void signalDeleteFood(const QString& flightName, const QString& foodName);
    void signalQueryFood(const QString& flightName);

public slots:
    void slotUpdateFoodList(const QList<Food>& foodList);
    void slotShowResult(const QString& msg, bool isSuccess);

private slots:
    void on_addFood_clicked();
    void on_deleteFood_clicked();
    void on_txtFlightID_returnPressed();

private:
    Ui::AddFoodWindow *ui; // 指向UI实例
    void clearInputs();
    QStandardItemModel *m_foodTableModel;
};

#endif // ADDFOODWINDOW_H
