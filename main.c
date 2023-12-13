#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <float.h>
#include <raymath.h>

#define MAP_SIZE 64

// map is 8x8
int mapX = 8;
int mapY = 8;
// the bounded walls in the array, denoted with a 1 in the int array
Rectangle walls[MAP_SIZE];
// 1's represent walls and 0 open space
int map[] =
{
1,1,1,1,1,1,1,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,1,1,1,1,1,1,1,
};

// the players current position on the screen
float px, py;
// the radius and angle of the player
float pr, pa;
// the turn and movement speed of the player
float pt, ps;
// the x and y comp of the angle the player is facing
float pdx, pdy;

// to render the map
void drawMap(){
  // need to iterate through row by row, treating it like a 2D array
  for(int i = 0; i < mapY; i++){
      for(int j = 0; j < mapX; j++){
          // the i is the first element in every row of this array, and the j is each element in said row
          Color color = (map[i * mapX + j] == 1) ? WHITE : BLACK;
          // draw the rectangle, draws from top left vertex
          // each point is MAP_SIZE (64px) away from each other
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

// drawing the rays stemming from the player, stopping at the nearest wall coord of the map
void drawRay(){
    // x/y coord of ray pos and angle
    float rx, ry, ra;
    // offset, or differnece between next horz/vert gridline
    float rxo, ryo;
    // coord of nearest vertical/horizontal gridline wall 
    float vx, vy, hx, hy;
    // depth relative to players position on the map
    int hdepth, vdepth;
    // h/v dist of h/v coords relative to players position
    float hdist, vdist;
    // index representation of map of wall user hits
    int mx, my, mp; 

    // need rays angle wider than players angle to recreate the FOV of a player
    ra = pa - 30;

    // adj angle from edge cases
    if(ra > 360)
        ra -= 360;
    if(ra < 0)
        ra += 360;

    // in order to draw the ray, we need to find the coord of the nearest horzontal and vertical gridline, then continue to offset until we find a wall
    for(int r = 0; r < 61; r++){
        // first, find the hx and hy
        hdepth = 0;
        // in this context, we have 0 and theta, but need A of the triangle, so use inverse tan
        float aTan = -1/tan(ra * DEG2RAD);

        // for looking up
        if(ra > 180){
            // the ry is the next number divisible by the mapsize 
            ry = (((int)py>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) - 0.0001;
            // the rx is the x comp, or the inverse tangent * the difference between the ry and py
            rx = (py - ry) * aTan + px;

            // then find the offsets, will increment rx and ry with these until we hit a wall
            // for yo, its just the next MAP_SIZEth pixel
            ryo = -MAP_SIZE;
            rxo = -ryo * aTan;
        }

        // now when looking down, same logic as looking up w few changes
        if(ra < 180){
            ry = (((int)py>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) + MAP_SIZE;
            rx = (py - ry) * aTan + px;

            ryo = MAP_SIZE;
            rxo = -ryo * aTan;
        }

        // looking straight left or right will never hit a horizontal line 
        if(ra == 0 || ra == 180 || ra == 360){
            // make ray nonexistent
            rx = px;
            ry = py;
            // make depth end
            hdepth = 8;
            // make hdist large as we're drawing the smallest ray and dont want this to be shown
            hdist = FLT_MAX;
        }

        // now, keep moving to the next horizontal gridlines until we reach a wall
        while(hdepth < mapX || hdepth < mapY){
            //  to find the nearest rectangle of rx and ry, divide down by MAP_SIZE, and see if the map array @ that point == 1
            mx = (int) rx >> (int)log2(MAP_SIZE);
            my = (int) ry >> (int)log2(MAP_SIZE);
            // find the maps position in the array, using same formula as the creation of the map
            mp = my * mapX + mx;

            // if you hit a wall
            if(mp > 0 && mp < mapX * mapY && map[(int)mp] == 1)
                hdepth = 8;
            
            // if we dont hit a wall, check the next horz line using the offsets
            else{
                // move to the next horz gridline
                rx += rxo;
                ry += ryo;
                // increase the rays depth as we are seeing further
                hdepth++;
            }
        }  
        
        // // set the hx and hy to that walline and calculate that distance relative to the player
        hx = rx; hy = ry; hdist = Vector2Distance((Vector2){px, py}, (Vector2){hx, hy});
        // DrawLineEx((Vector2){px, py}, (Vector2){hx, hy}, 3, PURPLE);

        // now, we need to find the vertical distance
        vdepth = 0;
        // since we need to calculate the O of the triangle, we only need tan
        // make negative bc unit circle of raylib is upside down
        float nTan = -tan(ra * DEG2RAD);
        
        // looking left
        if(ra < 270 && ra > 90){
            // the rx is the nearest number divisible by the map size
            rx = (((int)px >> (int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) - 0.0001;
            // solve for the O in the right triangle, the + py is so we start at the player's coord
            ry = (px  - rx) * nTan + py;  
            // the x offset is just the next vert gridline
            rxo = -MAP_SIZE;
            ryo = -rxo * nTan;    
        }

        // looking right
        if(ra < 90 || ra > 270){
            rx = (((int)px >> (int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) + MAP_SIZE;
            ry = (px  - rx) * nTan + py;  

            rxo = MAP_SIZE;
            ryo = -rxo * nTan; 
        }

        // looking striaght up or down will never hit a vertical line
        if(ra == 270 || ra == 90){
            // make ray invisible
            rx = px ;
            ry = py;
            // show maxdepth
            vdepth = 8;    
            // make vdist large so we dont use it
            vdist = FLT_MAX;
        }

        // just like before, keep updating rx and y until we reack a vert gridline thats a wall
        while(vdepth < mapX || vdepth < mapY){
            // find the mapX and Yrepresentation of this vertical gridline
           mx = (int) rx >> (int)log2(MAP_SIZE);
           my = (int) ry >> (int)log2(MAP_SIZE);
           // find the maps position in the array
           mp = my * mapX + mx;
           // if you hit a wall
           if(mp > 0 && mp < mapX * mapY && map[(int)mp] == 1)
               vdepth = 8;
                 
           // if we dont hit a wall, check the next vert line using the offsets
           else{
               rx += rxo;
               ry += ryo;
               // increase the rays depth
               vdepth++;
           }
       }
        // update the coords of the vertgridline and find distance
        vx = rx; vy = ry; vdist = Vector2Distance((Vector2){px, py}, (Vector2){vx, vy});

        // finally, find the smaller/closer ray and draw it
        if(hdist > vdist){
            rx = vx; ry = vy;
        }
        if(vdist > hdist){
            rx = hx; ry = hy;
        }

        // drawing the shortest path to a wall
        DrawLine(px, py, rx, ry, ORANGE);

        printf("angle: %f\n", ra);
        // inc angle to make ray for every angle 30 deg to the right and left of the player
        ra++;
        // adj angle from edge cases
        if(ra > 360)
            ra -= 360;
        if(ra < 0)
            ra += 360;
    }
}

// to update player position and angle based on WASD keys
void move(){
    // handling player movement, moving its pixel position based off of the speed

    // W and S keys are for forward and backward movement
    if(IsKeyDown(KEY_W)){
        // we have to move by the componets of x and y, GetFrameTime makes all screens show the same speed, regadless of refresh rate
        px += pdx * GetFrameTime() * ps;
        py += pdy * GetFrameTime() * ps;

        // checking the collision for every possible wall
        for(int i = 0; i < MAP_SIZE; i++){
            if (CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i])){
                // moves the player down in opposite directions as were running into the wall
                px -= pdx * GetFrameTime() * ps;
                py -= pdy * GetFrameTime() * ps;
            }
        }        
    }
        
    if(IsKeyDown(KEY_S)){
        // move backwards based on the x and y components of the players angle
        py -= pdy * ps * GetFrameTime();
        px -= pdx * ps * GetFrameTime(); 

        // checking the collisions of the walls
        for(int i = 0; i < MAP_SIZE; i++){
            if(CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i])){
                // move player up as were backing into the wall
                px += pdx * GetFrameTime() * ps;
                py += pdy * GetFrameTime() * ps;
            }
        }
    }

    // A and D are the turning keys, update the x and y component (x is cos and y is sin)
    if(IsKeyDown(KEY_D)){
        // as you press, turn right, so inc the angle as unit circle in raylib is upside down
        pa += pt;
        // update the x and y coordinates
        pdx = cos(pa * DEG2RAD);
        pdy = sin(pa * DEG2RAD);

        // check if angle is too high
        if(pa > 360)
            pa -= 360;
    }

    if(IsKeyDown(KEY_A)){
        // when you press, turning left, so dec
        pa -= pt;
        // update the x and y coordinates
        pdx = cos(pa * DEG2RAD);
        pdy = sin(pa * DEG2RAD);
                
        // checking if angle is too low
        if(pa < 0)
            pa += 360;
    }
}

void drawPlayer(){
   // draw circle based on player coords
   DrawCircle(px, py, pr, BLUE);
   // draw line to represent where the player is facing
   DrawLineEx((Vector2){px, py}, (Vector2){px + (pdx * 50), py + (pdy * 50)}, 3, GREEN);
}

int main() {

    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);
    // creating window
    InitWindow(510, 512, "Raycast Engine");

    // player starts at the center
    px = GetScreenWidth() / 2.0;
    py = GetScreenHeight() / 2.0;

    // quantitive properties
    pr = 15;
    pa = 90.00;
    pt = 3.00;
    ps = 200.00;

    // the game loop, everything that runs is within this loop
    while(!WindowShouldClose()){
        // start of the rendering phase
        BeginDrawing();
        // make background black
        ClearBackground(GRAY);
        // draw the map
        drawMap();
        // draw the player
        drawPlayer();
        // for player movement
        move();
        // draw the ray
        drawRay();
        // ending the rendering phase
        EndDrawing();
    }

    // to prevent leak
    CloseWindow();
    return 0;

}