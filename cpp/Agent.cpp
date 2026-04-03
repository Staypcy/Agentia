#include"Agent.h"
#include <cstdlib>

Action Agent::decide(){
	//先实现一个简单的随机决策
	static Action actions[] = { MoveUp,MoveDown,MoveRight,MoveLeft,Staying,Work,Iteract };
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