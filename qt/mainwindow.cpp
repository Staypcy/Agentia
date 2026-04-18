#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QString>
#include<QTimer>
#include<QElapsedTimer>

//测试
#include<QPushButton>
#include<QTextBlock>
#include<QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    Netmanager=new NetWorkManager(this);
    Netmanager->textEdit=ui->textEdit;

    //>>test_for_redis
    redismanager=new redisWorker(this);


    BaseDataDisplay=ui->BaseDataDisplay;
    SumDataDisplay=ui->SumDataDisplay;

    gridworld=ui->gridworld;
    gridworld->setRedisWorker(redismanager);

    BaseDataDisplay->Display->setReadOnly(true);
    SumDataDisplay->Display->setReadOnly(true);
    ui->textEdit->setReadOnly(true);


    //test
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        gridworld->addAgent();
    });
    //网络连接
    connect(ui->pushButton,&QPushButton::clicked,Netmanager,&NetWorkManager::onselfclicked);

    connect(Netmanager->manager,&QNetworkAccessManager::finished,Netmanager,&NetWorkManager::onNetworkReply_to_redis);
    connect(Netmanager,&NetWorkManager::AgentReply,this,&MainWindow::sendAgentDecideToredis);

    //>>test_for_redis
    int port=6379;
    if (!redismanager->connectToredis("127.0.0.1",port)) {
        ui->textEdit->append("警告：Redis 连接失败，请检查服务是否运行");
    } else {
        ui->textEdit->append("Redis 连接成功");
    }


/*
    QString cmd;
    connect(ui->setBtn,&QPushButton::clicked,[&](){
        QTextBlock blocks=ui->textEdit->document()->findBlockByNumber(1);
        const std::string agent_action_decide=blocks.text().toStdString();

        //暂且先只创建一位agent，后续实现循环将创建所有的agents的id提取
        const QString key_agent_test_id=QString::fromStdString(gridworld->getAgents()[0]->id);

        QString value=QString::fromStdString(agent_action_decide);
        if(value==""){
            QMessageBox::warning(this,"Waring","值不能为空");
            return;}
        cmd=QString("SET %1 %2").arg(key_agent_test_id,value);
        QString result=redismanager->execommand(cmd);
        ui->textEdit->append("redis:"+result);
    });

    connect(ui->getBtn,&QPushButton::clicked,[&](){

        const QString key_agent_test_id=QString::fromStdString(gridworld->getAgents()[0]->id);

        cmd=QString("GET %1").arg(key_agent_test_id);

        QString redis_reply=redismanager->execommand(cmd);
        ui->textEdit->append("redis:"+redis_reply);
    });*/

    //<<test_for_redis

    redismanager->subscribe("Agent:Decision");
    connect(redismanager,&redisWorker::newMessage,[=](const QString&channel,const QString&message){
        QString content=message;
        qDebug()<<message;
        QStringList parts=content.split(":");
        QString id=parts[0];
        QString action=parts[1];
        for(auto temp:gridworld->getAgents()){
            Action action_A=temp->decide(action);
            gridworld->actions[id]=action_A;
        }
        qDebug()<<id<<action;
    });

    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,gridworld,&GridWorld::updateWorld);
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendAgentDecideToredis(const QString& agent_decide_from_network)
{/*
    for(auto temp:gridworld->getAgents()){
        QString agent_redis_key=QString::fromStdString(temp->id);
        QString agent_redis_value=QString::fromStdString(Action_to_QString(temp->decide(agent_decide_from_network)));

        QString cmd=(QString("SET %1 %2").arg(agent_redis_key,agent_redis_value));
        redismanager->execommand(cmd);
    }*/
    for(auto temp:gridworld->getAgents()){
        QString agent_redis_pub(QString::fromStdString(temp->id+":"));
        agent_redis_pub+=QString::fromStdString(Action_to_QString(temp->decide(agent_decide_from_network)));
        redismanager->publish("Agent:Decision",agent_redis_pub);
    }
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
