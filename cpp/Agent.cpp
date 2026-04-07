#include"Agent.h"
#include <cstdlib>

Action Agent::decide(){
    //先实现一个简单的随机决策
    static Action actions[] = { MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Interact };
	int index = rand() % 6;
	return actions[index];
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

const std::string Action_to_QString(Action act){
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
}