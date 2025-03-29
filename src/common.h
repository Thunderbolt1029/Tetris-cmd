#ifndef COMMON_H
#define COMMON_H

#include <ncurses.h>

#define mod(x) ((x) < 0 ? -(x) : (x))
#define ARRAY_LENGTH(x) (int)(sizeof(x) / sizeof((x)[0]))

#define getmidx(win, width) ((getmaxx(win) - width) / 2)
#define getmidy(win, height) ((getmaxy(win) - height) / 2)
#define getmidyx(win, height, width, y, x) (x = getmidx(win, width), y = getmidy(win, height))

#define LARGE_NUM_HEIGHT 4                                                                      

typedef enum GameState_type
{
    QUIT,
    MENU,
    GAME,
    LOST
} GameState;

typedef struct GameInfo_type
{
    int score;
    int lines;
    // time?
} GameInfo;

int SwitchedState(void);
GameState GetGameState(void);
void SetGameState(GameState, WINDOW*);

long long timeInMilliseconds(void);

void shuffle(int *array, size_t arraySize);

char *LargeNum(int num, int i);

#endif
