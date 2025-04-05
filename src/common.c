#include <sys/time.h>
#include <stdlib.h>

#include "common.h"

#define LARGE(x, y) switch (y) { case 0: return LARGE_##x##0; case 1: return LARGE_##x##1; case 2: return LARGE_##x##2; case 3: return LARGE_##x##3; }

#define LARGE_00 " ___ "
#define LARGE_01 "|   |"
#define LARGE_02 "| | |"
#define LARGE_03 "|___|"

#define LARGE_10 "  __ "
#define LARGE_11 " /  |"
#define LARGE_12 "  | |"
#define LARGE_13 "  |_|"

#define LARGE_20 " ___ "
#define LARGE_21 "|_  |"
#define LARGE_22 "|  _|"
#define LARGE_23 "|___|"

#define LARGE_30 " ___ "
#define LARGE_31 "|_  |"
#define LARGE_32 "|_  |"
#define LARGE_33 "|___|"

#define LARGE_40 " ___ "
#define LARGE_41 "| | |"
#define LARGE_42 "|_  |"
#define LARGE_43 "  |_|"

#define LARGE_50 " ___ "
#define LARGE_51 "|  _|"
#define LARGE_52 "|_  |"
#define LARGE_53 "|___|"

#define LARGE_60 " ___ "
#define LARGE_61 "|  _|"
#define LARGE_62 "| . |"
#define LARGE_63 "|___|"

#define LARGE_70 " ___ "
#define LARGE_71 "|_  |"
#define LARGE_72 "  | |"
#define LARGE_73 "  |_|"

#define LARGE_80 " ___ "
#define LARGE_81 "| . |"
#define LARGE_82 "| . |"
#define LARGE_83 "|___|"

#define LARGE_90 " ___ "
#define LARGE_91 "| . |"
#define LARGE_92 "|_  |"
#define LARGE_93 "|___|"

int Settings[NO_SETTINGS];

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
void SetGameState(GameState value, WINDOW *win) 
{ 
    if (win != NULL)
    {
        werase(win);
        wrefresh(win);
    }
    
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

char *LargeNum(int num, int i)
{
    if (num < 0 || num > 9 || i < 0 || i > 3)
        return NULL;

    switch (num)
    {
    case 0: LARGE(0, i) break;
    case 1: LARGE(1, i) break;
    case 2: LARGE(2, i) break;
    case 3: LARGE(3, i) break;
    case 4: LARGE(4, i) break;
    case 5: LARGE(5, i) break;
    case 6: LARGE(6, i) break;
    case 7: LARGE(7, i) break;
    case 8: LARGE(8, i) break;
    case 9: LARGE(9, i) break;
    }
}

int buffI = 0;
char buff[20];
char *PrettifyInput(int input)
{
    if (input == -2) return "Changing";

    if (input >= 33 && input <= 126) 
    {
        buffI += 2;
        buffI %= 20;
        char *s = buff+buffI;
        s[0] = input;
        s[1] = '\0';
        return s;
    }
    if (input >= KEY_F0 && input <= KEY_F(12)) return "F Key";

    switch (input)
    {
    case '\n': return "Enter";
    case ' ': return "Space";

    case KEY_BACKSPACE: return "Backspace";
    case KEY_DL: return "Delete";
    case KEY_DOWN: return "Down Arrow";
    case KEY_END: return "End";
    case KEY_ENTER: return "Enter";
    case KEY_HOME: return "Home";
    case KEY_IC: return "Insert";
    case KEY_LEFT: return "Left Arrow";
    case KEY_MOUSE: return "Mouse";
    case KEY_NPAGE: return "Page Down";
    case KEY_PPAGE: return "Page Up";
    case KEY_RESIZE: return "Resized";
    case KEY_RIGHT: return "Right Arrow";
    case KEY_UP: return "Up Arrow";
    
    default: return "?";
    }
}
