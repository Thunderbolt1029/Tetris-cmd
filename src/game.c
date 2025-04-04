#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "common.h"
#include "game.h"
#include "lost.h"


#define PLAY_WIDTH 10
#define PLAY_HEIGHT 20
#define DRAW_SCALE 2

#define GAME_SPEED 0.5
#define GAME_SPEED_RAMPING (float)10
#define FLOOR_FRAMES 1
#define BAG_SIZE 14

#define HOLD_SIZE 5
#define HOLD_X 3
#define HOLD_Y 0

#define PREDICT_COUNT 5
#define PREDICT_X 3
#define PREDICT_Y 7

#define LINE_SCORE_1 40
#define LINE_SCORE_2 100
#define LINE_SCORE_3 300
#define LINE_SCORE_4 1200
int LineScore(int noLines)
{
    switch (noLines)
    {
        case 0: return 0;
        case 1: return LINE_SCORE_1;
        case 2: return LINE_SCORE_2;
        case 3: return LINE_SCORE_3;
        case 4: return LINE_SCORE_4;
        
        default: return -1;
    }
}

typedef enum PieceType_type {
    NONE = -1,

    T_PIECE,
    L_PIECE,
    J_PIECE,
    I_PIECE,
    S_PIECE,
    Z_PIECE,
    O_PIECE
} PieceType;


int score, linesCleared;
int level[PLAY_HEIGHT + 2][PLAY_WIDTH];
double moveDownTime;
int touchFloorCount;

PieceType currentPieceType;
int currentRotation;
int currentPiece[4][2];

int pieceFromBag;
PieceType pieceBag[BAG_SIZE];
PieceType nextBag[BAG_SIZE];

PieceType heldPiece;
int heldFrame;


void MoveTilesDown(int height);
int MovePieceDown(void);
void HardDrop(void);
void MovePieceLaterally(int dir);

int RemoveClearedLines(void);

void AddPiece(PieceType);
void RotatePiece(int dir);
void RefreshBag(void);

void DrawPiece(WINDOW* win, PieceType piece, int y, int x, int scale);

void HoldPiece(void);
void AddPieceFromBag(void);


void InitGame(void)
{
    srand(time(NULL));

    score = 0;
    linesCleared = 0;
    touchFloorCount = FLOOR_FRAMES;

    for (int x = 0; x < PLAY_WIDTH; x++)
        for (int y = 0; y < PLAY_HEIGHT+2; y++)
            level[y][x] = 0;

    moveDownTime = GAME_SPEED;

    heldPiece = NONE;
    heldFrame = false;

    RefreshBag();
    RefreshBag();
    AddPiece(pieceBag[pieceFromBag++]);
}

int UpdateGame(WINDOW *win, double deltaTime)
{
    int drawFrame = false;

    int inp;
    inp = wgetch(win);

    if (inp == 'q' || inp == 'Q')
    {
        InitGame();
        SetGameState(MENU, win);
        return false;
    }

    int hard = false;
    switch (inp)
    {
    case KEY_DOWN:
        MovePieceDown();
        drawFrame = true;
        break;
    case ' ':
        HardDrop();
        hard = true;
        drawFrame = true;
        break;

    case KEY_LEFT:
        MovePieceLaterally(-1);
        drawFrame = true;
        break;
    case KEY_RIGHT:
        MovePieceLaterally(1);
        drawFrame = true;
        break;

    case KEY_UP:
        RotatePiece(1);
        drawFrame = true;
        break;

    case 'c':
    case 'C':
        HoldPiece();
        drawFrame = true;
        break;
    }

    moveDownTime -= deltaTime;
    if (moveDownTime < 0 || hard)
    {
        moveDownTime = GAME_SPEED * exp(-(float)linesCleared * GAME_SPEED_RAMPING / 1000);

        int movePieceDown = MovePieceDown(); 
        if (movePieceDown && (touchFloorCount-- <= 0 || hard)) 
        {
            int lines;
            lines = RemoveClearedLines();
            if (lines > 0) RemoveClearedLines();
            score += LineScore(lines);
            linesCleared += lines;

            int lost = false;
            for (int x = 0; x < PLAY_WIDTH; x++)
                if (level[0][x] || level[1][x])
                {
                    lost = true;
                    break;
                }
            if (lost)
            {
                SendGameInfo((GameInfo){score, linesCleared});
                InitGame();
                SetGameState(LOST, win);
                return false;
            }

            touchFloorCount == FLOOR_FRAMES;
            heldFrame = false;

            AddPieceFromBag();
        }
        if (!movePieceDown) touchFloorCount = FLOOR_FRAMES;
        drawFrame = true;
    }

    return drawFrame;
}

