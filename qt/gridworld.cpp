#include "gridworld.h"
#include<QRandomGenerator>
#include<QString>
#include<QRect>
#include<QDebug>

#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>

GridWorld::GridWorld(QWidget *parent)
    : QWidget{parent}
{
    setMinimumSize(600,600);
    setMaximumSize(600,600);
    generateWorldmap(width() / gridsize, height() / gridsize);
}

GridWorld::GridWorld(int Gwidth, int Gheight, QWidget *parent)
    :QWidget(parent)
{
    int xSize=Gwidth/20;
    int ySize=Gheight/20;

    setMinimumSize(Gwidth,Gheight);
    setMaximumSize(Gwidth,Gheight);

    generateWorldmap(xSize,ySize);

    update();
}

void GridWorld::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    painter.setPen(QPen(Qt::black, 1));   // 明确画笔

    int w = width();
    int h = height();

    // 画竖线
    for (int x = 0; x <= w; x += gridsize) {
        painter.drawLine(x, 0, x, h);
    }
    // 画横线
    for (int y = 0; y <= h; y += gridsize) {
        painter.drawLine(0, y, w, y);
    }


    int xSize=w/gridsize;
    int ySize=h/gridsize;
    /*
    Empty,
    Supermakert,
    Financialexchange,
    Resident,
    Park,
    Government*/
    for(int i=0;i<xSize;i++){
        for(int j=0;j<ySize;j++){
            if(i>=gridset.size()&&j>gridset[i].size())continue;

            QRect cellRect(i*gridsize,j*gridsize,gridsize,gridsize);
            QColor color;

            switch(gridset[i][j].build){
            case Empty:
                color=Qt::lightGray;
                break;
            case Supermakert:
                color=Qt::yellow;
                break;
            case Financialexchange:
                color=Qt::black;
                break;
            case Resident:
                color=Qt::cyan;
                break;
            case Park:
                color=Qt::blue;
                break;
            case Government:
                color=Qt::red;
                break;
            default:
                color = Qt::white;
                break;
            }
            painter.fillRect(cellRect,color);
        }
    }

    for(Agent* temp:agents){
        QColor color;
        switch (temp->type) {
        case AgentType::Worker:
            color=Qt::black;
            break;
        case AgentType::Residenter:
            color=Qt::gray;
            break;
        case AgentType::Manager:
            color=Qt::green;
            break;
        default:
            break;
        }

        painter.setBrush(color);
        int x = temp->pos.x * gridsize;
        int y = temp->pos.y * gridsize;
        //画一个小一点的圆
        painter.drawEllipse(x + 2, y + 2, gridsize - 4, gridsize - 4);
    }
}

void GridWorld::generateWorldmap(int Xsize, int Ysize)
{
    //开辟容器空间
    gridset.resize(Xsize);
    for(int i=0;i<Xsize;i++){
        gridset[i].resize(Ysize);
    }

    //government position
    int x_g=QRandomGenerator::global()->bounded(Xsize);
    int y_g=QRandomGenerator::global()->bounded(Ysize);

    for(int i=0;i<Xsize;i++){
        for(int j=0;j<Ysize;j++){
            if((i==x_g) && (j==y_g)){
                gridset[i][j].build=Government;
                gridset[i][j].resource=QRandomGenerator::global()->bounded(30)+10;
            }else{
                int r=QRandomGenerator::global()->bounded(5);

                Building b=static_cast<Building>(r);
                if(b==Government)b=Empty;

                gridset[i][j].build=b;
                gridset[i][j].resource=QRandomGenerator::global()->bounded(10);
            }
        }
    }
}

const std::vector<Agent *> &GridWorld::getAgents() const
{
    return agents;
}

void GridWorld::addAgent()
{
    Status sta;//怎么会有这种个问题啊： Most Vexing Parse 服了

    int col=this->width()/gridsize;
    int row=this->height()/gridsize;
    int x_newAgent=rand()%(col-2)+1;
    int y_newAgent=rand()%(row-2)+1;
    Position pos(x_newAgent, y_newAgent);

    int work_type=rand()%2+1;
    AgentType type = static_cast<AgentType>(work_type);

    size_t reco=agents.size();
    std::string id="test_"+std::to_string(reco);
    Agent* New=new Agent(id, type, sta, pos);

    agents.push_back(New);
}

