#ifndef CLIENT_H
#define CLIENT_H
#include "user.h"

class Client:public User{
public:
    Client();
    Client(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type);
    virtual ~Client();
};

#endif // CLIENT_H
