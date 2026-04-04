#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QString>
#include<QTimer>
#include<QElapsedTimer>

//测试
#include<QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Netmanager=new NetWorkManager(this);

    Netmanager->textEdit=ui->textEdit;
/*
    //设计中心窗口
    QWidget* central= new QWidget(this);
    setCentralWidget(central);

    //主布局
    QHBoxLayout *mainLayout=new QHBoxLayout(central);

    //>>信息显示
    //统计信息
    SumDataDisplay=new DataDisplay(central);
    BaseDataDisplay=new DataDisplay(central);
    SumDataDisplay->Display->setReadOnly(true);
    BaseDataDisplay->Display->setReadOnly(true);
    //QString str="hello world";
    //SumDataDisplay->setLabel(str);

*/
    BaseDataDisplay=ui->BaseDataDisplay;
    SumDataDisplay=ui->SumDataDisplay;
    gridworld=ui->gridworld;

    BaseDataDisplay->Display->setReadOnly(true);
    SumDataDisplay->Display->setReadOnly(true);
    ui->textEdit->setReadOnly(true);
/*
    //地图部分
    QGraphicsScene *Scene=new QGraphicsScene(this);
    QPixmap mapPixmap("://images/6EiQ0zF4o9.jpg");
    if(mapPixmap.isNull()){
        qDebug()<<"图片加载失败";
    }
    QGraphicsPixmapItem* mapItem=Scene->addPixmap(mapPixmap);
    Scene->setSceneRect(mapPixmap.rect());

    QGraphicsView *view=new QGraphicsView(Scene,this);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->resize(480,600);
*/
    //test
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        gridworld->addAgent();
    });
    connect(ui->pushButton,&QPushButton::clicked,Netmanager,&NetWorkManager::onselfclicked);

    connect(Netmanager->manager,&QNetworkAccessManager::finished,Netmanager,&NetWorkManager::onNetworkReplay);

    QTimer* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,gridworld,&GridWorld::updateWorld);
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}
