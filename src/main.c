#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

#include "common.h"
#include "menu.h"
#include "game.h"

#define TARGET_FRAME_TIME 0.01

WINDOW *MenuWin, *GameWin;

double deltaTime = 0;

int Update();
void Draw();

int main()
{
    // Init
    setlocale(LC_ALL, "");

    // Setup ncurses screen
    initscr();
    clear();
    // raw();
    noecho();
    curs_set(0);

    // Init ncurses windows
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    MenuWin = newwin(MENU_HEIGHT, MENU_WIDTH, (maxY - MENU_HEIGHT) / 2, (maxX - MENU_WIDTH) / 2);
    nodelay(MenuWin, TRUE);
    keypad(MenuWin, TRUE);
    
    GameWin = newwin(GAME_HEIGHT, GAME_WIDTH, (maxY - GAME_HEIGHT) / 2, (maxX - GAME_WIDTH) / 2);
    nodelay(GameWin, TRUE);
    keypad(GameWin, TRUE);

    // Init game obj
    InitGame();

    long long start, frameTime;
    while (GetGameState() != QUIT)
    {
        start = timeInMilliseconds();

        if (Update() || SwitchedState())
            Draw();

        frameTime = timeInMilliseconds() - start;
        if (frameTime < TARGET_FRAME_TIME * 1000)
            usleep((TARGET_FRAME_TIME * 1000 - frameTime) * 1000);

        deltaTime = (double)(timeInMilliseconds() - start) / 1000;
    }

    endwin();

    return 0;
}

int Update()
{
    switch (GetGameState())
    {
        case QUIT:
            return false;

        case MENU:
            return UpdateMenu(MenuWin);

        case GAME:
            return UpdateGame(GameWin, deltaTime);
        
        default:
            SetGameState(QUIT);
            return false;
    }
}

void Draw()
{
    mvprintw(1, 1, "%f", deltaTime);
    refresh();

    switch (GetGameState())
    {
        case QUIT:
            break;

        case MENU:
            DrawMenu(MenuWin);
            break;

        case GAME:
            DrawGame(GameWin);
            break;
        
        default:
            SetGameState(QUIT);
            break;
    }
}