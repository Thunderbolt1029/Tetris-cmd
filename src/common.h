#define mod(x) ((x) < 0 ? -(x) : (x))
#define ARRAY_LENGTH(x) (int)(sizeof(x) / sizeof((x)[0]))

#define getmidx(win, width) ((getmaxx(win) - width) / 2)
#define getmidy(win, height) ((getmaxy(win) - height) / 2)
#define getmidyx(win, height, width, y, x) (x = getmidx(win, width), y = getmidy(win, height))

typedef enum GameState_type
{
    QUIT,
    MENU,
    GAME
} GameState;

int SwitchedState();
GameState GetGameState();
void SetGameState(GameState);

long long timeInMilliseconds(void);
