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
    redismanager=new redisWorker(this);
    Netmanager->textEdit=ui->textEdit;

    //>>test_for_redis
    redismanager=new redisWorker(this);
    //<<test_for_redis


    BaseDataDisplay=ui->BaseDataDisplay;
    SumDataDisplay=ui->SumDataDisplay;
    gridworld=ui->gridworld;

    BaseDataDisplay->Display->setReadOnly(true);
    SumDataDisplay->Display->setReadOnly(true);
    ui->textEdit->setReadOnly(true);


    //test
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        gridworld->addAgent();
    });
    connect(ui->pushButton,&QPushButton::clicked,Netmanager,&NetWorkManager::onselfclicked);
/*
    connect(Netmanager->manager,&QNetworkAccessManager::finished,Netmanager,&NetWorkManager::onNetworkReplay);

    connect(gridworld,&GridWorld::updated_world,[=](Agent*agent_temp){
        //Netmanager->onselfclicked();网络请求是异步的，也就是说可能在这条语句之前就已经连接了，那么这个时候就会将旧链接断开
        //同时一个信号连接多个槽函数，但是又只有一个networker实例，导致导致错误出现的多个lambada没有正确执行
        connect(Netmanager->manager,&QNetworkAccessManager::finished,Netmanager,&NetWorkManager::onNetworkReply_to_redis);
        connect(Netmanager,&NetWorkManager::AgentReply,[=](QString agentReply){
            QString cmd=QString("SET %1 %2").arg(QString::fromStdString(agent_temp->id),agentReply);
            redismanager->execommand(cmd);
        });
    });*/
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
    QTextBlock blocks=ui->textEdit->document()->findBlockByNumber(1);
    const std::string agent_action_decide=blocks.text().toStdString();

    qDebug()<<agent_action_decide;
    connect(ui->pushButton,&QPushButton::clicked,[=](){
        qDebug()<<agent_action_decide;
    });
*/
    //const QString key_agent_test_id="001";
    //const QString value=QString::fromStdString(agent_action_decide);
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
    });

    //<<test_for_redis

    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,gridworld,&GridWorld::updateWorld);
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendAgentDecideToredis(const QString& agent_decide_from_network)
{
    for(auto temp:gridworld->getAgents()){
        QString agent_redis_key=QString::fromStdString(temp->id);
        QString agent_redis_value=QString::fromStdString(Action_to_QString(temp->decide(agent_decide_from_network)));

        QString cmd=(QString("SET %1 %2").arg(agent_redis_key,agent_redis_value));
        redismanager->execommand(cmd);
    }
}