void DrawGame(WINDOW *win)
{
    int x, y, i;
    box(win, 0, 0);
    // mvwaddstr(win, 0, 2, "Game window");

    int top, left, right;
    getmidyx(win, PLAY_HEIGHT * DRAW_SCALE, PLAY_WIDTH * DRAW_SCALE * 2, top, left);
    right = left + PLAY_WIDTH * DRAW_SCALE * 2;


    char Score[10];
    snprintf(Score, 10, "%d", score);
    for (i = 0; i < 10; i++)
        if (Score[i] == '\0') break;
    for (i--; i >= 0; i--)
        for (int j = 0; j < 4; j++)
                mvwprintw(win, 1+j, 3+i*4, "%s", LargeNum(Score[i]-48, j));

    for (i = 0; i < HOLD_SIZE; i++)
    {
        mvwaddch(win, top+HOLD_Y,                 right+HOLD_X + 2*i,   ACS_HLINE);
        mvwaddch(win, top+HOLD_Y,                 right+HOLD_X + 2*i+1, ACS_HLINE);
        mvwaddch(win, top+HOLD_Y + HOLD_SIZE, right+HOLD_X + 2*i,   ACS_HLINE);
        mvwaddch(win, top+HOLD_Y + HOLD_SIZE, right+HOLD_X + 2*i+1, ACS_HLINE);

        mvwaddch(win, top + i, right+HOLD_X,                   ACS_VLINE);
        mvwaddch(win, top + i, right+HOLD_X + HOLD_SIZE*2+1, ACS_VLINE);
    }
    mvwaddch(win, top+HOLD_Y,                 right+HOLD_X + HOLD_SIZE*2, ACS_HLINE);
    mvwaddch(win, top+HOLD_Y + HOLD_SIZE, right+HOLD_X + HOLD_SIZE*2, ACS_HLINE);

    mvwaddch(win, top+HOLD_Y,                 right+HOLD_X,                   ACS_ULCORNER);
    mvwaddch(win, top+HOLD_Y,                 right+HOLD_X + HOLD_SIZE*2+1, ACS_URCORNER);
    mvwaddch(win, top+HOLD_Y + HOLD_SIZE, right+HOLD_X,                   ACS_LLCORNER);
    mvwaddch(win, top+HOLD_Y + HOLD_SIZE, right+HOLD_X + HOLD_SIZE*2+1, ACS_LRCORNER);
    mvwprintw(win, top+HOLD_Y, right+HOLD_X+2, "Held");
    
    for (y = 0; y < HOLD_SIZE-1; y++)
            for (x = 0; x < HOLD_SIZE*DRAW_SCALE-1; x++)
                mvwaddch(win, top+HOLD_Y + 1 + y, right+HOLD_X + 1 + x, ' ');

    if (heldPiece != -1)
        DrawPiece(win, heldPiece, top+HOLD_Y + 2, right+HOLD_X + 3, 1);


    for (x = 0; x < 4*DRAW_SCALE+3; x++)
    {
        mvwaddch(win, top+PREDICT_Y, right+PREDICT_X + x, ACS_HLINE);
        mvwaddch(win, top+PREDICT_Y + PREDICT_COUNT*3+2, right+PREDICT_X + x, ACS_HLINE);
    }
    for (y = 0; y < PREDICT_COUNT*3+2; y++)
    {
        mvwaddch(win, top+PREDICT_Y + y, right+PREDICT_X, ACS_VLINE);
        mvwaddch(win, top+PREDICT_Y + y, right+PREDICT_X + 4*DRAW_SCALE+3, ACS_VLINE);
    }
    mvwaddch(win, top+PREDICT_Y,                     right+PREDICT_X,                  ACS_ULCORNER);
    mvwaddch(win, top+PREDICT_Y,                     right+PREDICT_X + 4*DRAW_SCALE+3, ACS_URCORNER);
    mvwaddch(win, top+PREDICT_Y + PREDICT_COUNT*3+2, right+PREDICT_X,                  ACS_LLCORNER);
    mvwaddch(win, top+PREDICT_Y + PREDICT_COUNT*3+2, right+PREDICT_X + 4*DRAW_SCALE+3, ACS_LRCORNER);

    for (y = 0; y < PREDICT_COUNT*3+1; y++)
        for (x = 0; x < 4*DRAW_SCALE+2; x++)
            mvwaddch(win, top+PREDICT_Y+1 + y, right+PREDICT_X+1 + x, ' ');
    for (i = 0; i < PREDICT_COUNT; i++)
    {

        PieceType nextPieceType = pieceFromBag+i >= BAG_SIZE ? nextBag[(pieceFromBag+i) % BAG_SIZE] : pieceBag[pieceFromBag+i];
        wattron(win, COLOR_PAIR(nextPieceType+1));
        DrawPiece(win, nextPieceType, top+PREDICT_Y+2 + i*3, right+PREDICT_X+3, 1);
        wattroff(win, COLOR_PAIR(nextPieceType+1));
    }
    mvwprintw(win, top+PREDICT_Y, right+PREDICT_X+2, "Upcoming");


    for (x = 0; x < PLAY_WIDTH * DRAW_SCALE * 2; x++)
    {
        mvwaddch(win, top - 1, left + x, ACS_HLINE);
        mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left + x, ACS_HLINE);
    }
    for (y = 0; y < PLAY_HEIGHT * DRAW_SCALE; y++)
    {
        mvwaddch(win, top + y, left - 1, ACS_VLINE);
        mvwaddch(win, top + y, left + PLAY_HEIGHT * DRAW_SCALE, ACS_VLINE);
    }
    mvwaddch(win, top - 1, left - 1, ACS_ULCORNER);
    mvwaddch(win, top - 1, left + PLAY_HEIGHT * DRAW_SCALE, ACS_URCORNER);
    mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left - 1, ACS_LLCORNER);
    mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left + PLAY_HEIGHT * DRAW_SCALE, ACS_LRCORNER);

    // mvwaddstr(win, top - 1, left + 1, "Player 1");

    for (y = 0; y < PLAY_HEIGHT * DRAW_SCALE; y++)
        for (x = 0; x < PLAY_WIDTH * DRAW_SCALE * 2; x++)
        {
            wattron(win, COLOR_PAIR(level[y / DRAW_SCALE + 2][x / (DRAW_SCALE * 2)]));
            mvwaddch(win, top + y, left + x, level[y / DRAW_SCALE + 2][x / (DRAW_SCALE * 2)] ? ' ' | A_REVERSE : ' ');
            wattroff(win, COLOR_PAIR(level[y / DRAW_SCALE + 2][x / (DRAW_SCALE * 2)]));
        }

    wrefresh(win);
}

