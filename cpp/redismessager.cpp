#include"redismessager.h"
#include<QDebug>

redisMessager::redisMessager(QObject *parent)
    :QObject(parent),context(nullptr),m_stop(false)
{
}

redisMessager::~redisMessager()
{
    if(context){
        redisFree(context);
        context=nullptr;
    }
}

void redisMessager::ReceiveMessageToRedisWhile(const QString &channel)
{
    context=redisConnect("127.0.0.1",6379);
    if(context==nullptr||context->err){
        if(!context){
            qDebug()<<"未连接";
        }
        redisFree(context);
        context=nullptr;
        return;
    }

    redisAppendCommand(context,"SUBSCRIBE %s",channel.toStdString().c_str());
    while(!m_stop){
        redisReply*reply=nullptr;
        if((redisGetReply(context,(void**)(&reply))==REDIS_OK)){
            if(reply!=nullptr&&reply->type==REDIS_REPLY_ARRAY&&reply->elements==3){
                //["message"],频道名,消息内容
                QString head=reply->element[0]->str;
                if(head=="message"){
                    QString now_channel=reply->element[1]->str;
                    QString now_message=reply->element[2]->str;

                    emit messageReceived(now_channel,now_message);
                }
            }
        }else{
            break;
        }
    }

    if(context){
        redisFree(context);
        context=nullptr;
    }
}

void redisMessager::stop()
{
    m_stop=true;
}
