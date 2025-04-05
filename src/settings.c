#include <string.h>

#include "settings.h"
#include "common.h"

static char *choices[] = {
    "Move Left",
    "Move Right",
    "Spin Clockwise",
    "Spin Anti-clockwise",
    "Soft Drop",
    "Hard Drop",
    "Hold",
    "Exit to Main Menu"
};

static int selChoice = 0;

int changingControl = false;
int controlChanging;

int UpdateSettings(WINDOW* win)
{
    int inp;
    inp = wgetch(win);

    if (inp == -1) return false;

    if (changingControl)
    {
        Settings[selChoice] = inp;
        changingControl = false;
        wclear(win);
        return true;
    }

    if (inp == 'q' || inp == 'Q')
    {
        SetGameState(MENU, win);
        return false;
    }

    switch (inp)
    {
    case KEY_UP:
        selChoice += ARRAY_LENGTH(choices)-1;
        selChoice %= ARRAY_LENGTH(choices);
        break;
    case KEY_DOWN:
        selChoice++;
        selChoice %= ARRAY_LENGTH(choices);
        break;

    case '\n':
    case KEY_ENTER:
        if (selChoice == NO_SETTINGS)
        {
            SetGameState(MENU, win);
            return false;
        }

        if (selChoice >= MV_LEFT && selChoice <= HOLD)
        {
            changingControl = true;
            controlChanging = selChoice;
            Settings[selChoice] = -2;
            wclear(win);
            return true;
        }

        break;
    }

    return true;
}

void DrawSettings(WINDOW* win)
{
    int i, x, y;

	box(win, 0, 0);
    mvwaddstr(win, 0, 2, "Settings Menu");

    mvwaddstr(win, 2, getmidx(win, 14), "-- Controls --");
    for (i = MV_LEFT; i <= HOLD; i++)
    {
        y = 3 + i;

        mvwprintw(win, y, SETTINGS_WIDTH/2 - strlen(choices[i]) - 2, "%s", choices[i]);
        mvwaddstr(win, y, SETTINGS_WIDTH/2 - 1, ":");
        if (selChoice == i) 
        {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, SETTINGS_WIDTH/2 + 1, "%s", PrettifyInput(Settings[i]));
            wattroff(win, A_REVERSE);
        }
        else
            mvwprintw(win, y, SETTINGS_WIDTH/2 + 1, "%s", PrettifyInput(Settings[i]));
    }

    if (selChoice == NO_SETTINGS) 
    {
        wattron(win, A_REVERSE);
        mvwprintw(win, y+2, getmidx(win, strlen(choices[NO_SETTINGS])), "%s", choices[NO_SETTINGS]);
        wattroff(win, A_REVERSE);
    }
    else
        mvwprintw(win, y+2, getmidx(win, strlen(choices[NO_SETTINGS])), "%s", choices[NO_SETTINGS]);
	wrefresh(win);
}