void MoveTilesDown(int height)
{
    for (int y = height; y >= 0; y--)
        for (int x = 0; x < PLAY_WIDTH; x++)
            if (!level[y + 1][x])
            {
                level[y + 1][x] = level[y][x];
                level[y][x] = 0;
            }
}

int MovePieceDown(void)
{
    int x, y, i;

    int levelCopy[PLAY_HEIGHT + 2][PLAY_WIDTH];
    for (y = 0; y < PLAY_HEIGHT+2; y++)
        for (x = 0; x < PLAY_WIDTH; x++)
            levelCopy[y][x] = level[y][x];

    for (i = 0; i < 4; i++)
        levelCopy[currentPiece[i][0]][currentPiece[i][1]] = 0;

    for (i = 0; i < 4; i++)
    {
        y = currentPiece[i][0], x = currentPiece[i][1];
        if (y > PLAY_HEIGHT || levelCopy[y + 1][x]) return true;
    }

    for (i = 0; i < 4; i++)
        level[currentPiece[i][0]][currentPiece[i][1]] = 0;
    for (i = 0; i < 4; i++)
        level[++currentPiece[i][0]][currentPiece[i][1]] = currentPieceType + 1;

    return false;
}

void HardDrop(void)
{
    while (!MovePieceDown()) {}
}

