#pragma once

#include<hiredis.h>
#include<async.h>
#include<adapters/qt.h>   //connect this qt <core>

class redisWorker :public QObject{

    Q_OBJECT
public:
    redisContext* c;
public:
    explicit redisWorker(QObject*parent=nullptr);
    ~redisWorker();

    bool connectToredis(const QString& host="127.0.0.1",const int& port=6379);

    QString execommand(QString cmd);

    void disconnect();
};
