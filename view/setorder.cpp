#include "setorder.h"
#include "ui_setorder.h"

setorder::setorder(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::setorder)
{
    ui->setupUi(this);
}

setorder::~setorder()
{
    delete ui;
}
