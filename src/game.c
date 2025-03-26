#include <ncurses.h>

#include "common.h"
#include "game.h"

#define PLAY_WIDTH 10
#define PLAY_HEIGHT 20
#define DRAW_SCALE 2

#define GAME_SPEED 0.5

int level[PLAY_HEIGHT+2][PLAY_WIDTH];
double moveDownTime = GAME_SPEED;

void MoveTilesDown();

void InitGame() 
{
    int count = 0;
    int y, x;
    for (x = 0; x < PLAY_WIDTH; x++)
        level[0][x] = 1;
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

    moveDownTime -= deltaTime;
    if (moveDownTime < 0) {
        moveDownTime = GAME_SPEED;

        MoveTilesDown();
        drawFrame = true;
    }

    return drawFrame;
}

void DrawGame(WINDOW* win) 
{
    box(win, 0, 0);

    int top, left;
    getmidyx(win, PLAY_HEIGHT*DRAW_SCALE, PLAY_WIDTH*DRAW_SCALE*2, top, left);
    int y, x;

    for (x = 0; x < PLAY_WIDTH * DRAW_SCALE * 2; x++) {
        mvwaddch(win, top - 1, left + x, ACS_HLINE);
        mvwaddch(win, top + PLAY_HEIGHT*DRAW_SCALE, left + x, ACS_HLINE);
    }
    for (y = 0; y < PLAY_WIDTH * DRAW_SCALE * 2; y++) {
        mvwaddch(win, top + y, left - 1, ACS_VLINE);
        mvwaddch(win, top + y, left + PLAY_HEIGHT * DRAW_SCALE, ACS_VLINE);
    }
    mvwaddch(win, top - 1,                      left - 1,                        ACS_ULCORNER);
    mvwaddch(win, top - 1,                      left + PLAY_HEIGHT * DRAW_SCALE, ACS_URCORNER);
    mvwaddch(win, top + PLAY_HEIGHT*DRAW_SCALE, left - 1,                        ACS_LLCORNER);
    mvwaddch(win, top + PLAY_HEIGHT*DRAW_SCALE, left + PLAY_HEIGHT * DRAW_SCALE, ACS_LRCORNER);


    for (y = 0; y < PLAY_HEIGHT * DRAW_SCALE; y++)
        for (x = 0; x < PLAY_WIDTH * DRAW_SCALE * 2; x++)
            mvwaddch(win, top + y, left + x, level[y/DRAW_SCALE][x/(DRAW_SCALE*2)] ? ' '|A_REVERSE : ' ');

    wrefresh(win);
}

void MoveTilesDown() {
    int x, y;
    for (y = PLAY_HEIGHT-1; y >= 0; y--)
        for (x = 0; x < PLAY_WIDTH; x++) {
            // Do some checking whether you can do this first
            level[y+1][x] = level[y][x];
            level[y][x] = 0;
        }
}