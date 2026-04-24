#include<redisworker.h>
#include<qDebug>
#include<QPointer>

redisWorker::redisWorker(QObject *parent)
    :QObject(parent),c(nullptr),messager(nullptr),subThread(nullptr)
{
}

redisWorker::~redisWorker()
{
    if(messager){
        messager->stop();
    }
    if(subThread){
        subThread->quit();
        subThread->wait();
        delete subThread;
        subThread=nullptr;
    }

    if(messager){
        delete messager;
        messager=nullptr;
    }

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
    if(reply==nullptr)return "reply is nullptr.";

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

void redisWorker::publish(const QString &channel, const QString &message)
{
    if(!c){
        qDebug()<<"未连接到redis服务器";
        return;
    }
    redisReply*reply=(redisReply*)redisCommand(c,"PUBLISH %s %s",channel.toStdString().c_str(),message.toStdString().c_str());

    if(!reply){
        qDebug()<<"未连接";
    }else{
        freeReplyObject(reply);
    }
}

void redisWorker::subscribe(const QString &channel)
{
    if(messager){
        messager->stop();
    }
    if(subThread){
        subThread->quit();
        subThread->wait();
        delete subThread;
        subThread=nullptr;
    }

    subThread=new QThread(this);
    messager=new redisMessager(nullptr);
    messager->moveToThread(subThread);

    //subscribe
    connect(messager,&redisMessager::messageReceived,this,&redisWorker::newMessage);

    connect(subThread,&QThread::started,[=](){
        messager->ReceiveMessageToRedisWhile(channel);
    });

    connect(subThread,&QThread::finished,messager,&redisMessager::deleteLater);

    subThread->start();
}

bool redisWorker::connectToredis(const QString &host, const int &port)
{
    struct timeval timeout={2,0};
    c=redisConnectWithTimeout(host.toStdString().c_str(),port,timeout);

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

void redisWorker::publish_Async(const QString& channel,const QString &message)
{
    QFuture<void> future=QtConcurrent::run([=](){
        if(!c)return;
        redisReply*reply=(redisReply*)redisCommand(c,"PUBLISH %s %s",channel.toStdString().c_str(),message.toStdString().c_str());
        if(reply){
            freeReplyObject(reply);
        }
    });
}

void redisWorker::execommand_Async(const QString&cmd)
{
    QFuture<QString> future=QtConcurrent::run([=]()->QString{
        if(!c)return "redis未连接";
        redisReply* reply=(redisReply*)redisCommand(c,cmd.toStdString().c_str());

        if(reply){
            QString result;

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
        return "空指针reply";
    });

    QFutureWatcher<QString>*watcher=new QFutureWatcher<QString>(this);

    connect(watcher,&QFutureWatcher<QString>::finished,this,[this,watcher](){
        QString result=watcher->result();
        emit cmdresult(result);
        watcher->deleteLater();
    });
    watcher->setFuture(future);
}
