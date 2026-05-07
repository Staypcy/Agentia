#ifndef GRIDWORLD_H
#define GRIDWORLD_H

#include <QWidget>
#include<QVector>
#include<QPainter>

#include<vector>
#include"Agent.h"

#include"redisworker.h"
#include"FunctionTab.h"
#include<QPointer>


const int gridsize=20;

enum Building {
    Empty,
    Supermakert,
    Financialexchange,
    Resident,
    Park,
    Government,
};

struct Gridcell{
    Building build;
    int resource;
};

class GridWorld : public QWidget
{
    Q_OBJECT
public:
    explicit GridWorld(QWidget *parent = nullptr);
    GridWorld(int Gwidth, int Gheight,QWidget* parent=nullptr);
protected:
    void paintEvent(QPaintEvent *event)override;
    void generateWorldmap(int Xsize,int Ysize);

public:
    const std::vector<Agent*>& getAgents()const;
    void addAgent();
    void updateWorld();
    bool isOutWorld(int x,int y);

    void setRedisWorker(redisWorker*temp){m_worker=temp;}
    void send_AgentStatus_AndWorld_ToRedis(redisWorker *redis);

    void receive_AgentToolCall(redisWorker*redis);
    void setToolChannel();

private:
    std::vector<Agent*> agents;
    FunctionTab* functab;
signals:
    void updated_world(Agent* temp);
public:
    QVector<QVector<Gridcell>>gridset;

    std::map<QString,Action>actions;

    redisWorker*m_worker=nullptr;
    redisWorker* m_toolworker=nullptr;
};


#endif // GRIDWORLD_H
