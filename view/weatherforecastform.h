#ifndef WEATHERFORECASTFORM_H
#define WEATHERFORECASTFORM_H

#include <QWidget>
#include "../controller/controllerflight.h"

namespace Ui {
class WeatherForecastForm;
}

class WeatherForecastForm : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherForecastForm(QWidget *parent = nullptr,ControllerFlight *_controllerflight=nullptr);
    ~WeatherForecastForm();

private slots:
    void on_btnBadWeather_clicked();

    void on_btnGoodWeather_clicked();

private:
    Ui::WeatherForecastForm *ui;
    ControllerFlight *controllerflight;
};

#endif // WEATHERFORECASTFORM_H
