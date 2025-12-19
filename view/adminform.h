#ifndef ADMINFORM_H
#define ADMINFORM_H
#include <QWidget>
#include <QStandardItemModel>
#include "../model/flightmodel.h"
#include "../model/flightfilterproxymodel.h"
#include "../entity/plane.h"
#include "../entity/record.h"
#include "../entity/flight.h"
#include "../controller/controllerplane.h"

namespace Ui {
class AdminForm;
}
class AdminForm : public QWidget
{
    Q_OBJECT

public:
    explicit AdminForm(QWidget *parent = nullptr, FlightModel *_flightmodel = nullptr);
    ~AdminForm();

private slots:
    void on_btnAddPlane_clicked();    // 添加飞机
    void on_btnDeletePlane_clicked(); // 删除选中飞机
    void on_tablePlane_clicked(const QModelIndex &index); // 选中行变化

    void on_refreshFlight_clicked();

private:
    Ui::AdminForm *ui;
    int m_companyID;                  // 所属公司ID
    QStandardItemModel *m_planeModel;  // 飞机表格模型
    QStandardItemModel *m_recordModel; // 日志表格模型
    QStandardItemModel *m_flightModel; // 航班表格模型
    FlightModel *flightmodel;//和flightwidget的是同一个
    FlightFilterProxyModel *flightproxymodel;//属于adminform的proxymodel，以公司为筛选条件
    ControllerPlane *controllerplane;
    // 初始化模型
    void initModels();
    // 加载数据
    void loadCompanyInfo();
    void loadPlaneData();
    void loadRecordData();
    void loadFlightData();
    // 更新表格
    void UpdatePlaneList(const std::vector<Plane>& planes);
    void UpdateRecordList(const QList<Record>& recordList);
    void UpdateFlightList(const QList<Flight>& flightList);
    // 辅助：获取选中行的飞机ID
    int getSelectedPlaneID();
};
#endif // ADMINFORM_H
