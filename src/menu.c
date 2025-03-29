#include <string.h>

#include "menu.h"
#include "common.h"

#define TETRIS_WIDTH 54
#define TETRIS_HEIGHT 8 
#define TETRIS_0 "_________ _______ _________ _______ _________ _______ "
#define TETRIS_1 "\\__   __/(  ____ \\\\__   __/(  ____ )\\__   __/(  ____ \\"
#define TETRIS_2 "   ) (   | (    \\/   ) (   | (    )|   ) (   | (    \\/"
#define TETRIS_3 "   | |   | (__       | |   | (____)|   | |   | (_____ "
#define TETRIS_4 "   | |   |  __)      | |   |     __)   | |   (_____  )"
#define TETRIS_5 "   | |   | (         | |   | (\\ (      | |         ) |"
#define TETRIS_6 "   | |   | (____/\\   | |   | ) \\ \\_____) (___/\\____) |"
#define TETRIS_7 "   )_(   (_______/   )_(   |/   \\__/\\_______/\\_______)"

char *choices[] = {
    "Start Game",
    "Quit"
};
GameState choiceStates[] = { GAME, QUIT };

int selChoice = 0;
int first = true;

int UpdateMenu(WINDOW* win)
{
    int drawFrame = false;

    int inp;
    inp = wgetch(win);

    if (inp == 'q' || inp == 'Q')
    {
        SetGameState(QUIT, win);
        return false;
    }

    if (inp != -1)
        drawFrame = true;

    switch (inp)
    {
    case KEY_UP:
        selChoice--;
        selChoice = mod(selChoice);
        break;
    case KEY_DOWN:
        selChoice++;
        selChoice %= ARRAY_LENGTH(choices);
        break;

    case '\n':
    case KEY_ENTER:
        SetGameState(choiceStates[selChoice], win);
        break;
    }

    return drawFrame;
}

void DrawMenu(WINDOW* win)
{
    int i;

	box(win, 0, 0);

    int x;
    x = getmidx(win, TETRIS_WIDTH);
    mvwprintw(win, 1, x, TETRIS_0);
    mvwprintw(win, 2, x, TETRIS_1);
    mvwprintw(win, 3, x, TETRIS_2);
    mvwprintw(win, 4, x, TETRIS_3);
    mvwprintw(win, 5, x, TETRIS_4);
    mvwprintw(win, 6, x, TETRIS_5);
    mvwprintw(win, 7, x, TETRIS_6);
    mvwprintw(win, 8, x, TETRIS_7);

    int y = 12;
	for (i = 0; i < ARRAY_LENGTH(choices); i++)
	{
        if (selChoice == i) 
        {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, getmidx(win, strlen(choices[i])), "%s", choices[i]);
            wattroff(win, A_REVERSE);
        }
        else
            mvwprintw(win, y, getmidx(win, strlen(choices[i])), "%s", choices[i]);
		++y;
	}
	wrefresh(win);
}
