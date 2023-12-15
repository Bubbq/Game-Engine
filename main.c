#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <float.h>
#include <raymath.h>

#define BOX_SIZE 64

// map is 8x8
int mapX = 8;
int mapY = 8;
// the bounded walls in the array, denoted with a 1 in the int array
Rectangle walls[BOX_SIZE];
// 1's represent walls and 0 open space
int map[] =
{
1,1,1,1,1,1,1,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,1,0,0,0,0,0,1,
1,0,0,0,0,1,0,1,
1,0,0,1,0,0,0,1,
1,0,1,0,0,0,0,1,
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
          Color color = (map[i * mapX + j] == 1) ? WHITE : DARKGRAY;
          // draw the rectangle, draws from top left vertex
          // each point is BOX_SIZE (64px) away from each other
          DrawRectangle(j * BOX_SIZE - 1, i * BOX_SIZE - 1, BOX_SIZE - 1, BOX_SIZE - 1, color);
          // putting all the walls in the map into an array so we can iterate through the for collision checking
          if(map[i * mapX + j] == 1){
              walls[i * mapX + j].x = j * BOX_SIZE;
              walls[i * mapX + j].y = i * BOX_SIZE;
              walls[i * mapX + j].width = BOX_SIZE;
              walls[i * mapX + j].height = BOX_SIZE;
          }
      }
  }
    /*
        "i * mapX + j] == 1" gives the 1D map arr 2D properties, making it essentially look like:

        i j j j j j j j
        i j j j j j j j
        i j j j j j j j
        i j j j j j j j
        i j j j j j j j
        i j j j j j j j

     */
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
    float hdist, vdist = FLT_MAX;
    // index representation of map of wall user hits
    int mx, my, mp;
    // the final position and distance of the ray
    float fd; 

    // need rays angle wider than players angle to recreate the FOV of a person
    ra = pa - 30;

    // adjust angle from edge cases
    if(ra > 360) {ra -= 360;} if(ra < 0) {ra += 360;}

    // in order to draw the ray, we need to find the coord of the nearest horzontal and vertical gridline, then continue to offset until we find a wall
    for(int ray = 0; ray < 120; ray++){
        // first, find the hx and hy
        hdepth = 0;
        // hdist = FLT_MAX;
        // in this context, we have 0 and theta, but need A of the triangle, so use inverse tan
        float aTan = -1/tan(ra * DEG2RAD);

        // for looking up
        if(ra > 180){
            // the ry is the next number divisible by the mapsize 
            ry = (((int)py>>(int)log2(BOX_SIZE)) << (int)log2(BOX_SIZE)) - 0.0001;
            // the rx is the x comp, or the inverse tangent * the difference between the ry and py
            rx = (py - ry) * aTan + px;

            // then find the offsets, will increment rx and ry with these until we hit a wall
            // for yo, its just the next BOX_SIZEth pixel
            ryo = -BOX_SIZE;
            rxo = -ryo * aTan;
        }

        // now when looking down, same logic as looking up w few changes
        if(ra < 180){
            ry = (((int)py>>(int)log2(BOX_SIZE)) << (int)log2(BOX_SIZE)) + BOX_SIZE;
            rx = (py - ry) * aTan + px;

            ryo = BOX_SIZE;
            rxo = -ryo * aTan;
        }

        // looking straight left or right will never hit a horizontal line 
        if(ra == 0 || ra == 180){
            // make ray nonexistent by making its length the players coords
            rx = px;
            ry = py;
        }

        // now, keep moving to the next horizontal gridlines until we reach a wall
        while(hdepth < mapX || hdepth < mapY){
            //  to find the nearest rectangle of rx and ry, divide by BOX_SIZE (givng numbers 0-8), and see if the map array @ that point == 1
            mx = (int) rx >> (int)log2(BOX_SIZE);
            my = (int) ry >> (int)log2(BOX_SIZE);
            // find the maps position in the array, using same formula as the creation of the map
            mp = my * mapX + mx;

            // if you hit a wall
            if(mp > 0 && mp < mapX * mapY && map[(int)mp] == 1){
                hdepth = 8;
            }
            
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

        // now, we need to find the vertical distance
        vdepth = 0;
        
        // since we need to calculate the O of the triangle, we only need tan
        // make negative bc unit circle of raylib is upside down
        float nTan = -tan(ra * DEG2RAD);
        
        // looking left
        if(ra < 270 && ra > 90){
            // the rx is the nearest number divisible by the map size
            rx = (((int)px >> (int)log2(BOX_SIZE)) << (int)log2(BOX_SIZE)) - 0.0001;
            // solve for the O in the right triangle, the + py is so we start at the player's coord
            ry = (px  - rx) * nTan + py;  
            // the x offset is just the next vert gridline
            rxo = -BOX_SIZE;
            ryo = -rxo * nTan;    
        }

        // looking right
        if(ra < 90 || ra > 270){
            rx = (((int)px >> (int)log2(BOX_SIZE)) << (int)log2(BOX_SIZE)) + BOX_SIZE;
            ry = (px  - rx) * nTan + py;  

            rxo = BOX_SIZE;
            ryo = -rxo * nTan; 
        }

        // looking striaght up or down will never hit a vertical line
        if(ra == 270 || ra == 90){
            // make ray invisible
            rx = px ;
            ry = py;
        }

        // just like before, keep updating rx and y until we reack a vert gridline thats a wall
        while(vdepth < mapX || vdepth < mapY){
            // find the mapX and Yrepresentation of this vertical gridline
           mx = (int) rx >> (int)log2(BOX_SIZE);
           my = (int) ry >> (int)log2(BOX_SIZE);
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
        Color c;
        if(hdist > vdist){
            rx = vx; ry = vy; 
            fd = vdist;
            c = MAROON;
        }
        if(vdist > hdist){
            rx = hx; ry = hy;
            fd = hdist;
            c = RED;
        }

        // drawing the shortest path to a wall
        DrawLine(px, py, rx, ry, ORANGE);
        // drawing where the player is currently facing
        DrawLineEx((Vector2){px, py}, (Vector2){px + (pdx * 30), py + (pdy * 30)}, 3, GREEN);

        // the length of each line drawn will be the block area * screen height / distance
        float lineH = (BOX_SIZE * GetScreenHeight()) / fd; if(lineH > GetScreenHeight() * .75 || lineH == 0) {lineH = GetScreenHeight() * .75;} 
        // where every line should start from to prevent warping, grows upward by half the line hieght by subtracting as more down = increase in y
        float lineO = (GetScreenHeight() / 2.0) - (lineH / 2.0);
        // draw each line every 8px and 8px wide
        DrawLineEx((Vector2){ray * 8 + GetScreenWidth() / 1.95, lineO}, (Vector2){ray * 8 + GetScreenWidth() / 1.95, lineO + lineH}, 8, c);
        // doing this will make the walls larger the closer we are, and smaller the further we are

        // inc angle to make ray for every angle 30 deg to the right and left of the player
        ra += 0.5;
        // adj angle from edge cases
        if(ra > 360) {ra -= 360;} if(ra < 0) {ra += 360;}
    }
}

// to draw player based on updating position
void drawPlayer(){
   // draw circle based on player coords
   DrawCircle(px, py, pr, BLUE);
}

// to update player position and angle based on WASD keys
void move(){
    // W and S keys are for forward and backward movement
    if(IsKeyDown(KEY_W)){
        // first, update the positioning of x
        px += pdx * GetFrameTime() * ps;
        // if it collides move back by the x compent of the players direction
        for(int i = 0; i < BOX_SIZE; i++){
            if (CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i]))
                // keep the player at the bounds of the wall
                px -= pdx * GetFrameTime() * ps;
        }   

        // then, update y upon movement
        py += pdy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if (CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i]))
                py -= pdy * GetFrameTime() * ps;
        }        

        // doing this allows the components of movement to be independent of one another
        // so if you press W and D @ the same time, itll correct x, but the D will make the player "slide" along the top of the wall
    }
        
    if(IsKeyDown(KEY_S)){
        // first, update the positioning of x
        px -= pdx * GetFrameTime() * ps;
        // if it collides move back by the x compent of the players direction
        for(int i = 0; i < BOX_SIZE; i++){
            if (CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i]))
                // keep the player at the bounds of the wall
                px += pdx * GetFrameTime() * ps;
        }   

        // then, update y upon movement
        py -= pdy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if (CheckCollisionCircleRec((Vector2){px, py}, pr, walls[i]))
                py += pdy * GetFrameTime() * ps;
        }        

        // doing this allows the components of movement to be independent of one another
        // so if you press S and D @ the same time, itll correct y, but the D will make the player "slide" along the right of the wall
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

int main() {

    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);
    // creating window
    InitWindow(1020, 512, "Raycast Engine");

    // player starts at the center of left window
    px = GetScreenWidth() / 4.0;
    py = GetScreenHeight() / 4.0;

    // quantitive properties
    pr = 5;
    pa = 90.00;
    pt = 3.00;
    ps = 200.00;
    pdy = sin(pa * DEG2RAD);

    // the game loop, everything that runs is within this loop
    while(!WindowShouldClose()){
        // for player movement
        move();
        // start of the rendering phase
        BeginDrawing();
        // benchmark testing
        DrawFPS(GetScreenWidth() * .90, GetScreenHeight() * .05);
        // make background black
        ClearBackground(BLACK);
        // draw the map
        drawMap();
        // draw the player
        drawPlayer();
        // draw the ray
        drawRay();
        // ending the rendering phase
        EndDrawing();
    }

    // to prevent leak
    CloseWindow();
    return 0;
}