void MovePieceLaterally(int dir)
{
    touchFloorCount == FLOOR_FRAMES;

    int x, y, i;

    for (i = 0; i < 4; i++)
    {
        x = currentPiece[i][1] + dir;
        if (x < 0 || x >= PLAY_WIDTH)
            return;
    }

    int levelCopy[PLAY_HEIGHT + 2][PLAY_WIDTH];
    for (y = 0; y < PLAY_HEIGHT+2; y++)
        for (x = 0; x < PLAY_WIDTH; x++)
            levelCopy[y][x] = level[y][x];

    for (i = 0; i < 4; i++)
        levelCopy[currentPiece[i][0]][currentPiece[i][1]] = 0;
    for (i = 0; i < 4; i++)
    {
        y = currentPiece[i][0], x = currentPiece[i][1];

        if (levelCopy[y][x+dir])
            return;
    }

    for (i = 0; i < 4; i++)
        level[currentPiece[i][0]][currentPiece[i][1]] = 0;
    for (i = 0; i < 4; i++)
    {
        y = currentPiece[i][0], x = currentPiece[i][1];

        currentPiece[i][1] += dir;
        level[y][x+dir] = currentPieceType + 1;
    }
}

int RemoveClearedLines(void)
{
    int x, y, i, skip, noLines = 0, bottomLine = 0;

    // Check for full lines
    for (y = 0; y < PLAY_HEIGHT+2; y++)
    {
        skip = false;
        for (x = 0; x < PLAY_WIDTH; x++)
        {
            if (!level[y][x])
            {
                skip = true;
                break;
            }
        }
        if (skip) continue;

        noLines++;
        bottomLine = y;
    }

    // Clear lines
    for (y = bottomLine; y > bottomLine - noLines; y--)
        for (x = 0; x < PLAY_WIDTH; x++)
            level[y][x] = 0;

    // Drop lines
    for (i = 0; i < noLines; i++)
        MoveTilesDown(bottomLine);

    return noLines;
}

