#include <QMessageBox>
#include "weatherforecastform.h"
#include "ui_weatherforecastform.h"

WeatherForecastForm::WeatherForecastForm(QWidget *parent,ControllerFlight *_controllerflight)
    : QWidget(parent)
    , controllerflight(_controllerflight)
    , ui(new Ui::WeatherForecastForm)
{
    ui->setupUi(this);
}

WeatherForecastForm::~WeatherForecastForm()
{
    delete ui;
}

void WeatherForecastForm::on_btnBadWeather_clicked()
{
    QString err;
    QDateTime t1=ui->dtmStartTime->dateTime();
    QDateTime t2=ui->dtmEndTime->dateTime();
    if(!controllerflight->BadWeather(t1,t2,err)){
        QMessageBox::warning(this, "警告", err);
    }
    else{
        QMessageBox::information(this, "提示", "天气预报更新成功！");
    }
}


void WeatherForecastForm::on_btnGoodWeather_clicked()
{
    QString err;
    QDateTime t1=ui->dtmStartTime->dateTime();
    QDateTime t2=ui->dtmEndTime->dateTime();
    if(!controllerflight->GoodWeather(t1,t2,err)){
        QMessageBox::warning(this, "警告", err);
    }
    else{
        QMessageBox::information(this, "提示", "天气预报更新成功！");
    }
}
