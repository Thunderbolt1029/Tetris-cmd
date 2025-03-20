#include <ncurses.h>
#include<unistd.h>
#include<time.h>

#define TARGET_FRAME_TIME 0.001

int frame = 0;
int running = 1;
double deltaTime = 0;

int Update();
void Draw();

int main() 
{
    initscr();

    time_t startMillis;
    double frameTime;
    while (running) {
        startMillis = time(NULL);

        if (Update()) 
            Draw();

        frameTime = difftime(time(NULL), startMillis);
        if (frameTime < TARGET_FRAME_TIME) 
            usleep((TARGET_FRAME_TIME - frameTime) * 1000000);

        deltaTime = difftime(time(NULL), startMillis);
    }
             

    endwin();

    return 0;
}

int Update() {
    int drawFrame = false;
    frame++;

    printw("%d\n", frame);
    drawFrame = true;

    return drawFrame;
}

void Draw() {
    refresh();
    clear();
}