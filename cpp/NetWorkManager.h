#pragma once

#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QTextEdit>

class NetWorkManager:public QObject{
    Q_OBJECT
public:
    QNetworkAccessManager* manager;
    QTextEdit* textEdit;
public:
    explicit NetWorkManager(QObject* parent=nullptr);
    ~NetWorkManager(){}
public slots:
    void onselfclicked();
    void onNetworkReplay(QNetworkReply* reply);

    void onNetworkReply_to_redis(QNetworkReply *reply);
signals:
    void AgentReply(QString aiReply);
};
