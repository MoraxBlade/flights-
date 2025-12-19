#include "adminform.h"
#include "ui_adminform.h"
#include <QMessageBox>
#include <QHeaderView>
#include "../model/flightmodel.h"
#include "../tool/recordtool.h"
#include "../entity/admin.h"
#include "../entity/company.h"

extern std::map<int, Company*> companies;
extern User* curUser;
extern std::map<QString,QString> cnameToccode;

AdminForm::AdminForm(QWidget *parent, FlightModel *_flightmodel)
    : QWidget(parent)
    , ui(new Ui::AdminForm)
    , flightmodel(_flightmodel)
    , m_planeModel(new QStandardItemModel(this))
    , m_recordModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    m_companyID=dynamic_cast<Admin *>(curUser)->getCompanyID();
    initModels();

    loadCompanyInfo();
    loadPlaneData();
    loadRecordData();

    // 初始化机型下拉框（仅窄体机、宽体机）
    ui->cbxPlaneType->addItems({"窄体机", "宽体机"});
    // 绑定选中行变化信号
    connect(ui->tablePlane, &QTableView::clicked, this, &AdminForm::on_tablePlane_clicked);
}
AdminForm::~AdminForm()
{
    delete ui;
}
// 初始化模型（设置表格列名和伸缩属性）
void AdminForm::initModels()
{
    // 飞机表格
    m_planeModel->setHorizontalHeaderLabels({"飞机ID", "飞机名称", "飞机类型", "座位数"});
    ui->tablePlane->setModel(m_planeModel);
    ui->tablePlane->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePlane->verticalHeader()->setVisible(false);
    // 创建controllerplane用于管理飞机
    controllerplane = new ControllerPlane(this);

    // 航班表格
    // 创建flightproxymodel
    flightproxymodel = new FlightFilterProxyModel(this);
    flightproxymodel->setSourceModel(flightmodel);
    flightproxymodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    flightproxymodel->setSortCaseSensitivity(Qt::CaseInsensitive);
    // 设置flightproxymodel筛选条件
    int cid=dynamic_cast<Admin *>(curUser)->getCompanyID();
    flightproxymodel->CompanyName = companies[cid]->getCompanyName();
    flightproxymodel->StartCity = "";
    flightproxymodel->EndCity = "";
    flightproxymodel->StartTimeFrom = QDateTime(QDate(2000, 1, 1), QTime(0, 0, 0));
    flightproxymodel->StartTimeTo = QDateTime(QDate(2050, 1, 1), QTime(23, 59, 59));
    flightproxymodel->PriceLowFrom = 00.00;
    flightproxymodel->PriceLowTo = 99999.99;
    flightproxymodel->refreshFilter();
    // tableFlight视图关联flightproxymodel
    ui->tableFlight->setModel(flightproxymodel);
    ui->tableFlight->setSortingEnabled(true);
    ui->tableFlight->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止在表格中直接编辑
    ui->tableFlight->setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选择
    ui->tableFlight->setSelectionMode(QAbstractItemView::ExtendedSelection); // 多选
    ui->tableFlight->setColumnHidden(0, true);
    ui->tableFlight->sortByColumn(0, Qt::AscendingOrder);

    // 日志表格
    m_recordModel->setHorizontalHeaderLabels({
        "记录ID", "管理员ID", "操作类型", "对象类型", "对象ID", "操作描述", "操作时间"
    });
    ui->tableRecords->setModel(m_recordModel);
    // 关键优化：列宽策略（优先保证操作描述完整）
    ui->tableRecords->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // 允许手动调整
    // 固定窄列宽度，节省空间
    ui->tableRecords->setColumnWidth(0, 80);  // 记录ID
    ui->tableRecords->setColumnWidth(1, 80);  // 管理员ID
    ui->tableRecords->setColumnWidth(2, 80);  // 操作类型
    ui->tableRecords->setColumnWidth(3, 80);  // 对象类型
    ui->tableRecords->setColumnWidth(4, 80);  // 对象ID
    ui->tableRecords->setColumnWidth(6, 180); // 操作时间（固定宽度）
    // 操作描述列：拉伸填充剩余空间，确保详情显示
    ui->tableRecords->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    // 启用文字换行（当内容过长时自动换行，避免截断）
    ui->tableRecords->setWordWrap(true);
    // 调整行高为自适应（根据内容高度自动调整）
    ui->tableRecords->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableRecords->verticalHeader()->setVisible(false);
}
// 加载公司信息（原有逻辑不变）
void AdminForm::loadCompanyInfo()
{
    if (companies.find(m_companyID) != companies.end()) {
        QString companyName = companies[m_companyID]->getCompanyName();
        ui->labelCompanyName->setText(
            QString("<html><head/><body><p><span style=\"font-size:14pt;\">所属公司：%1（ID：%2）</span></p></body></html>")
                .arg(companyName).arg(m_companyID)
            );
        this->setWindowTitle(QString("%1 - 管理员控制台").arg(companyName));
    } else {
        ui->labelCompanyName->setText(
            QString("<html><head/><body><p><span style=\"font-size:14pt;color:red;\">所属公司：未找到（ID：%1）</span></p></body></html>")
                .arg(m_companyID)
            );
    }
}

