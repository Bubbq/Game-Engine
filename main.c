#include <stdio.h>
#include <raylib.h>
#include <math.h>
#define MAP_SIZE 64

// the map of the entire game
// 1's represent walls and 0 open space
int mapX = 8;
int mapY = 8; // map is 8x8
// the bounded walls in the array, denoted with a 1 in the int array
Rectangle walls[MAP_SIZE];
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
           // each point is MAP_SIZE (64px) away from each other
           // subtract one so you can see the individual lines
           DrawRectangle(j * MAP_SIZE - 1, i * MAP_SIZE - 1, MAP_SIZE - 1, MAP_SIZE - 1, color);
           // initalizing the reactangles of the map for collision checking
           if(map[i * mapX + j] == 1){
               walls[i * mapX + j].x = j * MAP_SIZE;
               walls[i * mapX + j].y = i * MAP_SIZE;
               walls[i * mapX + j].width = MAP_SIZE;
               walls[i * mapX + j].height = MAP_SIZE;
           }
       }
   }
}

// object representing a single player
struct Player{
    // the positioning of the player on the screen 
    float x,y;
    float speed;
    // the angle, x, and y components of the players movement
    float angle, turnSpeed, dx, dy;
    int radius;
};

// representing a single ray in the engine
struct Ray3D{
    // same as player angle
    float angle;
    // the cartesian coordinates for the nearest horizontal gridline you will hit with the ray
    float x, y;
    // the offset x & y coords, the next horizontal line, denoted as (rx + ro, ry + yo)
    float xo, yo;
    // how far the player will see
    float depth;
};

// // drawing the rays in the engine
void drawRays3D(struct Player player, struct Ray3D ray){
    // first, the rays angle  must b =2 the players angle
    ray.angle = player.angle;
    float mx, my, mp;
    // next, we need to find the x and y of the closest horizontal gridline the ray will hit

    // making one line for now
    for(int i = 0; i < 1; i++){
        // setting depth 2 0
        ray.depth = 0;
        // need to find the inverse tangent to find the x compoennt of rx
        float aTan = -1/tan(ray.angle * DEG2RAD);
        // now, we need to constantly update rx and ry

        // for looking down, the unit circle in raylib is upside down
        if(ray.angle > 180){
            // the ry is    
            ray.y = (((int)player.y>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) - 0.0001;
            // the rx is the x comp, or the inverse tangent * the difference between the ry and py
            ray.x = (player.y - ray.y) * aTan + player.x;

            // find the offsets, will increment rx and ry with these until we hit a wall
            // for yo, its just the next area of each cube in the map
            ray.yo = -MAP_SIZE;
            ray.xo = -ray.yo * aTan;
        }

        // now when looking up
        if(ray.angle < 180){
            // the ry is the nearest number divisible by 64, or the rectangle area, to make modular, just mapX squared
            ray.y = (((int)player.y>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) + MAP_SIZE;
            // the rx is the x comp, or the inverse tangent * the difference between the ry and py
            ray.x = (player.y - ray.y) * aTan + player.x;

            // find the offsets, will increment rx and ry with these until we hit a wall
            // for yo, its just the next area of each cube in the map
            ray.yo = MAP_SIZE;
            ray.xo = -ray.yo * aTan;
        }

        // looking striaght left or right well never hit a horizontal line
        if(ray.angle == 0 || ray.angle == 180){
            ray.x = player.x;
            ray.y = player.y;
            ray.depth = 8;
        }

        // do this until we reach the end of the map or hit a wall
        while(ray.depth < mapX || ray.depth < mapY){
            //  to find the nearest map, divide down by 64 to get 8 * 8, and see if that array == 1
            mx = (int)ray.x>>(int)log2(MAP_SIZE);
            my = (int)ray.y>>(int)log2(MAP_SIZE);
            // find the maps position in the array
            mp = my * mapX + mx;
            // if you hit a wall
            if(mp < mapX * mapY && map[(int)mp] == 1){
                ray.depth = 8;
            }
            // if we dont hit a wall, check the next horz line using the offsets
            else{
                ray.x += ray.xo;
                ray.y += ray.yo;
                // increase the rays depth
                ray.depth++;
            }
        }

        // draw the lines
        DrawLine(player.x, player.y, ray.x, ray.y, GREEN);
    }


}

// to render a player, will be based on the players x and y coordinate
void drawPlayer(struct Player player){
    // draw circle based on player coords
    DrawCircle(player.x, player.y, player.radius, BLUE);
    // draw line so we know where we are turning, have to add player.x/y to endpoint bc we want to end from there and add by the components
    DrawLine(player.x, player.y, player.x + player.dx * 30, player.y + player.dy * 30, BLUE);
}

struct Ray3D ray;
struct Player player;

int main() {
    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);
    // creating window 
    InitWindow(510, 512, "Raycast Engine");

    // player starts at the center
    player.x = GetScreenWidth() / 2.0;
    player.y = GetScreenHeight() / 2.0;
    player.radius = 15;
    player.angle = 90.00;
    player.turnSpeed = 5.00;
    player.speed = 200.00;

    // the game loop, everything that runs is within this loop
    while(!WindowShouldClose()){
        // start of the rendering phase
        BeginDrawing();
        // show fps for benchmark testing
        DrawFPS(100, 100);
        // make background black
        ClearBackground(GRAY);
        // draw the map
        drawMap();
        // draw the player
        drawPlayer(player);
        // draw the ray
        drawRays3D(player, ray);

        // handling player movement, moving its pixel position based off of the speed
        if(IsKeyDown(KEY_W)){
            // we have to move by the componets of x and y
            player.x += player.dx * GetFrameTime() * player.speed;
            player.y += player.dy * GetFrameTime() * player.speed;

            // checking the collision for every wall
            for(int i = 0; i < MAP_SIZE; i++){
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
            for(int i = 0; i < MAP_SIZE; i++){
                if(CheckCollisionCircleRec((Vector2){player.x, player.y}, player.radius, walls[i])){
                    // move player
                    player.x += player.dx * GetFrameTime() * player.speed;
                    player.y += player.dy * GetFrameTime() * player.speed;
                }
            }
        }

        // turning keys, update the x and y component as the player is turning (x is cos and y is sin)
        if(IsKeyDown(KEY_D)){
            // as you press, turn right, so decrese the angle
            player.angle += player.turnSpeed;
            // update the x and y coordinates 
            player.dx = cos(player.angle * DEG2RAD);
            player.dy = sin(player.angle * DEG2RAD);

            // check if angle is too high, then reset it back to 0
            if(player.angle == 360)
                player.angle -= 360;
        }

        if(IsKeyDown(KEY_A)){
            // player.x -= player.speed * GetFrameTime();
            // when you press, turning left, so increase the angle (bc raylib unit circle is upside down)
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