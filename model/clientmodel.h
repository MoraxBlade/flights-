#ifndef CLIENTMODEL_H
#define CLIENTMODEL_H
#include <QString>

class ClientModel
{
public:
    static bool InsertClient(const QString& name,const QString& hashpwd,const QByteArray& salt,QString& err);
};

#endif // CLIENTMODEL_H