void AdminForm::loadPlaneData()// 加载飞机列表
{
    std::vector<Plane> planes=companies[m_companyID]->Planes;
    UpdatePlaneList(planes);
}

void AdminForm::loadRecordData()// 加载操作日志
{
    QList<Record> recordList;
    QString err;
    if (RecordTool::GetRecordListByCompanyID(m_companyID, recordList, err)) {
        UpdateRecordList(recordList);
    } else {
        QMessageBox::warning(this, "加载失败", "操作记录加载失败：" + err);
    }
}

void AdminForm::UpdatePlaneList(const std::vector<Plane>& planes)//更新飞机表格
{
    m_planeModel->clear();
    m_planeModel->setHorizontalHeaderLabels({"飞机ID", "飞机名称", "飞机类型", "座位数"});
    for (const Plane& plane : planes) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(plane.getPlaneID())));
        items.append(new QStandardItem(plane.getPlaneName()));
        items.append(new QStandardItem(plane.getPlaneType()));
        items.append(new QStandardItem(QString::number(plane.getSeatCnt())));
        for (QStandardItem* item : items) {
            item->setEditable(false);
        }
        m_planeModel->appendRow(items);
    }
}

void AdminForm::UpdateRecordList(const QList<Record>& recordList)// 更新日志表格
{
    m_recordModel->clear();
    m_recordModel->setHorizontalHeaderLabels({
        "记录ID", "管理员ID", "操作类型", "对象类型", "对象ID", "操作描述", "操作时间"
    });
    for (const Record& record : recordList) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(QString::number(record.getId())));
        items.append(new QStandardItem(QString::number(record.getAdminID())));
        items.append(new QStandardItem(record.getOperationType()));
        items.append(new QStandardItem(record.getTargetType()));
        items.append(new QStandardItem(record.getTargetID() == 0 ? "-" : QString::number(record.getTargetID())));
        items.append(new QStandardItem(record.getOperationDesc().isEmpty() ? "-" : record.getOperationDesc()));
        items.append(new QStandardItem(record.getOperationTime().toString("yyyy-MM-dd hh:mm:ss")));
        for (QStandardItem* item : items) {
            item->setEditable(false);
        }
        m_recordModel->appendRow(items);
    }
}
int AdminForm::getSelectedPlaneID()// 获取选中行的飞机ID
{
    QModelIndexList selectedIndexes = ui->tablePlane->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return -1;
    }
    QModelIndex index = selectedIndexes.first().siblingAtColumn(0);// 获取选中行的第0列（飞机ID）
    return index.data().toInt();
}

void AdminForm::on_tablePlane_clicked(const QModelIndex &index)// 选中行变化：同步机型下拉框
{
    int planeID = getSelectedPlaneID();
    if (planeID <= 0) {
        return;
    }

    bool found=0;
    for(const auto& plane:companies[m_companyID]->Planes){
        if(plane.getPlaneID()==planeID){
            found=1;
            QString planeType = plane.getPlaneType();
            int typeIndex = ui->cbxPlaneType->findText(planeType);// 同步下拉框选中状态
            if (typeIndex != -1) {
                ui->cbxPlaneType->setCurrentIndex(typeIndex);
            }
            break;
        }
    }
    if(!found){
        qDebug() << "选中飞机不在缓存中";
    }
}

void AdminForm::on_btnAddPlane_clicked()// 添加飞机按钮（仅调用Model，无数据库直接操作）
{
    QString err;
    QString planeType = ui->cbxPlaneType->currentText().trimmed();
    if(!controllerplane->AddPlane(planeType,m_companyID,err))
    {
        QMessageBox::warning(this, "添加失败", err);
        return;
    }
    else{
        QMessageBox::information(this,"添加成功",err);
    }
    loadPlaneData();//刷新表格
    loadRecordData();
}

void AdminForm::on_btnDeletePlane_clicked()// 删除选中飞机
{
    int planeID = getSelectedPlaneID();
    if (planeID <= 0) {
        QMessageBox::warning(this, "操作错误", "请先选中一行飞机数据！");
        return;
    }
    // 获取选中飞机名称（用于确认提示）
    QModelIndex nameIndex = ui->tablePlane->selectionModel()->selectedRows().first().siblingAtColumn(1);
    QString planeName = nameIndex.data().toString();
    // 二次确认
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("是否删除飞机「%1」（ID：%2）？\n删除后不可恢复！").arg(planeName).arg(planeID),
        QMessageBox::Yes | QMessageBox::No
        );
    if (reply != QMessageBox::Yes) {
        return;
    }

    QString err;
    if(!controllerplane->DeletePlane(planeID,planeName,m_companyID,err))
    {
        QMessageBox::warning(this, "删除失败", err);
        return;
    }
    else{
        QMessageBox::information(this, "删除成功", "飞机删除成功！");
    }
    loadPlaneData();// 刷新表格
    loadRecordData();

    ui->cbxPlaneType->setCurrentIndex(-1);
}

void AdminForm::on_refreshFlight_clicked()
{
    flightproxymodel->refreshFilter();
    qDebug() << "refresh adminform flight_page!";
}
