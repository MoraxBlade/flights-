#ifndef USERMODEL_H
#define USERMODEL_H
#include "../entity/user.h"

class UserModel{
public:
    static bool SelectByName(const QString& name,User& user,QString& err);
};

#endif // USERMODEL_H
