#include <stdio.h>
#include <raylib.h>

// since raylib has its origin (0,0) in the top left, rather than the center
// this struct is converiting that into cartesian coordinates in real life
struct Intuition{
    Vector2 ORIGIN;
    Vector2 TOP_LEFT;
    Vector2 TOP_RIGHT;
    Vector2 BOTTOM_RIGHT;
    Vector2 BOTTOM_LEFT;
};

// raylib defined values needed as it only reads in radians
// DEG2RAD = pi / 180 deg
// RAD2DEG = 180 deg / pi

struct Intuition coord;
int main() {
    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);
    // making window 1000 by 800px
    InitWindow(1000, 800, "Raycast Engine");

    // instantiate cartesian coordiante values
    coord.ORIGIN.x = (GetScreenWidth() / 2.0);
    coord.ORIGIN.y = (GetScreenHeight() / 2.0);
    
    coord.TOP_RIGHT.x = GetScreenWidth();
    coord.TOP_RIGHT.y = 0.00;

    coord.TOP_LEFT.x = 0.00;
    coord.TOP_LEFT.y = 0.00;

    coord.BOTTOM_RIGHT.x = GetScreenWidth();
    coord.BOTTOM_RIGHT.y = GetScreenHeight();

    coord.BOTTOM_LEFT.x = 0.00;
    coord.BOTTOM_LEFT.y = GetScreenHeight();


    // the game loop, everything that runs is within this loop
    while(!WindowShouldClose()){
        // where you should assing variables before rendering their x and y positions


        // start of the rendering phase
        BeginDrawing();
        // show fps on screen
        // DrawFPS(50, 50);
        // ending the rendering phase
        EndDrawing();
    }
    // to prevent leak
    CloseWindow();
    return 0;
}
