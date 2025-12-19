// SeatItem.cpp
#include "seatitem.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QBrush>

SeatItem::SeatItem(int id, SeatState state)
    : id(id)
    , seatNumber(id + 1)  // 默认座位号
    , state(state)
    , isCurrentlySelected(false)
{
    sname = QString("S%1").arg(seatNumber);

    setRect(0, 0, 40, 40);
    setPen(QPen(Qt::black));

    // 设置可接受鼠标事件
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    label = new QGraphicsSimpleTextItem(sname, this);
    label->setPos(5, 10);
    label->setBrush(Qt::white);  // 文字颜色设为白色，在深色背景上更清楚

    updateColor();
}

void SeatItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "点击座位 id:" << id << "状态:" << state << "座位号:" << seatNumber;

    // 如果是被占用的座位，不发送信号
    if (state == Occupied) {
        qDebug() << "座位被占用，忽略点击";
        event->accept();
        return;
    }

    // 发送点击信号
    emit clicked(id);
    event->accept();
}

void SeatItem::setState(SeatState s)
{
    if (state == s) return;

    state = s;
    updateColor();

    // 如果状态变为占用，自动取消选中
    if (state == Occupied) {
        isCurrentlySelected = false;
    }
}

void SeatItem::setSelected(bool selected)
{
    if (isCurrentlySelected == selected) return;

    isCurrentlySelected = selected;
    updateColor();
}

void SeatItem::updateColor()
{
    QColor color;

    if (state == Occupied) {
        // 被占用的座位总是灰色
        color = QColor("#808080");
    }
    else if (isCurrentlySelected) {
        // 被选中的座位
        color = QColor("#3399FF");
    }
    else {
        // 根据状态选择颜色
        switch(state) {
        case Available:
            color = QColor("#8EC07C");  // 绿色
            break;
        case Selected:
            color = QColor("#FABD2F");  // 黄色
            break;
        default:
            color = QColor("#8EC07C");
            break;
        }
    }

    setBrush(QBrush(color));

    // 更新边框颜色
    if (isCurrentlySelected) {
        setPen(QPen(Qt::black, 2));  // 选中时边框加粗
    } else {
        setPen(QPen(Qt::black, 1));  // 普通边框
    }
}
