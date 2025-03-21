#include <ncurses.h>

#include "common.h"

GameState currentState = MENU;

int switchedState = 1;
int SwitchedState() 
{
    int s = switchedState;
    switchedState = 0;
    return s; 
}

GameState GetGameState()
{
    return currentState;
}
void SetGameState(GameState value) 
{ 
    switchedState = 1;
    currentState = value;
}
