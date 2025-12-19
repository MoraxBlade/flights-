#include "client.h"

Client::Client():User() {}

Client::Client(int _id,const QString& _name,const QString& _hashpwd,const QByteArray& _salt,const QString& _type)
    :User(_id,_name,_hashpwd,_salt,_type){}

Client::~Client(){}
