#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "game.h"
#include "lost.h"


#define PLAY_WIDTH 10
#define PLAY_HEIGHT 20
#define DRAW_SCALE 2

#define GAME_SPEED 0.5
#define BAG_SIZE 7

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

PieceType currentPieceType;
int currentRotation;
int currentPiece[4][2];

int pieceFromBag;
PieceType pieceBag[BAG_SIZE];


void MoveTilesDown(int height);
int MovePieceDown(void);
void HardDrop(void);
void MovePieceLaterally(int dir);

int RemoveClearedLines(void);

void AddPiece(PieceType);
void RotatePiece(int dir);
void RefreshBag(void);


void InitGame(void)
{
    srand(time(NULL));

    score = 0;
    linesCleared = 0;

    for (int x = 0; x < PLAY_WIDTH; x++)
        for (int y = 0; y < PLAY_HEIGHT+2; y++)
            level[y][x] = 0;

    moveDownTime = GAME_SPEED;

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

    switch (inp)
    {
    case KEY_DOWN:
        MovePieceDown();
        drawFrame = true;
        break;
    case ' ':
        HardDrop();
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
    }

    moveDownTime -= deltaTime;
    if (moveDownTime < 0)
    {
        moveDownTime = GAME_SPEED;

        if (MovePieceDown()) 
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

            AddPiece(pieceBag[pieceFromBag++]);
            if (pieceFromBag == BAG_SIZE)
            {
                pieceFromBag = 0;
                RefreshBag();
            }
        }
        drawFrame = true;
    }

    return drawFrame;
}

void DrawGame(WINDOW *win)
{
    box(win, 0, 0);

    char Score[10];
    snprintf(Score, 10, "%d", score);
    int i;
    for (i = 0; i < 10; i++)
        if (Score[i] == '\0') break;
    for (i--; i >= 0; i--)
        for (int j = 0; j < 4; j++)
                mvwprintw(win, 1+j, 3+i*4, "%s", LargeNum(Score[i]-48, j));

    int top, left;
    getmidyx(win, PLAY_HEIGHT * DRAW_SCALE, PLAY_WIDTH * DRAW_SCALE * 2, top, left);
    int y, x;

    for (x = 0; x < PLAY_WIDTH * DRAW_SCALE * 2; x++)
    {
        mvwaddch(win, top - 1, left + x, ACS_HLINE);
        mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left + x, ACS_HLINE);
    }
    for (y = 0; y < PLAY_WIDTH * DRAW_SCALE * 2; y++)
    {
        mvwaddch(win, top + y, left - 1, ACS_VLINE);
        mvwaddch(win, top + y, left + PLAY_HEIGHT * DRAW_SCALE, ACS_VLINE);
    }
    mvwaddch(win, top - 1, left - 1, ACS_ULCORNER);
    mvwaddch(win, top - 1, left + PLAY_HEIGHT * DRAW_SCALE, ACS_URCORNER);
    mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left - 1, ACS_LLCORNER);
    mvwaddch(win, top + PLAY_HEIGHT * DRAW_SCALE, left + PLAY_HEIGHT * DRAW_SCALE, ACS_LRCORNER);

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
    moveDownTime = 0;
}

void MovePieceLaterally(int dir)
{
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
        }
    
        level[currentPiece[i][0]][currentPiece[i][1]] = currentPieceType + 1;
    }
    currentRotation++;
    currentRotation %= 4;
}

void RefreshBag(void)
{
    pieceFromBag = 0;
    for (int i = 0; i < BAG_SIZE; i++)
        pieceBag[i] = i%7;

    shuffle((int*)pieceBag, BAG_SIZE);
}
