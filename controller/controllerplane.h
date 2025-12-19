#ifndef CONTROLLERPLANE_H
#define CONTROLLERPLANE_H
#include <QObject>
class ControllerPlane:public QObject{
    Q_OBJECT
public:
    explicit ControllerPlane(QObject* parent = nullptr);
    bool AddPlane(const QString& planeType,int cid,QString& err);
    bool DeletePlane(int pid,const QString& pname,int cid,QString& err);
};

#endif // CONTROLLERPLANE_H
