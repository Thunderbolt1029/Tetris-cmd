#include <ncurses.h>
#include "common.h"

#define LOST_WIDTH 80
#define LOST_HEIGHT 30

int UpdateLost(WINDOW* lostWindow);
void DrawLost(WINDOW* lostWindow);

void SendGameInfo(GameInfo gameInfo);
