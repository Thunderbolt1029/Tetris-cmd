#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "menu.h"
#include "game.h"
#include "lost.h"
#include "settings.h"

#define TARGET_FRAME_TIME 0.01

WINDOW *MenuWin, *GameWin, *LostWin, *SettingsWin;

double deltaTime = 0;

int noColoursFlag = 0;

int Update(void);
void Draw(void);

void ShowHelp(void);

int main(int argc, char **argv)
{
    int c;
    while (1)
    {
        int option_index = 0;
        struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"no-colour", no_argument, &noColoursFlag, 1},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "h",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 'h':
            ShowHelp();
            exit(0);
            break;

        case '?':
            ShowHelp();
            exit(EXIT_FAILURE);
            break;

        default:
            printf("?? getopt returned character code 0x%x:%c ??\n", c, c);
        }
    }

    if (optind < argc)
    {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }


    Settings[MV_LEFT] = KEY_LEFT;
    Settings[MV_RIGHT] = KEY_RIGHT;
    Settings[SPIN] = KEY_UP;
    Settings[ASPIN] = 'z';
    Settings[SDROP] = KEY_DOWN;
    Settings[HDROP] = ' ';
    Settings[HOLD] = 'c';


    setlocale(LC_ALL, "");

    initscr();

    if (!noColoursFlag)
        start_color();

    init_pair(1, COLOR_MAGENTA, COLOR_BLACK); // T
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // L - meant to be orange but yellow looks orange
    init_pair(3, COLOR_BLUE, COLOR_BLACK); // J
    init_pair(4, COLOR_CYAN, COLOR_BLACK); // I
    init_pair(5, COLOR_GREEN, COLOR_BLACK); // S
    init_pair(6, COLOR_RED, COLOR_BLACK); // Z
    init_pair(7, COLOR_WHITE, COLOR_BLACK); // O - yellow but yellow looks orange

    clear();
    // raw();
    noecho();
    curs_set(0);


    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    MenuWin = newwin(MENU_HEIGHT, MENU_WIDTH, (maxY - MENU_HEIGHT) / 2, (maxX - MENU_WIDTH) / 2);
    nodelay(MenuWin, TRUE);
    keypad(MenuWin, TRUE);
    
    GameWin = newwin(GAME_HEIGHT, GAME_WIDTH, (maxY - GAME_HEIGHT) / 2, (maxX - GAME_WIDTH) / 2);
    nodelay(GameWin, TRUE);
    keypad(GameWin, TRUE);

    LostWin = newwin(LOST_HEIGHT, LOST_WIDTH, (maxY - LOST_HEIGHT) / 2, (maxX - LOST_WIDTH) / 2);
    nodelay(LostWin, TRUE);
    keypad(LostWin, TRUE);

    SettingsWin = newwin(SETTINGS_HEIGHT, SETTINGS_WIDTH, (maxY - SETTINGS_HEIGHT) / 2, (maxX - SETTINGS_WIDTH) / 2);
    nodelay(SettingsWin, TRUE);
    keypad(SettingsWin, TRUE);


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

int Update(void)
{
    switch (GetGameState())
    {
        case QUIT:
            return false;

        case MENU:
            return UpdateMenu(MenuWin);

        case GAME:
            return UpdateGame(GameWin, deltaTime);

        case LOST:
            return UpdateLost(LostWin);

        case SETTINGS:
            return UpdateSettings(SettingsWin);
        
        default:
            SetGameState(QUIT, NULL);
            return false;
    }
}

void Draw(void)
{
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

        case LOST:
            DrawLost(LostWin);
            break;
        
        case SETTINGS:
            DrawSettings(SettingsWin);
            break;
        
        default:
            SetGameState(QUIT, NULL);
            break;
    }
}

void ShowHelp(void)
{
    printf("usage: tetris [-h | --help] [--no-colour]\n\n");
}
