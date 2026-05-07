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

    functab=new FunctionTab();

    functab->creatFunction(
        FunctionStruct{"perceive_env", "获取指定坐标周围3x3的环境信息", R"({"type":"object","properties":{"x":{"type":"integer"},"y":{"type":"integer"}},"required":["x","y"]})"},
                [this](const QJsonObject& params) -> FunctionResult {
                int x = params["x"].toInt();
                int y = params["y"].toInt();

                // 返回3x3范围的grid信息
                QJsonArray env;
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        if (!isOutWorld(x+i, y+j)) {
                            QJsonObject cell;
                            cell["x"] = x+i;
                            cell["y"] = y+j;
                            cell["building"] = gridset[x+i][y+j].build;
                            cell["resource"] = gridset[x+i][y+j].resource;
                            env.append(cell);
                        }
                    }
                }
                QJsonDocument doc(env);
                return {true, doc.toJson().toStdString()};
            }
        );

    functab->creatFunction(
        FunctionStruct{"take_action", "执行指定的行动/操作", R"({"type":"object","properties":{"direction":{"type":"string","enum":["Staying","MoveUp","MoveDown","MoveRight","MoveLeft","Work","Interact"]}},"required":["direction"]})"},
            [this](const QJsonObject& params) -> FunctionResult {
             for (size_t i = 0; i < agents.size(); i++) {
                 Agent* temp = agents[i];
                 Action action_temp = actions[QString::fromStdString(temp->id)];

                 qDebug() << "Agent" << QString::fromStdString(temp->id) << "executes action:" << action_temp;

                 Position temp_pos = temp->pos;
                 int temp_resource=temp->agent_resource;
                 //qDebug()<<QString::fromStdString(temp->id)<<":"<<pos.x<<","<<pos.y;
                 switch (action_temp)
                 {
                 case MoveUp:
                     temp_pos.y--;
                     temp_resource-=3;
                     if(temp_resource<0){
                         break;
                     }
                     temp->agent_resource-=3;
                     if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                         temp->move(action_temp);
                     }
                     break;
                 case MoveDown:
                     temp_pos.y++;
                     temp_resource-=3;
                     if(temp_resource<0){
                         break;
                     }
                     temp->agent_resource-=3;
                     if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                         temp->move(action_temp);
                     }
                     break;
                 case MoveRight:
                     temp_pos.x++;
                     temp_resource-=3;
                     if(temp_resource<0){
                         break;
                     }
                     temp->agent_resource-=3;
                     if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                         temp->move(action_temp);
                     }
                     break;
                 case MoveLeft:
                     temp_pos.x--;
                     temp_resource-=3;
                     if(temp_resource<0){
                         break;
                     }
                     temp->agent_resource-=3;
                     if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                         temp->move(action_temp);
                     }
                     break;
                 case Staying:
                     if(gridset[temp_pos.x][temp_pos.y].build==Building::Resident){
                         if (temp->status.spirit_energy.energyValue <= 80)
                             temp->status.spirit_energy.energyValue += 20;

                         else temp->status.spirit_energy.energyValue = 100;

                         break;
                     }
                     else if(temp_resource>=5&&gridset[temp_pos.x][temp_pos.y].build==Building::Supermakert){
                         temp->agent_resource-=5;
                         if(temp->status.action_energy.energyValue<=85)
                             temp->status.action_energy.energyValue+=15;

                         else temp->status.action_energy.energyValue=100;

                         break;
                     }
                     else if(gridset[temp_pos.x][temp_pos.y].build==Building::Park){
                         if(temp->status.spirit_energy.energyValue<=90)
                             temp->status.spirit_energy.energyValue+=10;
                         else temp->status.spirit_energy.energyValue+=10;
                     }
                     else{
                         qDebug()<<temp->id<<"已无资源";
                     }

                     break;
                 case Work:
                     if(temp->status.spirit_energy.energyValue>=10){
                         if(temp->type==AgentType::Worker&&gridset[temp_pos.x][temp_pos.y].build==Building::Financialexchange)
                         {
                             if(gridset[temp_pos.x][temp_pos.y].resource>=5){
                                 if (temp->status.action_energy.energyValue <= 5)
                                     temp->status.action_energy.energyValue = 0;
                                 else temp->status.action_energy.energyValue -= 5;

                                 temp->agent_resource+=15;
                             }
                         }
                         else if(temp->type==AgentType::Residenter&&gridset[temp_pos.x][temp_pos.y].build==Building::Supermakert){
                             if(gridset[temp_pos.x][temp_pos.y].resource>=5){
                                 if (temp->status.action_energy.energyValue <= 5)
                                     temp->status.action_energy.energyValue = 0;
                                 else temp->status.action_energy.energyValue -= 5;

                                 temp->agent_resource+=15;
                             }
                         }
                         else qDebug()<<"网格世界 ("<<temp_pos.x<<","<<temp_pos.y<<") 资源枯竭";
                     }else{
                         qDebug()<<temp->id<<"已无心理能量，无法工作";
                     }
                     gridset[temp_pos.x][temp_pos.y].resource-=5;
                     temp->status.spirit_energy.energyValue-=10;
                     if (temp->status.action_energy.energyValue <= 30)temp->status.eat_Status = EatStatus::Hungry;

                     break;
                 case Interact:
                     if(temp->status.action_energy.energyValue>=5){
                         if(gridset[temp_pos.x][temp_pos.y].build==Building::Park){
                             if(temp->status.spirit_energy.energyValue<=95)
                                 temp->status.spirit_energy.energyValue+=5;
                             else temp->status.spirit_energy.energyValue+=5;
                         }
                         temp->status.action_energy.energyValue-=5;
                     }
                     break;
                 default:
                     qDebug()<<"default";
                     break;
                 }
                 //actions[QString::fromStdString(temp->id)]=Staying;
                 //temp->getPos();
                 //emit updated_world(temp);这样会信号爆炸
             }

             return {false,""};
         });
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
                gridset[i][j].resource=0;
            }else{
                int r=QRandomGenerator::global()->bounded(5);

                Building b=static_cast<Building>(r);
                if(b==Government)b=Empty;

                gridset[i][j].build=b;
                if(b!=Building::Empty||b!=Building::Park||b!=Building::Resident)
                    gridset[i][j].resource=0;
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
    //take actions
    for (size_t i = 0; i < agents.size(); i++) {
        Agent* temp = agents[i];
        Action action_temp = actions[QString::fromStdString(temp->id)];

        qDebug() << "Agent" << QString::fromStdString(temp->id) << "executes action:" << action_temp;

        Position temp_pos = temp->pos;
        int temp_resource=temp->agent_resource;
        //qDebug()<<QString::fromStdString(temp->id)<<":"<<pos.x<<","<<pos.y;
        switch (action_temp)
        {
        case MoveUp:
            temp_pos.y--;
            temp_resource-=3;
            if(temp_resource<0){
                break;
            }
            temp->agent_resource-=3;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveDown:
            temp_pos.y++;
            temp_resource-=3;
            if(temp_resource<0){
                break;
            }
            temp->agent_resource-=3;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveRight:
            temp_pos.x++;
            temp_resource-=3;
            if(temp_resource<0){
                break;
            }
            temp->agent_resource-=3;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case MoveLeft:
            temp_pos.x--;
            temp_resource-=3;
            if(temp_resource<0){
                break;
            }
            temp->agent_resource-=3;
            if (!isOutWorld(temp_pos.x, temp_pos.y)) {
                temp->move(action_temp);
            }
            break;
        case Staying:
            if(gridset[temp_pos.x][temp_pos.y].build==Building::Resident){
                if (temp->status.spirit_energy.energyValue <= 80)
                    temp->status.spirit_energy.energyValue += 20;

                else temp->status.spirit_energy.energyValue = 100;

                break;
            }
            else if(temp_resource>=5&&gridset[temp_pos.x][temp_pos.y].build==Building::Supermakert){
                temp->agent_resource-=5;
                if(temp->status.action_energy.energyValue<=85)
                    temp->status.action_energy.energyValue+=15;

                else temp->status.action_energy.energyValue=100;

                break;
            }
            else if(gridset[temp_pos.x][temp_pos.y].build==Building::Park){
                if(temp->status.spirit_energy.energyValue<=90)
                    temp->status.spirit_energy.energyValue+=10;
                else temp->status.spirit_energy.energyValue+=10;
            }
            else{
                qDebug()<<temp->id<<"已无资源";
            }

            break;
        case Work:
            if(temp->status.spirit_energy.energyValue>=10){
                if(temp->type==AgentType::Worker&&gridset[temp_pos.x][temp_pos.y].build==Building::Financialexchange)
                {
                    if(gridset[temp_pos.x][temp_pos.y].resource>=5){
                        if (temp->status.action_energy.energyValue <= 5)
                            temp->status.action_energy.energyValue = 0;
                        else temp->status.action_energy.energyValue -= 5;

                        temp->agent_resource+=15;
                    }
                }
                else if(temp->type==AgentType::Residenter&&gridset[temp_pos.x][temp_pos.y].build==Building::Supermakert){
                    if(gridset[temp_pos.x][temp_pos.y].resource>=5){
                        if (temp->status.action_energy.energyValue <= 5)
                            temp->status.action_energy.energyValue = 0;
                        else temp->status.action_energy.energyValue -= 5;

                        temp->agent_resource+=15;
                    }
                }
                else qDebug()<<"网格世界 ("<<temp_pos.x<<","<<temp_pos.y<<") 资源枯竭";
            }else{
                qDebug()<<temp->id<<"已无心理能量，无法工作";
            }
            gridset[temp_pos.x][temp_pos.y].resource-=5;
            temp->status.spirit_energy.energyValue-=10;
            if (temp->status.action_energy.energyValue <= 30)temp->status.eat_Status = EatStatus::Hungry;

            break;
        case Interact:
            if(temp->status.action_energy.energyValue>=5){
                if(gridset[temp_pos.x][temp_pos.y].build==Building::Park){
                    if(temp->status.spirit_energy.energyValue<=95)
                        temp->status.spirit_energy.energyValue+=5;
                    else temp->status.spirit_energy.energyValue+=5;
                }
                temp->status.action_energy.energyValue-=5;
            }
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
    int width=gridset.size();
    int height=gridset[0].size();

    //agent本身的信息
    QJsonArray AgentState;
    for(auto agent:agents){
        QJsonObject alldate;

        QJsonObject agentstate;
        agentstate["id"]=QString::fromStdString(agent->id);
        agentstate["type"]=agent->type;
        agentstate["x"]=agent->pos.x;
        agentstate["y"]=agent->pos.y;
        agentstate["energy"]=agent->status.action_energy.energyValue;
        agentstate["spirit"]=agent->status.spirit_energy.energyValue;
        agentstate["resource"]=agent->agent_resource;

        /*
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
        */
        alldate["AgentState"]=agentstate;
        //alldate["WorldDate"]=worlddate;

        QJsonArray functions;
        QJsonObject func1;
        func1["name"]="perceive_env";
        func1["description"]="获取指定坐标周围3x3的环境信息";
        func1["parameters"] = QJsonDocument::fromJson(
                                  R"({"type":"object","properties":{"x":{"type":"integer"},"y":{"type":"integer"}},"required":["x","y"]})"
                                  ).object();

        QJsonObject func2;
        func2["name"]="take_action";
        func2["description"]="执行指定的行动/操作";
        func2["parameters"]=QJsonDocument::fromJson(R"({"type":"object","properties":{"direction":{"type":"string","enum":["Staying","MoveUp","MoveDown","MoveRight","MoveLeft","Work","Interact"]}},"required":["direction"]})").object();

        functions.append(func1);
        functions.append(func2);

        AgentState.append(alldate);
        AgentState.append(functions);
    }
    QJsonDocument doc(AgentState);
    QString json=doc.toJson(QJsonDocument::Compact);

    m_worker->publish_Async("Agent:State",json);
}

void GridWorld::receive_AgentToolCall(redisWorker *redis)
{
    if(!redis){
        qDebug()<<"未连接";
        return;
    }

}

void GridWorld::setToolChannel()
{
    if(m_toolworker)return;
    m_toolworker=new redisWorker(this);
    if(!m_toolworker->connectToredis("127.0.0.1",6379)){
        qDebug()<<"ToolCal频道连接失败";
        return ;
    }
    m_toolworker->subscribe("Agent:ToolCall");
    connect(m_toolworker,&redisWorker::newMessage,this,[=](const QString&channel,const QString& message){
        if(channel!="Agent:ToolCall"){
            qDebug()<<"Agent：ToolCall频道订阅失败";
            return;
        }
        QJsonDocument doc=QJsonDocument::fromJson(message.toUtf8());
        if(!doc.isObject()){
            qDebug()<<"函数调用符错误";
            return ;
        }
        QJsonObject obj=doc.object();
        QString call_id=obj["call_id"].toString();
        QString func_name=obj["function_name"].toString();
        QJsonObject params=obj["arguments"].toObject();

        qDebug()<<"id:"<<call_id<<"调用:"<<func_name<<"("<<params<<")";

        FunctionResult result=functab->execute(func_name.toStdString(),params);

        QJsonObject response;
        response["call_id"]=call_id;
        if(result.success){
            response["success"]=true;
            QJsonDocument result_doc=QJsonDocument::fromJson(QByteArray::fromStdString(result.data));

            if(result_doc.isArray()){
                response["data"]=result_doc.array();
            }else if(result_doc.isObject()){
                response["data"]=result_doc.object();
            }
            else{
                response["data"]=QString::fromStdString(result.data);
            }
        }
        else{
            response["success"]=false;
            response["error"]=QString::fromStdString(result.data);

        }

        m_toolworker->publish_Async("Agent:ToolResult",QJsonDocument(response).toJson(QJsonDocument::Compact));
    });
}

