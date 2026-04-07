#pragma once
#include< redisworker.h>
#include<qDebug>

redisWorker::redisWorker(QObject *parent)
    :QObject(parent),c(nullptr)
{
}

redisWorker::~redisWorker()
{
    disconnect();
}



//处理命令
QString redisWorker::execommand(QString cmd)
{
    if(!c){
        qDebug()<<"未连接redis";
        return "commad is failed";
    }

    QString result;

    redisReply*reply=(redisReply*)redisCommand(c,cmd.toStdString().c_str());
    switch (reply->type) {
    case REDIS_REPLY_STRING:
        result=QString::fromStdString(std::string(reply->str,reply->len));
        break;
    case REDIS_REPLY_INTEGER:
        result=QString::number(reply->integer);
        break;
    case REDIS_REPLY_STATUS:
        result=QString::fromStdString(std::string(reply->str,reply->len));
        break;
    case REDIS_REPLY_NIL:
        break;
    case REDIS_REPLY_ERROR:
        result=QString("Errot:")+QString::fromStdString(std::string(reply->str,reply->len));
        break;
    default:
        result ="type is wrong";
        break;
    }
    freeReplyObject(reply);
    return result;
}

void redisWorker::disconnect()
{
    if(c){
        redisFree(c);
        c=nullptr;
        return;
    }
}

bool redisWorker::connectToredis(const QString &host, const int &port)
{
    struct timeval timeout={2,0};
    c=redisConnectWithTimeout("127.0.0.1",6379,timeout);

    if(!c||c->err){
        if(c==nullptr){
            qDebug()<<"未连接";
            redisFree(c);
        }else{
            qDebug()<<"error:"<<c->err;
        }
        c = nullptr;
        return false;
    }
    qDebug()<<"已连接";
    return true;
}

