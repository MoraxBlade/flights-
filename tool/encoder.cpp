#include <QCryptographicHash>
#include <QRandomGenerator>
#include "encoder.h"

QByteArray Encoder::GenerateSalt(int length){
    QByteArray salt;
    salt.reserve(length);
    for(int i=0;i<length;i++){ //生成ASCII 可打印字符作为盐
        char c=static_cast<char>(QRandomGenerator::global()->bounded(33, 127));
        salt.append(c);
    }
    return salt;
}

QString Encoder::Hash(const QString& pwd,const QByteArray& salt){
    QByteArray passBytes=pwd.toUtf8();
    QByteArray salted=salt+passBytes;
    QByteArray hash=QCryptographicHash::hash(salted,QCryptographicHash::Sha256);
    return hash.toHex();
}

QByteArray Encoder::pubSalt=QByteArray::fromHex("WeAllLoveSYSU!!!");
QString Encoder::pubHash="24a2cdf61c9ed3245be876db82b89d7cdd4b9975c1af78e59e0b7ced4bbcc7fc";
