#ifndef FLIGHTWIDGET_H
#define FLIGHTWIDGET_H

#include <QWidget>
#include <QDataWidgetMapper>
#include <QTimer>
#include "../model/flightmodel.h"
#include "../model/flightfilterproxymodel.h"
#include "../controller/controllerflight.h"
#include "../model/foodmodel.h"
#include "../model/ordermodel.h"  // 订单模型头文件

namespace Ui {
class FlightWidget;
}

class FlightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightWidget(QWidget *parent = nullptr, FlightModel *_flightmodel = nullptr);
    ~FlightWidget();

protected:
    void GetCompanies();
    void setAdminAccess();
public slots:
    void SetTouristMode();
    void SetAdminMode();
    void SetClientMode();
    void RefreshCompanies();
private slots:
    void refreshDatabase();
    void onFlightSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_cbxCompanyName_currentIndexChanged(int index);
    void on_cbxPlaneName_currentIndexChanged(int index);
    void on_btnAddFlight_clicked();
    void on_btnDeleteFlight_clicked();
    void on_btnModifyFlight_clicked();
    void on_btnSearch_clicked();
    void on_btnSearchClear_clicked();
    void on_btnWeather_clicked();
    void on_btnAddFood_clicked();
    void on_tableFlight_activated(const QModelIndex &index);
    void on_btnOrder_clicked();
    void loadFoodsToComboBox(int flightId);
    void on_cbxFoodName_currentIndexChanged(int index);
    void on_btnBatchModify_clicked();
    void on_btnBatchDelete_clicked();
private:
    Ui::FlightWidget *ui;
    FlightModel *flightmodel;
    FlightFilterProxyModel *flightproxymodel;
    ControllerFlight *controllerflight;
    OrderModel *ordermodel;  // 订单模型指针
    QDataWidgetMapper *flightmodelmapper;
    QTimer *refreshTimer;
    const int timerefreshMs = 60 * 1000; // 1分钟刷新一次
    int seatCnt = 0;
    int currentFoodId = 0;
};

#endif // FLIGHTWIDGET_H
