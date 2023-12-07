#include <stdio.h>
#include <raylib.h>
#include<math.h>
#define mapSize 64

// the map of the entire game
// 1's represent walls and 0 open space
int mapX = 8;
int mapY = 8; // map is 8x8
// the bounded walls in the array, denoted with a 1 in the int array
Rectangle walls[mapSize];
int map[] =
{
1,1,1,1,1,1,1,1,
1,0,1,0,0,0,0,1,
1,0,1,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,1,1,0,0,0,0,1,
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
           // initalizing the reactangles of the map for collision checking
           if(map[i * mapX + j] == 1){
               walls[i * mapX + j].x = j * mapSize;
               walls[i * mapX + j].y = i * mapSize;
               walls[i * mapX + j].width = mapSize;
               walls[i * mapX + j].height = mapSize;
           }
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

// object representing a single player
struct Player{
    // the positioning of the player on the screen 
    float x,y;
    float speed;
    // the angle, x, and y components of the players movement
    float angle, turnSpeed, dx, dy;
    int radius;
};

// to render a player, will be based on the players x and y coordinate
void drawPlayer(struct Player player){

    // draw circle based on player coords
    DrawCircle(player.x, player.y, player.radius, BLUE);
    // draw line so we know where we are turning, have to add player.x/y to endpoint bc we want to end from there and add by the components
    DrawLine(player.x, player.y, player.x + player.dx * 30, player.y + player.dy * 30, BLUE);
    // x component
    DrawLine(player.x, player.y, player.x + player.dx * 30, player.y, RED);
    // y component
    DrawLine(player.x, player.y, player.x, player.y + player.dy * 30, YELLOW);

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
    player.radius = 15;
    player.angle = 90.00;
    player.turnSpeed = 5.00;
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
        drawPlayer(player);

        // handling player movement, moving its pixel position based off of the speed
        if(IsKeyDown(KEY_W)){
            // we have to move by the componets of x and y
            player.x += player.dx * GetFrameTime() * player.speed;
            player.y += player.dy * GetFrameTime() * player.speed;

            // checking the collision for every wall
            for(int i = 0; i < mapSize; i++){
                if (CheckCollisionCircleRec((Vector2){player.x, player.y}, player.radius, walls[i])){
                    // moves the player in the oppositte direction as the line shown on it
                    player.x -= player.dx * GetFrameTime() * player.speed;
                    player.y -= player.dy * GetFrameTime() * player.speed;
                }
            }
            
        }
    
        if(IsKeyDown(KEY_S)){
            // move backwards based on the x and y components of the players angle
            player.y -= player.dy * player.speed * GetFrameTime();
            player.x -= player.dx * player.speed * GetFrameTime();  

            // checking the collisions of the walls
            for(int i = 0; i < mapSize; i++){
                if(CheckCollisionCircleRec((Vector2){player.x, player.y}, player.radius, walls[i])){
                    // move player
                    player.x += player.dx * GetFrameTime() * player.speed;
                    player.y += player.dy * GetFrameTime() * player.speed;
                }
            }
        }

        // On "A" and "D" keys, the player turns
        // update the x and y component as the player is turning (x is cos and y is sin)
        if(IsKeyDown(KEY_D)){
            //    player.x += player.speed * GetFrameTime();
            // as you press, turn right, so decrese the angle
            player.angle += player.turnSpeed;
            // update the x and y coordinates 
            player.dx = cos(player.angle * DEG2RAD);
            player.dy = sin(player.angle * DEG2RAD);

            // check if angle is too high, then reset it back to 0
            if(player.angle > 360)
                player.angle -= 360;
        }

        if(IsKeyDown(KEY_A)){
            // player.x -= player.speed * GetFrameTime();
            // when you press, turning left, so increase the angle
            player.angle -= player.turnSpeed;
            // update the x and y coordinates 
            player.dx = cos(player.angle * DEG2RAD);
            player.dy = sin(player.angle * DEG2RAD);
           
            // checking if angle is too high, only going from 0 - 2pi
            if(player.angle < 0)
                player.angle += 360;

        }
        // ending the rendering phase
        EndDrawing();
    }
    // to prevent leak
    CloseWindow();
    return 0;
}



