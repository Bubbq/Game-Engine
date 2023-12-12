#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <raymath.h>
#define MAP_SIZE 64

struct Ray3D ray;
struct Player player;

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
};

// map is 8x8
int mapX = 8;
int mapY = 8;
// the bounded walls in the array, denoted with a 1 in the int array
Rectangle walls[MAP_SIZE];
// 1's represent walls and 0 open space
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

// finds the distance of the wall the ray collides into based off the players current position
float rayDistance(float px, float py, float rx, float ry){
   // use the distance formula
   return sqrt((rx - px) * (rx - px) + (ry - py) * (ry - py));
}

// to draw the ray from the player to the wall hes looking at
void drawRay(Vector2 ray1, Vector2 ray2){
   // the end coordinates of the ray (collision w wall)
   float rx, ry;
   // want the smallest distance from the nearest horizontal or vertical line
   if(Vector2Distance(ray1, (Vector2){player.x, player.y}) > Vector2Distance(ray2, (Vector2){player.x, player.y})){
       rx = ray2.x;
       ry = ray2.y;
   }

   else {
       rx = ray1.x;
       ry = ray1.y;
   }
    //  draw the ray from starting from the players center
    DrawLine(player.x, player.y, rx, ry, ORANGE);
}

// finding the coords of the nearest horizontal gridline line of the wal
Vector2 drawRayH(struct Player player, struct Ray3D ray){


   // first, the rays angle  must b =2 the players angle
   ray.angle = player.angle;
   // the x and y of the map array, and the index of those coords
   float mx, my, mp;
    // the farthest a player can see (will eventually be 8)
   int hdepth = 0;
   // need to find the inverse tangent to find the x compoennt of rx
   float aTan = -1/tan(ray.angle * DEG2RAD);
   // now, we need to constantly update rx and ry

   // for looking down
   if(ray.angle > 180){
       // the ry is the next number divisible by the mapsize 
       ray.y = (((int)player.y>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) - 0.0001;
       // the rx is the x comp, or the inverse tangent * the difference between the ry and py
       ray.x = (player.y - ray.y) * aTan + player.x;


       // then find the offsets, will increment rx and ry with these until we hit a wall
       // for yo, its just the next MAP_SIZEth pixel
       ray.yo = -MAP_SIZE;
       ray.xo = -ray.yo * aTan;
   }

   // now when looking up
   if(ray.angle < 180){
       ray.y = (((int)player.y>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) + MAP_SIZE;
       ray.x = (player.y - ray.y) * aTan + player.x;


       ray.yo = MAP_SIZE;
       ray.xo = -ray.yo * aTan;
   }

   // looking straight left or right will never hit a horizontal line make dist high so we dont use it
   if(ray.angle == 0 || ray.angle == 180){
       ray.x = player.x;
       ray.y = player.y;
       hdepth = 8;
       return (Vector2){10000000000, 10000000000};
   }

   // do this until we reach the end of the map or hit a wall
   while(hdepth < mapX || hdepth < mapY){
       //  to find the nearest rectangle of rx and ry, divide down by MAP_SIZE, and see if the map array @ that point == 1
       mx = (int)ray.x>>(int)log2(MAP_SIZE);
       my = (int)ray.y>>(int)log2(MAP_SIZE);
       // find the maps position in the array, using same formula as the creation of the map
       mp = my * mapX + mx;

       // if you hit a wall
       if(mp > 0 && mp < mapX * mapY && map[(int)mp] == 1){
           printf("Wall hit at %f, %f\n", ray.x, ray.y);
           hdepth = 8;
       }
       // if we dont hit a wall, check the next horz line using the offsets
       else{
           ray.x += ray.xo;
           ray.y += ray.yo;
           // increase the rays depth
           hdepth++;
       }
      
   }
      
   // return the collided position of the ray looking for the nearest horizontal
   return ((Vector2){ray.x, ray.y});
}

// finding the coords of the nearest vertical gridline thats a wall
Vector2 drawRayV(struct Player player, struct Ray3D ray){
       ray.angle = player.angle;
       float mx, my, mp;
       // since we need to calculate the O of the triangle, we only need tan
       // make negative bc unit circle of raylib is upside down
       float nTan = -tan(ray.angle * DEG2RAD);

       int vdepth = 0;

       // since were worried about vertical, there are 2 different states, looking left or right
       // looking left
       if(ray.angle < 270 && ray.angle > 90){
           // the rx is the nearest number divisible by the map size
           ray.x = (((int)player.x>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) - 0.0001;
           // solve for the O in the right triangle, the + player.y is so we start at the player's coord
           ray.y = (player.x - ray.x) * nTan + player.y;  
           // the x offset is just the next vert gridline
           ray.xo = -MAP_SIZE;
           ray.yo = -ray.xo * nTan;    
       }

       // looking right
       if(ray.angle < 90 || ray.angle > 270){
           ray.x = (((int)player.x>>(int)log2(MAP_SIZE)) << (int)log2(MAP_SIZE)) + MAP_SIZE;
           ray.y = (player.x - ray.x) * nTan + player.y;  

           ray.xo = MAP_SIZE;
           ray.yo = -ray.xo * nTan; 
       }

       // looking striaght up or down will never hit a vertical line
       if(ray.angle == 270 || ray.angle == 90){
            ray.x = player.x;
            ray.y = player.y;
            vdepth = 8;    
            return (Vector2){10000000000, 10000000000};
       }

       // casting the ray
       while(vdepth < mapX || vdepth < mapY){
            // find the mapX and Yrepresentation of this vertical gridline
           mx = (int)ray.x>>(int)log2(MAP_SIZE);
           my = (int)ray.y>>(int)log2(MAP_SIZE);
           // find the maps position in the array
           mp = my * mapX + mx;
           // if you hit a wall
           if(mp > 0 && mp < mapX * mapY && map[(int)mp] == 1)
               vdepth = 8;
                 
           // if we dont hit a wall, check the next vert line using the offsets
           else{
               ray.x += ray.xo;
               ray.y += ray.yo;
               // increase the rays depth
               vdepth++;
           }
       }
       // now return the collided position
       return (Vector2){ray.x, ray.y};
   }

// to render a player, will be based on the players x and y coordinate

void drawPlayer(struct Player player){
   // draw circle based on player coords
   DrawCircle(player.x, player.y, player.radius, BLUE);
}

int main() {
   // capping framerate to monitors referesh rate
   SetWindowState(FLAG_VSYNC_HINT);
   // creating window
   InitWindow(510, 512, "Raycast Engine");

   // player starts at the center
   player.x = GetScreenWidth() / 2.0;
   player.y = GetScreenHeight() / 2.0;
   player.radius = 15;
   player.angle = 80.00;
   player.turnSpeed = 4.00;
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
       drawRay(drawRayH(player, ray), drawRayV(player, ray));
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
