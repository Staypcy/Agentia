#pragma once

#include<hiredis.h>
#include<async.h>
#include<adapters/qt.h>   //connect this qt <core>
#include<QThread>
#include"redismessager.h"

class redisWorker :public QObject{

    Q_OBJECT
public:
    redisContext* c;

    QThread *subThread;
    redisMessager* messager;
public:
    explicit redisWorker(QObject*parent=nullptr);
    ~redisWorker();

    bool connectToredis(const QString& host="127.0.0.1",const int& port=6379);

    QString execommand(QString cmd);

    void disconnect();

    void publish(const QString& channel,const QString& message);
    void subscribe(const QString& channel);
signals:
    void newMessage(const QString& channel,const QString& message);

};
