#pragma once
#include"NetWorkManager.h"
#include<QUrl>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
NetWorkManager::NetWorkManager(QObject *parent)
    :QObject(parent)
{
    manager=new QNetworkAccessManager(this);
    textEdit=nullptr;
}

void NetWorkManager::onselfclicked()
{
    //接入Ai，api接口
    QUrl qurl("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");
    QNetworkRequest request(qurl);

    //Json请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //>>自定义请求头
    QByteArray key("Bearer sk-5463835ade8d4045bb989ff2f5cf2097");
    request.setRawHeader("Authorization", key);


    QJsonObject obj;
    obj["model"]="qwen-plus";
    QJsonArray messages;
    QJsonObject userMSG;
    userMSG["role"]="user";
    //MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Interact
    userMSG["content"]="你现在处于gridworld[10,10]的位置，周围很空旷，需要你随便逛逛来确认环境"
                        "请你在这几个单词里选择一个单词：MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Interact"
                        "来探索周围的世界，你的上一次动作是MoveUp"
                        "你回答的格式只能是这几个单词，不准出现任何除这几个之外的字样";
    messages.append(userMSG);
    obj["messages"]=messages;

    obj["temperature"]=1.2;

    QJsonDocument doc(obj);
    QByteArray data=doc.toJson();

    manager->post(request,data);
}

void NetWorkManager::onNetworkReplay(QNetworkReply* reply)
{
    if(reply->error()!=QNetworkReply::NoError){
        QString errorstring=reply->errorString();
        textEdit->append("错误："+errorstring);
        reply->deleteLater();
        return ;
    }

    QByteArray data=reply->readAll();
    QUrl url=reply->url();

    //处理信息：
    if(url.toString().contains("compatible-mode")){
        QJsonDocument doc=QJsonDocument::fromJson(data);
        if(doc.isObject()){
            QJsonObject root=doc.object();

            if(root.contains("choices")){
                QJsonArray choices=root["choices"].toArray();

                if(!choices.isEmpty()){
                    QJsonObject choice=choices[0].toObject();
                    QJsonObject message=choice["message"].toObject();
                    QString aiReply=message["content"].toString();
                    textEdit->append("Ai:"+aiReply);
                }
            }else{
                textEdit->append("未接受任何响应");
            }
        }
    }
    reply->deleteLater();
}

void NetWorkManager::onNetworkReply_to_redis(QNetworkReply* reply)
{
    if(reply->error()!=QNetworkReply::NoError){
        QString errorstring=reply->errorString();
        textEdit->append("错误："+errorstring);
        reply->deleteLater();
        return ;
    }

    QByteArray data=reply->readAll();
    QUrl url=reply->url();

    //处理信息：
    if(url.toString().contains("compatible-mode")){
        QJsonDocument doc=QJsonDocument::fromJson(data);
        if(doc.isObject()){
            QJsonObject root=doc.object();

            if(root.contains("choices")){
                QJsonArray choices=root["choices"].toArray();

                if(!choices.isEmpty()){
                    QJsonObject choice=choices[0].toObject();
                    QJsonObject message=choice["message"].toObject();
                    QString aiReply=message["content"].toString();
                    emit AgentReply(aiReply);
                }
            }else{
                textEdit->append("未接受任何响应");
            }
        }
    }
    reply->deleteLater();
}
