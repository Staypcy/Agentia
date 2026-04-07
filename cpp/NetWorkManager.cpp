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
    QByteArray key("Bearer sk-bb9715ee3c3d4773ab760709e45e7433");
    request.setRawHeader("Authorization",key);

    QJsonObject obj;
    obj["model"]="qwen-plus";
    QJsonArray messages;
    QJsonObject userMSG;
    userMSG["role"]="user";
    userMSG["content"]="请选择：[Up,Down,Left,Right],回答我时只能回答这个四个单词，一律不准有任何其他的token";
    messages.append(userMSG);
    obj["messages"]=messages;

    obj["temperature"]=0.2;

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
