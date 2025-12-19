// SeatItem.h
#ifndef SEATITEM_H
#define SEATITEM_H
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <QBrush>

enum SeatState {
    Available,
    Selected,
    Occupied
};

class SeatItem: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    SeatItem(int id, SeatState state = Available);
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void setState(SeatState s);
    SeatState getState() const { return state; }

    void setSelected(bool selected);
    bool isSelected() const { return isCurrentlySelected; }

    void setSeatName(QString s) { sname = s; }
    QString getSeatName() const { return sname; }

    void setSeatNumber(int num) { seatNumber = num; }
    int getSeatNumber() const { return seatNumber; }

    void setId(int sid) { id = sid; }
    int getId() const { return id; }

signals:
    void clicked(int id);

private:
    void updateColor();

    int id;
    int seatNumber;  // 实际座位号
    QString sname;   // 座位显示名称
    SeatState state;
    bool isCurrentlySelected;
    QGraphicsSimpleTextItem *label;
};

#endif // SEATITEM_H
