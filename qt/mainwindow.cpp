#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QString>
#include<QTimer>
#include<QElapsedTimer>

//测试
#include<QPushButton>
#include<QTextBlock>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    Netmanager=new NetWorkManager(this);

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

    connect(Netmanager->manager,&QNetworkAccessManager::finished,Netmanager,&NetWorkManager::onNetworkReplay);

    //>>test_for_redis
    int port=6379;
    if (!redismanager->connectToredis()) {
        ui->textEdit->append("警告：Redis 连接失败，请检查服务是否运行");
    } else {
        ui->textEdit->append("Redis 连接成功");
    }

    QTextBlock blocks=ui->textEdit->document()->findBlockByNumber(1);
    const std::string agent_action_decide=blocks.text().toStdString();


    //const QString key_agent_test_id="001";
    //const QString value=QString::fromStdString(agent_action_decide);
    QString cmd;
    connect(ui->setBtn,&QPushButton::clicked,[&](){
        const QString key_agent_test_id="001";
        QString value=QString::fromStdString(agent_action_decide);
        cmd=QString("SET %1 %2").arg(key_agent_test_id,value);
        QString result=redismanager->execommand(cmd);
        ui->textEdit->append("redis:"+result);
    });

    connect(ui->getBtn,&QPushButton::clicked,[&](){
        QString key_agent_test_id="001";
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