void GridWorld::updateWorld()
{
    //test,this is not the finally versio
    /*
    for (Agent* agent : agents) {

        redisWorker redis_worker_temp(this);
        if(!redis_worker_temp.connectToredis("127.0.0.1",6379))
        }
        QString agent_id=QString::fromStdString(agent->id);

        QString cmd(QString ("GET %1").arg(agent_id));

        QString redis_reply=redis_worker_temp.execommand(cmd);
        int start=redis_reply.indexOf(':')+1;
        QString agent_reply=redis_reply.mid(start);
        actions.push_back(agent->decide(agent_reply));
    }*/

    //take actions
    for (size_t i = 0; i < agents.size(); i++) {
        Agent* temp = agents[i];
        Action action_temp = actions[QString::fromStdString(temp->id)];

        qDebug() << "Agent" << QString::fromStdString(temp->id) << "executes action:" << action_temp;

        Position temp_pos = temp->pos;
        //qDebug()<<QString::fromStdString(temp->id)<<":"<<pos.x<<","<<pos.y;
        switch (action_temp)
        {
        case MoveUp:
            temp_pos.y--;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveDown:
            temp_pos.y++;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveRight:
            temp_pos.x++;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveLeft:
            temp_pos.x--;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case Staying:
            if (temp->status.action_energy.energyValue <= 95)
                temp->status.action_energy.energyValue += 5;

            else temp->status.action_energy.energyValue = 100;
            break;
        case Work:
            if (temp->status.action_energy.energyValue <= 5)
                temp->status.action_energy.energyValue = 0;
            else temp->status.action_energy.energyValue -= 5;

            if (temp->status.action_energy.energyValue <= 30)temp->status.eat_Status = EatStatus::Hungry;
            break;
        case Interact:
            break;
        default:
            qDebug()<<"default";
            break;
        }
        //actions[QString::fromStdString(temp->id)]=Staying;
        //temp->getPos();
        //emit updated_world(temp);这样会信号爆炸
    }

    if(m_worker){
        send_AgentStatus_AndWorld_ToRedis(m_worker);
    }

    update();
}

bool GridWorld::isOutWorld(int x,int y)
{
    if(gridset.isEmpty()||gridset[0].size()==0){
        return true;
    }
    int col = gridset.size();
    int row = gridset[0].size();
    //qDebug()<<"col:"<<col;
    //qDebug()<<"row:"<<row;
    if ((x >= 0 && x < col) && (y >= 0 && y < row))return false;
    return true;
}

void GridWorld::send_AgentStatus_AndWorld_ToRedis(redisWorker *redis)
{
    if(!redis){
        qDebug()<<"未连接";
        return;
    }


    //agent本身的信息
    for(auto agent:agents){
        QJsonObject alldate;

        QJsonArray AgentState;
        QJsonObject agentstate;
        agentstate["id"]=QString::fromStdString(agent->id);
        agentstate["type"]=agent->type;
        agentstate["x"]=agent->pos.x;
        agentstate["y"]=agent->pos.y;
        agentstate["energy"]=agent->status.action_energy.energyValue;
        AgentState.append(agentstate);

        //gridworld局部信息
        QJsonArray worlddate;
        for(int i=-2;i<=2;i++){
            for(int j=-2;j<=2;j++){
                int temp_x=agent->pos.x+i;
                int temp_y=agent->pos.y+j;
                if(isOutWorld(temp_x,temp_y)){
                    continue;
                }
                QJsonObject gridworld_cell;
                gridworld_cell["x"]=temp_x;
                gridworld_cell["y"]=temp_y;
                gridworld_cell["building"]=gridset[temp_x][temp_y].build;
                gridworld_cell["resource"]=gridset[temp_x][temp_y].resource;

                worlddate.append(gridworld_cell);
            }
        }

        alldate["AgentState"]=AgentState;
        alldate["WorldDate"]=worlddate;

        QJsonDocument doc(alldate);
        QString json=doc.toJson(QJsonDocument::Compact);

        redis->publish("Agent:State",json);
    }
}
