#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QString>
#include<QTimer>
#include<QElapsedTimer>

//测试
#include<QPushButton>
#include<QTextBlock>
#include<QMessageBox>
#include<QJsonDocument>
#include<QJsonObject>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    //Netmanager=new NetWorkManager(this);
    //Netmanager->textEdit=ui->textEdit;

    //>>test_for_redis
    redismanager=new redisWorker(this);


    BaseDataDisplay=ui->BaseDataDisplay;
    SumDataDisplay=ui->SumDataDisplay;

    gridworld=ui->gridworld;
    gridworld->setRedisWorker(redismanager);

    BaseDataDisplay->Display->setReadOnly(true);
    SumDataDisplay->Display->setReadOnly(true);
    ui->textEdit->setReadOnly(true);


    //添加agent
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        gridworld->addAgent();
    });

    //连接redis
    int port=6379;
    if (!redismanager->connectToredis("127.0.0.1",port)) {
        ui->textEdit->append("警告：Redis 连接失败，请检查服务是否运行");
    } else {
        ui->textEdit->append("Redis 连接成功");
    }

    //订阅频道,处理消息
    redismanager->subscribe("Agent:Decision");
    QPointer<GridWorld>safe_gridworld=gridworld;
    connect(redismanager,&redisWorker::newMessage,[=](const QString&channel,const QString&message){
        if(channel!="Agent:Decision")return;
        if(!safe_gridworld)return;

        QJsonDocument doc=QJsonDocument::fromJson(message.toUtf8());
        if(!doc.isObject()){
            qDebug()<<"不是约定的操作";
            return;
        }
        QJsonObject obj=doc.object();
        QString agent_id=obj["id"].toString();
        QString agent_action=obj["decision"].toString().trimmed();

        for(auto temp:gridworld->getAgents()){
            if(temp->id==agent_id){
                Action action_A=temp->decide(agent_action);
                gridworld->actions[QString::fromStdString(temp->id)]=action_A;

                qDebug()<<temp->id<<action_A;
            }
        }
    });

    QTimer* timer=new QTimer(this);
    int reco=0;//时间计数
    connect(timer,&QTimer::timeout,gridworld,&GridWorld::updateWorld);
    connect(timer,&QTimer::timeout,this,[=,&reco](){
        reco++;
        displayAllData(reco);
    });
    timer->start(500);
}

MainWindow::~MainWindow()
{
    QTimer* timer=findChild<QTimer*>();
    if(timer)timer->stop();
    disconnect(redismanager,nullptr,this,nullptr);

    delete ui;
}

void MainWindow::displayAllData(int reco)
{
    if(!gridworld){
        return;
    }
    auto agents=gridworld->getAgents();
    auto gridset=gridworld->gridset;

    int number_agent=agents.size();
    int workers=0;
    int residenters=0;
    for(auto temp:agents){
        switch (temp->type) {
        case AgentType::Worker:
            workers++;
            break;
        case AgentType::Residenter:
            residenters++;
            break;
        default:
            break;
        }
    }

    QString base_data=QString(
        "Agent 总数: %1\n"
        "  工人: %2\n"
        "  居民: %3\n").arg((std::to_string(number_agent)),std::to_string(workers),std::to_string(residenters));
    BaseDataDisplay->Display->setPlainText(base_data);



    QString total_time=QString::fromStdString(std::to_string(500*reco)+"ms");


    QString sum_data=QString("当前世界经过总时间: %2\n").arg(total_time);
    SumDataDisplay->Display->setPlainText(sum_data);
}



void MainWindow::sendAgentDecideToredis(const QString& agent_decide_from_network)
{/*
    for(auto temp:gridworld->getAgents()){
        QString agent_redis_pub(QString::fromStdString(temp->id+":"));
        agent_redis_pub+=QString::fromStdString(Action_to_QString(temp->decide(agent_decide_from_network)));
        redismanager->publish("Agent:Decision",agent_redis_pub);
    }*/
}

void MainWindow::updatetheActions()
{
    Agent*temp;
    connect(redismanager,&redisWorker::newMessage,[=](const QString&channel,const QString&message){
        QString content=message;
        qDebug()<<message;

        QStringList parts=content.split(":");
        QString id=parts[0];
        QString action=parts[1];
        Action action_A=temp->decide(action);
        gridworld->actions[id]=action_A;
        qDebug()<<id<<action;
    });
}
