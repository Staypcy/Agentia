#ifndef GRIDWORLD_H
#define GRIDWORLD_H

#include <QWidget>
#include<QVector>
#include<QPainter>

#include<vector>
#include"Agent.h"

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
private:
    std::vector<Agent*> agents;
signals:

public:
    QVector<QVector<Gridcell>>gridset;
};


#endif // GRIDWORLD_H
