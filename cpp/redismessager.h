#pragma once
#include<QObject>
#include<hiredis.h>

class redisMessager:public QObject{
    Q_OBJECT
public:
    redisMessager(QObject*parent);
    ~redisMessager();

    void ReceiveMessageToRedisWhile(const QString&channel);
    void stop();
signals:
    void messageReceived(const QString&channel,const QString&message);
private:
    redisContext*context;
    QString currentChannel;
    std::atomic<bool>m_stop;
};
