#include <ncurses.h>

#include "common.h"
#include "game.h"

#define PLAY_WIDTH 10
#define PLAY_HEIGHT 20
int level[PLAY_HEIGHT+2][PLAY_WIDTH];

void InitGame() 
{
    int count = 0;
    int y, x;
    for (y = 0; y < PLAY_HEIGHT; y++)
        for (x = 0; x < PLAY_WIDTH; x++) {
            level[y][x] = count++%3 == 0 ? 1 : 0;
        }
}

int UpdateGame(WINDOW* win, double deltaTime) 
{
    int drawFrame = false;

    int inp = 0;
    inp = wgetch(win);

    if (inp == 'q' || inp == 'Q')
    {
        SetGameState(MENU);
        werase(win);
        wrefresh(win);
        return false;
    }

    if (inp != -1)
        drawFrame = true;



    return drawFrame;
}

void DrawGame(WINDOW* win) 
{
    box(win, 0, 0);

    int top, left;
    getmidyx(win, PLAY_HEIGHT*2, PLAY_WIDTH*4, top, left);
    int y, x;
    for (y = 2; y < PLAY_HEIGHT * 2; y++)
        for (x = 0; x < PLAY_WIDTH * 4; x++)
            mvwaddch(win, top + y, left + x, level[y/2][x/4] ? ' '|A_REVERSE : ' ');

    wrefresh(win);
}