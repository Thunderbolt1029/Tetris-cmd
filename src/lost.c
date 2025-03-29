#include "lost.h"

GameInfo gameInfo;

int UpdateLost(WINDOW* win)
{
    int drawFrame = false;

    int inp;
    inp = wgetch(win);

    if (inp == 'q' || inp == 'Q' || inp == '\n' || inp == KEY_ENTER)
    {
        SetGameState(MENU, win);
        return false;
    }

    return drawFrame;
}

void DrawLost(WINDOW* win)
{
    int i;

	box(win, 0, 0);

    mvwprintw(win, 10, 10, "You Lost");
    mvwprintw(win, 12, 10, "Score: %d", gameInfo.score);
    mvwprintw(win, 13, 10, "Lines cleared: %d", gameInfo.lines);

    wattron(win, A_REVERSE);
    mvwprintw(win, 15, 10, "Return to Menu");
    wattroff(win, A_REVERSE);

	wrefresh(win);
}

void SendGameInfo(GameInfo _gameInfo)
{
    gameInfo = _gameInfo;
}
