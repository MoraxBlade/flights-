#ifndef ENCODER_H
#define ENCODER_H
#include<QString>
class Encoder
{
public:
    static QByteArray GenerateSalt(int length=16);
    static QString Hash(const QString& pwd,const QByteArray& salt);
    static QByteArray pubSalt;
    static QString pubHash;
};
#endif // ENCODER_H
