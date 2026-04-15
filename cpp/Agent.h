#include<iostream>
#include<string>
#include<QString>
#include<QDebug>
//ÀàÐÍ
enum AgentType {
	Manager,
	Residenter,
	Worker
};

//×´Ì¬
enum EatStatus {
	Hungry,
	Full
};
enum IngStatus {
	Working,
	Moveing,
	Interacting,
	Thinking
};
struct ActionEnergy {
public:
	int energyValue;

	ActionEnergy() {
		energyValue = 100;
	}
	ActionEnergy(int energyValue_new) {
		energyValue = energyValue_new;
	}
};
struct SpiritEnergy {
	int energyValue;

	SpiritEnergy() {
		energyValue = 100;
	}
	SpiritEnergy(int energyValue_new) {
		energyValue = energyValue_new;
	}
};
struct Status {
	IngStatus ing_Status;
	EatStatus eat_Status;
	ActionEnergy action_energy;
	SpiritEnergy spirit_energy;

	Status() {
		ing_Status = IngStatus::Thinking;
		eat_Status = EatStatus::Full;
		action_energy.energyValue=100;
		spirit_energy.energyValue = 100;
	}
	Status(const Status& sta){
		this->ing_Status = sta.ing_Status;
		this->eat_Status = sta.eat_Status;
		this->action_energy = sta.action_energy;
		this->spirit_energy = sta.spirit_energy;
	}
};

struct Position {
    int x;
    int y;

    Position() : x(0), y(0) {}

    Position(int x1, int y1) {
        x = x1;
        y = y1;
    }
    Position(const Position& pos) {
        this->x = pos.x;
        this->y = pos.y;
    }
};



enum Action {
    MoveUp,MoveDown,MoveRight,MoveLeft,Work,Interact,Staying
};

class Agent {
public:
	std::string id;
	AgentType type;
	Status status;
	Position pos;

public:
    Agent(): id(""), type(AgentType::Residenter), status(), pos(0, 0){}
	Agent(std::string id1, AgentType type1, Status status1, Position pos1) {
		id = id1;
		type = type1;
		status = status1;
		pos = pos1;
	}

    virtual Action decide(QString agent_decide_form_network);
	virtual void interact(Agent& other);
	void move(Action movedir);

    //test fuction for agent`s pos
    void getPos(){
        qDebug()<<QString::fromStdString(id)<<pos.x<<","<<pos.y;
    }
};



inline const std::string Action_to_QString(Action act){
    std::string act_str;
    switch (act) {
    case MoveUp:
        act_str="MoveUp";
        break;
    case MoveDown:
        act_str="MoveDown";
        break;
    case MoveRight:
        act_str="MoveRight";
        break;
    case MoveLeft:
        act_str="MoveLeft";
        break;
    case Work:
        act_str="Work";
        break;
    case Interact:
        act_str="Interact";
        break;
    case Staying:
        act_str="Staying";
        break;
    default:
        act_str="Staying";
        break;
    }
    return act_str;
}
