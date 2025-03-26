#include <ncurses.h>
#include <sys/time.h>

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

long long timeInMilliseconds(void) 
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}
