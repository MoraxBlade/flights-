#ifndef SETORDER_H
#define SETORDER_H

#include <QWidget>

namespace Ui {
class setorder;
}

class setorder : public QWidget
{
    Q_OBJECT

public:
    explicit setorder(QWidget *parent = nullptr);
    ~setorder();

private:
    Ui::setorder *ui;
};

#endif // SETORDER_H