void AddPiece(PieceType piece)
{
    currentPieceType = piece;
    currentRotation = 0;

    switch (piece)
    {
    case T_PIECE:
        currentPiece[0][0] = 0;
        currentPiece[0][1] = 5;

        currentPiece[1][0] = 1;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 5;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 6;
        break;

    case L_PIECE:
        currentPiece[0][0] = 0;
        currentPiece[0][1] = 6;

        currentPiece[1][0] = 1;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 5;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 6;
        break;

    case J_PIECE:
        currentPiece[0][0] = 0;
        currentPiece[0][1] = 4;

        currentPiece[1][0] = 1;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 5;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 6;
        break;

    case I_PIECE:
        currentPiece[0][0] = 1;
        currentPiece[0][1] = 3;

        currentPiece[1][0] = 1;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 5;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 6;
        break;

    case S_PIECE:
        currentPiece[0][0] = 0;
        currentPiece[0][1] = 5;

        currentPiece[1][0] = 1;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 5;

        currentPiece[3][0] = 0;
        currentPiece[3][1] = 6;
        break;

    case Z_PIECE:
        currentPiece[0][0] = 0;
        currentPiece[0][1] = 4;

        currentPiece[1][0] = 0;
        currentPiece[1][1] = 3;

        currentPiece[2][0] = 1;
        currentPiece[2][1] = 4;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 5;
        break;

    case O_PIECE:
        currentPiece[1][0] = 0;
        currentPiece[1][1] = 4;

        currentPiece[2][0] = 0;
        currentPiece[2][1] = 5;

        currentPiece[0][0] = 1;
        currentPiece[0][1] = 4;

        currentPiece[3][0] = 1;
        currentPiece[3][1] = 5;
        break;
    }

    for (int i = 0; i < 4; i++)
        level[currentPiece[i][0]][currentPiece[i][1]] = currentPieceType + 1;
}

