#include <ncurses.h>
#include <sys/time.h>
#include <stdlib.h>

#include "common.h"

GameState currentState = MENU;

int switchedState = 1;
int SwitchedState(void) 
{
    int s = switchedState;
    switchedState = 0;
    return s; 
}

GameState GetGameState(void)
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

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        size_t i;
        for (i = 0; i < n - 1; i++) 
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}
