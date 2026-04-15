#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datadisplay.h"
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QLabel>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>

#include "gridworld.h"
#include"NetWorkManager.h"
#include" redisworker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;

//改为ui指针来管理
    DataDisplay *SumDataDisplay;
    DataDisplay *BaseDataDisplay;
    GridWorld* gridworld;
public slots:
    //在这里处理agent返回的数据，并将数据储存在redis里
    void sendAgentDecideToredis(const QString &agent_decide_from_network);
public:
    NetWorkManager* Netmanager;

    redisWorker* redismanager;
};
#endif // MAINWINDOW_H