const int CLOCK_ROTATE_DISPLACEMENTS[7][4][4][2] = {
    // T_PIECE
    {
        // 0->1
        { {1, 1}, {1, -1}, {0, 0}, {-1, 1} },

        // 1->2
        { {-1, 1}, {1, 1}, {0, 0}, {-1, -1} },

        // 2->3
        { {-1, -1}, {-1, 1}, {0, 0}, {1, -1} },

        // 3->0
        { {1, -1}, {-1, -1}, {0, 0}, {1, 1} }
    },

    // L_PIECE
    {
        // 0->1
        { {0, 2}, {1, -1}, {0, 0}, {-1, 1} },

        // 1->2
        { {-2, 0}, {1, 1}, {0, 0}, {-1, -1} },

        // 2->3
        { {0, -2}, {-1, 1}, {0, 0}, {1, -1} },

        // 3->0
        { {2, 0}, {-1, -1}, {0, 0}, {1, 1} }
    },

    // J_PIECE
    {
        // 0->1
        { {2, 0}, {1, -1}, {0, 0}, {-1, 1} },

        // 1->2
        { {0, 2}, {1, 1}, {0, 0}, {-1, -1} },

        // 2->3
        { {-2, 0}, {-1, 1}, {0, 0}, {1, -1} },

        // 3->0
        { {0, -2}, {-1, -1}, {0, 0}, {1, 1} }
    },

    // I_PIECE
    {
        // 0->1
        { {2, -1}, {1, 0}, {0, 1}, {-1, 2} },

        // 1->2
        { {1, 2}, {0, 1}, {-1, 0}, {-2, -1} },

        // 2->3
        { {-2, 1}, {-1, 0}, {0, -1}, {1, -2} },

        // 3->0
        { {-1, -2}, {0, -1}, {1, 0}, {2, 1} }
    },

    // S_PIECE
    {
        // 0->1
        { {1, 1}, {1, -1}, {0, 0}, {0, 2} },

        // 1->2
        { {-1, 1}, {1, 1}, {0, 0}, {-2, 0} },

        // 2->3
        { {-1, -1}, {-1, 1}, {0, 0}, {0, -2} },

        // 3->0
        { {1, -1}, {-1, -1}, {0, 0}, {2, 0} }
    },

    // Z_PIECE
    {
        // 0->1
        { {1, 1}, {2, 0}, {0, 0}, {-1, 1} },

        // 1->2
        { {-1, 1}, {0, 2}, {0, 0}, {-1, -1} },

        // 2->3
        { {-1, -1}, {-2, 0}, {0, 0}, {1, -1} },

        // 3->0
        { {1, -1}, {0, -2}, {0, 0}, {1, 1} }
    },

    // O_PIECE
    {
        // 0->1
        { {0, 0}, {0, 0}, {0, 0}, {0, 0} },

        // 1->2
        { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
        
        // 2->3
        { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
        
        // 3->0
        { {0, 0}, {0, 0}, {0, 0}, {0, 0} }
    }
};
const int KICK_CHECKS[2][4][4][2] = {
    // J, L, T, S, Z
    {
        // 0->1
        {
            // Test 2
            { -1, 0 },

            // Test 3
            { -1, 1 },

            // Test 4
            { 0, -2 },
            
            // Test 5
            { -1, -2 }
        },

        // 1->2
        {
            // Test 2
            { 1, 0 },

            // Test 3
            { 1, -1 },

            // Test 4
            { 0, 2 },
            
            // Test 5
            { 1, 2 }
        },

        // 2->3
        {
            // Test 2
            { 1, 0 },

            // Test 3
            { 1, 1 },

            // Test 4
            { 0, -2 },
            
            // Test 5
            { 1, -2 }
        },

        // 3->0
        {
            // Test 2
            { -1, 0 },

            // Test 3
            { -1, -1 },

            // Test 4
            { 0, 2 },
            
            // Test 5
            { -1, 2 }
        }
    },
    // I
    {
        // 0->1
        {
            // Test 2
            { -2, 0 },

            // Test 3
            { 1, 0 },

            // Test 4
            { -2, -1 },
            
            // Test 5
            { 1, 2 }
        },

        // 1->2
        {
            // Test 2
            { -1, 0 },

            // Test 3
            { 2, 0 },

            // Test 4
            { -1, 2 },
            
            // Test 5
            { 2, -1 }
        },

        // 2->3
        {
            // Test 2
            { 2, 0 },

            // Test 3
            { -1, 0 },

            // Test 4
            { 2, 1 },
            
            // Test 5
            { -1, -2 }
        },

        // 3->0
        {
            // Test 2
            { 1, 0 },

            // Test 3
            { -2, 0 },

            // Test 4
            { 1, -2 },
            
            // Test 5
            { -2, 1 }
        }
    }
};
void RotatePiece(int dir)
{
    int x, y, i;

    int height = 0;
    for (i = 0; i < 4; i++)
        if (currentPiece[i][0] > height)
            height = currentPiece[i][0];

    int levelCopy[PLAY_HEIGHT + 2][PLAY_WIDTH];
    for (y = 0; y < PLAY_HEIGHT+2; y++)
        for (x = 0; x < PLAY_WIDTH; x++)
            levelCopy[y][x] = level[y][x];
    
    int kickTest;
    for (kickTest = -1; kickTest < 4; kickTest++)
    {
        int pieceCopy[4][2];
        for (i = 0; i < 4; i++)
        {
            pieceCopy[i][0] = currentPiece[i][0];
            pieceCopy[i][1] = currentPiece[i][1];
        }

        for (i = 0; i < 4; i++)
            levelCopy[pieceCopy[i][0]][pieceCopy[i][1]] = 0;

        int valid = true;
        for (i = 0; i < 4; i++)
        {
            pieceCopy[i][0] += CLOCK_ROTATE_DISPLACEMENTS[currentPieceType][currentRotation][i][1];
            pieceCopy[i][1] += CLOCK_ROTATE_DISPLACEMENTS[currentPieceType][currentRotation][i][0];
        
            if (kickTest != -1 && currentPieceType != O_PIECE) 
            {
                pieceCopy[i][0] += KICK_CHECKS[currentPieceType == I_PIECE][currentRotation][kickTest][1];
                pieceCopy[i][1] += KICK_CHECKS[currentPieceType == I_PIECE][currentRotation][kickTest][0];
            }

            int oobKick = pieceCopy[i][0] < 0 || pieceCopy[i][0] >= PLAY_HEIGHT + 2 || pieceCopy[i][1] < 0 || pieceCopy[i][1] >= PLAY_WIDTH;
            if (oobKick || levelCopy[pieceCopy[i][0]][pieceCopy[i][1]])
            {
                valid = false;
                break;
            }
        }
        if (valid)
            break;
    }
    if (kickTest == 4)
        return;

    int floorKick = false;
    for (i = 0; i < 4; i++)
        level[currentPiece[i][0]][currentPiece[i][1]] = 0;
    for (i = 0; i < 4; i++)
    {
        currentPiece[i][0] += CLOCK_ROTATE_DISPLACEMENTS[currentPieceType][currentRotation][i][1];
        currentPiece[i][1] += CLOCK_ROTATE_DISPLACEMENTS[currentPieceType][currentRotation][i][0];

        if (kickTest != -1 && currentPieceType != O_PIECE) 
        {
            currentPiece[i][0] += KICK_CHECKS[currentPieceType == I_PIECE][currentRotation][kickTest][1];
            currentPiece[i][1] += KICK_CHECKS[currentPieceType == I_PIECE][currentRotation][kickTest][0];

            floorKick = KICK_CHECKS[currentPieceType == I_PIECE][currentRotation][kickTest][1] < 0;
        }
    
        level[currentPiece[i][0]][currentPiece[i][1]] = currentPieceType + 1;
    }
    currentRotation++;
    currentRotation %= 4;

    touchFloorCount == FLOOR_FRAMES; // change?
}

void RefreshBag(void)
{
    int i;

    pieceFromBag = 0;

    for (i = 0; i < BAG_SIZE; i++)
        pieceBag[i] = nextBag[i];

    for (i = 0; i < BAG_SIZE; i++)
        nextBag[i] = i%7;
    shuffle((int*)nextBag, BAG_SIZE);
}

void DrawPiece(WINDOW* win, PieceType piece, int y, int x, int scale)
{
    wattron(win, COLOR_PAIR(piece+1));

    for (int dx = 0; dx < scale*2; dx++)
        for (int dy = 0; dy < scale; dy++)
    switch (piece)
    {
    case T_PIECE:
        mvwaddch(win, y+dy+1*scale, x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+2*scale*2, ' ' | A_REVERSE);
        break;

    case L_PIECE:
        mvwaddch(win, y+dy+1*scale, x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+2*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+dx+2*scale*2, ' ' | A_REVERSE);
        break;

    case J_PIECE:
        mvwaddch(win, y+dy,         x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+2*scale*2, ' ' | A_REVERSE);
        break;

    case I_PIECE:
        mvwaddch(win, y+dy+1*scale, x-scale+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x-scale+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x-scale+dx+2*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x-scale+dx+3*scale*2, ' ' | A_REVERSE);
        break;

    case S_PIECE:
        mvwaddch(win, y+dy+1*scale, x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+dx+2*scale*2, ' ' | A_REVERSE);
        break;

    case Z_PIECE:
        mvwaddch(win, y+dy,         x+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+dx+2*scale*2, ' ' | A_REVERSE);
        break;

    case O_PIECE:
        mvwaddch(win, y+dy,         x+scale+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy,         x+scale+dx+1*scale*2, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+scale+dx, ' ' | A_REVERSE);
        mvwaddch(win, y+dy+1*scale, x+scale+dx+1*scale*2, ' ' | A_REVERSE);
        break;
    }

    wattroff(win, COLOR_PAIR(piece+1));
}

void HoldPiece(void)
{
    if (heldFrame) return;

    PieceType temp = heldPiece;
    heldPiece = currentPieceType;

    for (int i = 0; i < 4; i++)
        level[currentPiece[i][0]][currentPiece[i][1]] = 0;

    if (temp == NONE)
        AddPieceFromBag();
    else
        AddPiece(temp);

    heldFrame = true;
}

void AddPieceFromBag(void)
{
    AddPiece(pieceBag[pieceFromBag++]);
    if (pieceFromBag == BAG_SIZE)
    {
        pieceFromBag = 0;
        RefreshBag();
    }
}
