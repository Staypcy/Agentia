#include"Agent.h"
#include <cstdlib>

int getEnumvalue(const QString& actions){
    if(actions=="MoveUp"){
        return 0;
    }
    if(actions=="MoveDown"){
        return 1;
    }
    if(actions=="MoveRight"){
        return 2;
    }
    if(actions=="MoveLeft"){
        return 3;
    }
    if(actions=="Staying"){
        return 4;
    }
    if(actions=="Work"){
        return 5;
    }
    if(actions=="Interact"){
        return 6;
    }
    return 4;
}
Action Agent::decide(QString agent_decide_form_network){
/*
    //先实现一个简单的随机决策
    static Action actions[] = { MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Interact };
	int index = rand() % 6;
    return actions[index];
*/
    static Action actions[]={ MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Interact };
    int agent_decide_value=getEnumvalue(agent_decide_form_network);
    switch (agent_decide_value) {
    case 0:
        return actions[0];
        break;
    case 1:
        return actions[1];
        break;
    case 2:
        return actions[2];
        break;
    case 3:
        return actions[3];
        break;
    case 4:
        return actions[4];
        break;
    case 5:
        return actions[5];
        break;
    case 6:
        return actions[6];
        break;
    default:return actions[4];
        break;
    }
}
void Agent::interact(Agent& other) {

}
void Agent::move(Action movedir) {
	switch (movedir)
	{
	default:
		break;
	case MoveRight:
		pos.x++;
		break;
	case MoveLeft:
		pos.x--;
		break;
	case MoveUp:
		pos.y--;
		break;
	case MoveDown:
		pos.y++;
		break;
	}
}
