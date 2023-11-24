#include <stdio.h>
#include <raylib.h>


// the map of the entire game
// 1's represent walls and 0 open space

int mapX = 8;
int mapY = 8; // map is 8x8
int mapSize = 64; // each grid is 64 by 64 px

int map[] =
{
1,1,1,1,1,1,1,1,
1,0,1,0,0,0,0,1,
1,0,1,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,1,1,0,1,
1,1,1,0,0,1,0,1,
1,0,0,0,0,1,0,1,
1,1,1,1,1,1,1,1,
};

// to render in the map, the 1's represent blocks and the o's void space
void drawMap(){
    // need to iterate through row by row, treating it like a 2D array
    // treat outer loop as the row and the inner each element in said row
    for(int i = 0; i < mapY; i++){
        for(int j = 0; j < mapX; j++){
            // determine the color based on the element value
            // the i is the first element in every row of this array, and the j is each element in said row
            Color color = (map[i * mapX + j] == 1) ? WHITE : BLACK;
            // draw the rectangle, draws from top left vertex
            // each point is mapSize (64px) away from each other
            // subtract one so you can see the individual lines
            DrawRectangle(j * mapSize, i * mapSize, mapSize - 1, mapSize - 1, color);
        }
    }
}

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

// object representing a single player
struct Player{
    float x;
    float y;
    float speed;
};

// to render a player, will be based on the players x and y coordinate
void drawPlayer(float x, float y){
    DrawCircle(x, y, 10, YELLOW);
}


struct Intuition coord;
struct Player player;

int main() {
    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);

    InitWindow(510, 512, "Raycast Engine");

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

    // player starts at the center 
    player.x = coord.ORIGIN.x;
    player.y = coord.ORIGIN.y;

    player.speed = 200.00;

    // the game loop, everything that runs is within this loop
    while(!WindowShouldClose()){
        // start of the rendering phase
        BeginDrawing();
        // make background black
        ClearBackground(BLACK);


        // draw the map
        drawMap();
        // draw the player
        drawPlayer(player.x, player.y);

            
        // handling player movement, moving its pixel position based off of the speed 
        if(IsKeyDown(KEY_W))
            player.y -= player.speed * GetFrameTime();
    
        if(IsKeyDown(KEY_D))
            player.x += player.speed * GetFrameTime();

        if(IsKeyDown(KEY_S))
            player.y += player.speed * GetFrameTime();

        if(IsKeyDown(KEY_A))
            player.x -= player.speed * GetFrameTime();



        // bounds checking for the player
        if(player.x > GetScreenWidth() - 64)
            player.x = GetScreenWidth() - 64 - 10;
        if(player.x < 64)
            player.x = 64 + 10;

        if(player.y > GetScreenHeight() - 64)
            player.y = GetScreenWidth() - 64;
        if(player.y < 64)
            player.y = 64 + 10;


        // ending the rendering phase
        EndDrawing();
    }
    // to prevent leak
    CloseWindow();
    return 0;
}
