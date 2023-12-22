#include <stdio.h>
#include <raylib.h>
#include <float.h>
#include <raymath.h>
#include <stdlib.h>

// size of each rectangle in the map
#define BOX_SIZE 64

// map is 8x8
int mapX = 8;
int mapY = 8;
// the list of every rectangle in the map
Rectangle boxes[BOX_SIZE];
// 1's represent walls and 0 open boxes (-1 walls cannot be altered)
int map[] =
{
-1,-1,-1,-1,-1,-1,-1,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1, 0, 0, 0, 0, 0, 0,-1,
-1,-1,-1,-1,-1,-1,-1,-1,
};

// the players current position on the screen
float px, py;
// the radius and angle of the player
float pr, pa;
// the turn and movement speed of the player
float pt, ps;
// the x and y comp of the angle the player is facing
float pdx, pdy;
// the vertial offset that the player is looking at the wall with and the speed of this movement
float pz, pzs;

// to load map from save file if user has one
void loadMap(){
   FILE *filePointer = fopen("map_save.txt", "r");
  
   // if theres no file saved, used default
   if (filePointer == NULL)
       return;   
   else
       // Read map data from the file
       for (int i = 0; i < BOX_SIZE; ++i)
           fscanf(filePointer, "%d", &map[i]);
  
   fclose(filePointer);
}

void saveMap() {
   // remove("map_save.txt");
   FILE *file = fopen("map_save.txt", "w");

   if (file == NULL) {
       // Error opening the file
       printf("Error opening file!\n");
       return;
   }

   // Write current map content
   for (int i = 0; i < BOX_SIZE; i++)
       fprintf(file, "%d ", map[i]);

   // Close the file
   fclose(file);
}

