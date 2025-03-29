#include <ncurses.h>

#define GAME_WIDTH 100
#define GAME_HEIGHT 50

void InitGame(void);
int UpdateGame(WINDOW* gameWindow, double deltaTime);
void DrawGame(WINDOW* gameWindow);
