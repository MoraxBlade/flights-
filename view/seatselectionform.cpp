#include "seatselectionform.h"
#include "ui_seatselectionform.h"
#include "seatitem.h"

SeatSelectionForm::SeatSelectionForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SeatSelectionForm)
{
    ui->setupUi(this);

    if (!ui->graphicsView->scene())
        ui->graphicsView->setScene(new QGraphicsScene(this));

    seat.resize(numSeats);
    for (int i = 0; i < numSeats; i++) {
        seat[i] = new SeatItem(i, Available);
        seat[i]->setPos((i%3)*50, (i/3) * 50);// 一列排列：y = i * 50
        ui->graphicsView->scene()->addItem(seat[i]);
        connect(seat[i], &SeatItem::clicked, this, &SeatSelectionForm::focusedIdChanged);
    }
}

SeatSelectionForm::~SeatSelectionForm()
{
    delete ui;
    for(const auto& ptr:seat) delete ptr;
}
void SeatSelectionForm::focusedIdChanged(int id){
    focusedID=id;
}