// to render the map
void drawMap(){

   // need to iterate through row by row, treating it like a 2D array
   for(int i = 0; i < mapY; i++){
       for(int j = 0; j < mapX; j++){
           // the i is the first element in every row of this array, and the j is each element in said row
           Color color = (map[i * mapX + j] == 1) || (map[i * mapX + j] == -1) ? WHITE : DARKGRAY;
           // draw the rectangle, draws from top left vertex
           // each point is BOX_SIZE (64px) away from each other
           DrawRectangle(j * BOX_SIZE - 1, i * BOX_SIZE - 1, BOX_SIZE - 1, BOX_SIZE - 1, color);
           // putting all the walls in the map into an array so we can iterate through the for collision checking
           boxes[i * mapX + j].x = j * BOX_SIZE;
           boxes[i * mapX + j].y = i * BOX_SIZE;
           boxes[i * mapX + j].width = BOX_SIZE;
           boxes[i * mapX + j].height = BOX_SIZE;
       }
 }
   /*
       "i * mapX + j " gives the 1D map arr 2D properties, making it essentially look like:

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
    // the final distance of the ray
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
            ry = (((int)py >> (int)log2(BOX_SIZE)) << (int)log2(BOX_SIZE)) + BOX_SIZE;
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
            if(mp > 0 && mp < mapX * mapY && (map[(int)mp] == 1 || map[(int)mp] == -1)){
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
            ry = (px - rx) * nTan + py; 
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
        while(vdepth < 8){
            // find the mapX and Yrepresentation of this vertical gridline
            mx = (int) rx >> (int)log2(BOX_SIZE);
            my = (int) ry >> (int)log2(BOX_SIZE);
            // find the maps position in the array
            mp = my * mapX + mx;
            // if you hit a wall
            if(mp > 0 && mp < mapX * mapY && (map[(int)mp] == 1 || map[(int)mp] == -1))
                vdepth = 8;
                
            // if we dont hit a wall, check the next vert line using the offsets
            else{
                rx += rxo;
                ry += ryo;
                    
                // dont want rays to show in the window in the abscence of walls
                if(rx >= GetScreenWidth() / 2.0)
                    vdepth = 8;
                    
                    
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

        // draws the mutiple rays
        DrawLine(px, py, rx, ry, ORANGE);
        // drawing where the player is currently facing
        DrawLineEx((Vector2){px, py}, (Vector2){px + (pdx * 30), py + (pdy * 30)}, 3, GREEN);

        // fixing fisheye effect on walls
        float angleDiff = pa - ra; if(angleDiff > 360){angleDiff -= 360;} if(angleDiff < 0) {angleDiff += 360;}
        fd *= cos(angleDiff * DEG2RAD);

        // the length of each line drawn will be the block area * screen height / distance
        float lineH = (BOX_SIZE * GetScreenHeight() * .6) / fd; 
        // where every line should start from to prevent warping, grows upward by half the line hieght by subtracting as more down = increase in y
        float lineO = (GetScreenHeight() / 2.0) - (lineH / 2.0) + pz * 10;
       
        // adjust the bottom of the line to the vertical offset of the player, giving the illusion of looking up or down
        lineH += pz / 10.0;
            
        // only draw wall if player is not looking into void
        if((rx <= GetScreenWidth() / 2.0 && rx >= 0) && (ry <= GetScreenHeight() && ry >= 0))
            DrawLineEx((Vector2){ray * 8 + GetScreenWidth() / 1.95, lineO}, (Vector2){ray * 8 + GetScreenWidth() / 1.95,  lineO + lineH}, 8, c);        
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
    // crosshair of player in 3D view
    DrawCircle(GetScreenWidth() * .75, GetScreenHeight() * .5, 3, GREEN);  
}

// to update player position and angle based on WASD keys
void move(float mdx){
    // Check if the mouse moved left or right based on the difference in position passed
    if(abs((int)mdx) > 7){
        // moving right
        if (mdx < 0) {
            pa += pt;
            pdx = cos(pa * DEG2RAD);
            pdy = sin(pa * DEG2RAD);
            // angle correction
            if(pa > 360) {pa -= 360;} if(pa < 0) {pa += 360;}
        }
            
        // moving left
        if (mdx > 0) {
            pa -= pt;
            pdx = cos(pa * DEG2RAD);
            pdy = sin(pa * DEG2RAD);
            if(pa > 360) {pa -= 360;} if(pa < 0) {pa += 360;}
        }
    }
        
    // W and S keys are for forward and backward movement
    if(IsKeyDown(KEY_W)){
        // first, update the positioning of x
        px += pdx * GetFrameTime() * ps;
        // if it collides move back by the x compent of the players direction
        for(int i = 0; i < BOX_SIZE; i++){
            // check every rectangle, only "collide" when rectangle is a wall (map[i] ==1) or if your trying to leave the void
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                px -= pdx * GetFrameTime() * ps;
        }  

        // then, update y upon movement
        py += pdy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
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
            // check every rectangle, only "collide" when rectangle is a wall (map[i] ==1)
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                px += pdx * GetFrameTime() * ps;
        }  

        // then, update y upon movement
        py -= pdy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                py += pdy * GetFrameTime() * ps;
        }       

        // doing this allows the components of movement to be independent of one another
        // so if you press S and D @ the same time, itll correct y, but the D will make the player "slide" along the right of the wall
    }

    // A and D are the strafing keys, need to make 2 lines perpendicular where the player is currently facing
    if(IsKeyDown(KEY_D)){
        // whatever the players angle is, you need 2 add 90 to get a line purpendicular to wherever the player is facing, strafing right
        float srA = pa + 90; if(srA < 0) {srA += 360;} if(srA > 360) {srA -= 360;}
        float srdx = cos(srA * DEG2RAD);
        float srdy = sin(srA * DEG2RAD);

        px += srdx * GetFrameTime() * ps;
        // if it collides move back by the x compent of the players direction
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                // keep the player at the bounds of the wall
                px -= srdx * GetFrameTime() * ps;
        }  

        // then, update y upon movement
        py += srdy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                py -= srdy * GetFrameTime() * ps;
        }       
    }

    if(IsKeyDown(KEY_A)){
        // whatever the players angle is, you need 2 subract 90 to get a line purpendicular to wherever the player is facing, strafing left
        float slA = pa - 90; if(slA < 0) {slA += 360;} if(slA > 360) {slA -= 360;}
        float sldx = cos(slA * DEG2RAD);
        float sldy = sin(slA * DEG2RAD);

        px += sldx * GetFrameTime() * ps;
        // if it collides move back by the x compent of the players direction
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                // keep the player at the bounds of the wall
                px -= sldx * GetFrameTime() * ps;
        }  

        // then, update y upon movement
        py += sldy * GetFrameTime() * ps;
        // if collision, keep y at the same position
        for(int i = 0; i < BOX_SIZE; i++){
            if ((CheckCollisionCircleRec((Vector2){px, py}, pr, boxes[i]) && (map[i] == 1 || map[i] == -1)))
                py -= sldy * GetFrameTime() * ps;
        }       
    }
}

int main() {
    // capping framerate to monitors referesh rate
    SetWindowState(FLAG_VSYNC_HINT);
    // creating window
    InitWindow(1020, 512, "Raycast Engine");

    // load previous map, if applicable
    loadMap();

    // starting in playmode, where you can move the player through the map
    bool edit = false;
    DisableCursor();
    Vector2 prevMousePosition = GetMousePosition();

    // quantitive properties 
    px = GetScreenWidth() / 4.0;
    py = GetScreenHeight() / 2.0;
    pr = 5;
    pa = 90.00;
    pt = 1.50;
    ps = 100.00;
    pdx = 0.00;
    pdy = sin(pa * DEG2RAD);
    pz = 0.00;
    pzs = 1.25;

    // while the game is running, preform these operations
    while(!WindowShouldClose()){
        // start of the rendering phase
        BeginDrawing();

        // toggle edit mode
        if(IsKeyPressed(KEY_E)){
            edit = true;
            // user can see their cursor
            EnableCursor();
        }
        
        // toggle play mode
        if(IsKeyPressed(KEY_Q)){
            edit = false;
            // cannot see cursor
            DisableCursor();
            // save map after leaving edit mode
            saveMap();
            // load new changes
            loadMap();
        }

        // in edit mode, when user presses a box, if its a wall, its now empty boxes, and vice versa
        if(edit){
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                // get position of click
                Vector2 mc = GetMousePosition();
                for(int i = 0; i < BOX_SIZE; i++){
                    // Check collision against the rectangles representing the map
                    if(CheckCollisionPointRec(mc, boxes[i]) && map[i] != -1){
                        // toggle between walls and empty boxes
                        map[i] = (map[i] == 1) ? 0 : 1;
                    }
                }
            } 
        }

        // in play mode, user can move the player around the map
        else{ 
            // using mouse to see the map
            Vector2 currentMousePosition = GetMousePosition();
            // difference between the prev and current position's x/y componenets, for turing player
            float mdx = prevMousePosition.x - currentMousePosition.x;
            float mdy = prevMousePosition.y - currentMousePosition.y;

            // adjust the offset for looking both down and up

            if(abs((int)mdy) > 7){
                if(mdy < 0)
                    pz -= pzs;
                else if(mdy > 0)
                    pz += pzs;
            }

            // handling player movement
            move(mdx);
            // Update previous mouse position to continue checking the difference while game is running
            prevMousePosition = currentMousePosition;
        }
        
        // make background black
        ClearBackground(BLACK);
        // drawing the map
        drawMap();
        // draw the rays
        drawRay();
        // rendering the player
        drawPlayer();
        // screen headers
        DrawText("2D VIEW", GetScreenWidth() * .167, GetScreenHeight() * .02, 50, BLUE);
        DrawText("3D VIEW", GetScreenWidth() * .667, GetScreenHeight() * .02, 50, BLUE);
        // benchmark testing
        DrawFPS(GetScreenWidth() * .90, GetScreenHeight() * .9);
        // ending the rendering phase
        EndDrawing();
    }

    // to prevent leak
    CloseWindow();
    return 0;
}
