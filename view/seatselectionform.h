#ifndef SEATSELECTIONFORM_H
#define SEATSELECTIONFORM_H

#include <QWidget>
#include "seatitem.h"
namespace Ui {
class SeatSelectionForm;
}

class SeatSelectionForm : public QWidget
{
    Q_OBJECT

public:
    QVector<SeatItem*> seat;
    explicit SeatSelectionForm(QWidget *parent = nullptr);
    ~SeatSelectionForm();
private slots:
    void focusedIdChanged(int id);
private:
    Ui::SeatSelectionForm *ui;
    int numSeats=20;
    int focusedID=-1;
};

#endif // SEATSELECTIONFORM_H
