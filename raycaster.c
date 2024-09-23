#include <stdio.h>
#include <stdlib.h>

#include "headers/raylib.h"
#include "headers/raymath.h"

#define CELL_SIZE 64
#define NRAYS 120
#define ROW 16
#define COL 16
#define MAP_SIZE (ROW * COL)

const int FPS = 60;
const float SCR = CELL_SIZE * ROW;
const float LINE_THICKNESS = SCR / NRAYS;
const Vector2 CENTER = {(SCR / 2.0), (SCR / 2.0)};

float pz, pzs;

float angle_correction(float angle)
{
    if(angle > 360)
        angle -= 360;

    else if(angle < 0)
        angle += 360;

    // BAD SOLUTION LMFAO
    if(angle == 0.00)
        angle += 0.01;

    return angle;
}

float get_angle(Vector2 v1, Vector2 v2)
{
    Vector2 diff = Vector2Subtract(v1, v2);

    return angle_correction((atan2f(diff.y * DEG2RAD, diff.x * DEG2RAD)) * RAD2DEG);
}

typedef enum
{
    WALL = 0,
    FLOOR = 1,
} Type;

typedef struct
{
    Rectangle bounds;
    Type type;
} Grid;

void init_map(Grid map[MAP_SIZE])
{
    Vector2 current_position = (Vector2){0,0};

    for(int i = 0; i < ROW; i++, current_position.y += CELL_SIZE)
    {
        for(int j = 0; j < COL; j++)
        {
            map[(i * ROW) + j].bounds = (Rectangle){current_position.x, current_position.y, CELL_SIZE, CELL_SIZE};
            map[(i * ROW) + j].type = (i == 0 || i == (ROW - 1) || j == 0 || j == (COL - 1)) ? WALL : FLOOR;
            
            current_position.x += CELL_SIZE;
        }

        current_position.x = 0;
    }
}

void edit_grid(Grid map[MAP_SIZE], Vector2 player_position, float radius)
{
    for(int i = 0; i < MAP_SIZE; i++)
    {
        // add walls
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if(CheckCollisionPointRec(GetMousePosition(), map[i].bounds) && !CheckCollisionCircleRec(player_position, radius, map[i].bounds))
            {
                map[i].type = WALL;
            }
        }

        // remove walls
        else if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            if(CheckCollisionPointRec(GetMousePosition(), map[i].bounds))
            {
                map[i].type = FLOOR;
            }
        }
    }
}

void draw_grid(Grid map[MAP_SIZE])
{
    for(int i = 0; i < MAP_SIZE; i++)
    {
        DrawRectangleRec(map[i].bounds, map[i].type == WALL ? RAYWHITE : GRAY);
    }
}

void draw_world(float distances[NRAYS])
{
    for(int i = 0; i < NRAYS; i++)
    {
        // the length of each line drawn will be the height of the block proportional to the screens height divided by how far the player is from the wall
        float line_height = (CELL_SIZE * GetScreenHeight() * .6) / distances[i]; 
        // every line starts from half the height of the screen and moved upwards by half the increase in height
        float line_start = (GetScreenHeight() / 2.00) - (line_height / 2.0) + (pz * 20);
       
        // adjust the bottom of the line by the vertical offset of the player, giving the illusion of looking up or down
        line_height += pz / 10.0;
            
        // 3D render
        DrawLineEx((Vector2){(i * LINE_THICKNESS), line_start}, (Vector2){(i * LINE_THICKNESS),  (line_start + line_height)}, LINE_THICKNESS, BLUE);   
    }
}

typedef struct
{
    Vector2 position;
    Vector2 previous_position;
    float angle;
    float radius;
    float speed;
    float turn_speed;
} Player;

Player init_player()
{
    Player player;

    player.position = CENTER;
    player.angle = 0.00;
    player.radius = 10.00;
    player.speed = 250.00;
    player.turn_speed = 1.50;

    return player;
}

void show_player_direction(Vector2 position, float angle)
{
    Vector2 final_position = Vector2Scale((Vector2){cosf(angle * DEG2RAD), sinf(angle * DEG2RAD)}, 50);

    DrawLineEx(position, Vector2Add(position, final_position), 1, GREEN);
}

void draw_player(Player *player)
{
    DrawCircleSector(player->position, player->radius, 0, 360, 1, BLUE);
    show_player_direction(player->position, player->angle);
}

bool player_wall_collision(Grid map[MAP_SIZE], Vector2 position, float radius)
{
    for(int i = 0; i < MAP_SIZE; i++)
    {
        if((CheckCollisionCircleRec(position, radius, map[i].bounds)) && (map[i].type == WALL))
        {
            return true;
        }
    }

    return false;
}

bool is_movement_diagonal()
{
    if( (IsKeyDown(KEY_W) && IsKeyDown(KEY_D)) || 
        (IsKeyDown(KEY_W) && IsKeyDown(KEY_A)) || 
        (IsKeyDown(KEY_S) && IsKeyDown(KEY_A)) || 
        (IsKeyDown(KEY_S) && IsKeyDown(KEY_D)))
        {
            return true;
        }

    return false;
}

void move_player(Grid map[MAP_SIZE], Player *player, float angle)
{
    player->previous_position = player->position;

    // movement direction
    float dx = cosf(angle * DEG2RAD);
    float dy = sinf(angle * DEG2RAD);

    // prevents speedup during diagonal movement
    float movement_speed = is_movement_diagonal() ? (player->speed / sqrtf(2.0)) : player->speed;

    player->position.x += (dx * movement_speed * GetFrameTime());
    
    if(player_wall_collision(map, player->position, player->radius))
    {
        player->position.x -= (dx * movement_speed * GetFrameTime());
    }
    
    player->position.y += (dy * movement_speed * GetFrameTime());
    
    if(player_wall_collision(map, player->position, player->radius))
    {
        player->position.y -= (dy * movement_speed * GetFrameTime());
    }
}

void movement_controls(Grid map[MAP_SIZE], Player *player)
{
    // direction
    if(abs((int)GetMouseDelta().x) > 0)
    {
        // player->angle = get_angle(GetMousePosition(), player->position);
        if(GetMouseDelta().x < 0)
        {
            player->angle -= player->turn_speed;
        }

        else if(GetMouseDelta().x > 0)
        {
            player->angle += player->turn_speed;
        }

        player->angle = angle_correction(player->angle);
    }

    // movement
    if(IsKeyDown(KEY_W))
    {
        move_player(map, player, player->angle);
    }

    if(IsKeyDown(KEY_A))
    {
       move_player(map, player, (player->angle - 90));
    }

    if(IsKeyDown(KEY_S))
    {
        move_player(map, player, (player->angle - 180));
    }
    
    if(IsKeyDown(KEY_D))
    {
        move_player(map, player, (player->angle + 90));
    }
}

typedef struct
{
    Vector2 position;
    Vector2 offset;
} RAY;

Vector2 find_collision_point(Grid map[MAP_SIZE], RAY ray)
{
    int line_depth = 0;

    // used to find cells
    int mx, my, mp;

    while(line_depth < ROW)
    {
        // grid representation of potential wall
        mx = (int) ray.position.x >> (int)log2(CELL_SIZE);
        my = (int) ray.position.y >> (int)log2(CELL_SIZE);
        mp = (my * ROW) + mx;

        // if you hit a wall
        if((mp > 0) && (mp < MAP_SIZE) && (map[(int)mp].type == WALL))
        {
            line_depth = ROW;
        }
        
        // move to the next horz gridline
        else
        {
            ray.position = Vector2Add(ray.position, ray.offset);
            line_depth++;
        }
    } 

    return ray.position;
}

RAY get_horizontal_ray(Vector2 player_position, float ray_angle)
{
    RAY horizontal_projection;

    // looking for the A in triangle created from first horizontal line collision
    float aTan = -1 / tan(ray_angle * DEG2RAD);

    // for looking up
    if(ray_angle > 180)
    {
        horizontal_projection.position.y = (((int)player_position.y >> (int)log2(CELL_SIZE)) << (int)log2(CELL_SIZE)) - 0.0001;
        horizontal_projection.position.x = (player_position.y - horizontal_projection.position.y) * aTan + player_position.x;

        horizontal_projection.offset.y = -CELL_SIZE;
    }

    // looking down
    if(ray_angle < 180)
    {
        horizontal_projection.position.y = (((int)player_position.y >> (int)log2(CELL_SIZE)) << (int)log2(CELL_SIZE)) + CELL_SIZE;
        horizontal_projection.position.x = (player_position.y - horizontal_projection.position.y) * aTan + player_position.x;

        horizontal_projection.offset.y = CELL_SIZE;
    }

    horizontal_projection.offset.x = -horizontal_projection.offset.y * aTan;

    // looking straight left or right will never hit a horizontal line
    if((ray_angle == 0) || (ray_angle == 180))
    {
        horizontal_projection.position = player_position;
    }

    return horizontal_projection;
}

RAY get_vertical_ray(Vector2 player_position, float ray_angle)
{
    RAY ray;
    // since we need to calculate the O of the triangle and Tan(theta) = O/A, then O = Tan(theta) * A 
    float nTan = -tan(ray_angle * DEG2RAD);
            
    // looking left
    if(ray_angle < 270 && ray_angle > 90)
    {
        ray.position.x = (((int)player_position.x >> (int)log2(CELL_SIZE)) << (int)log2(CELL_SIZE)) - 0.0001;
        ray.position.y = (player_position.x - ray.position.x) * nTan + player_position.y; 

        ray.offset.x = -CELL_SIZE;
    }

    // looking right
    if(ray_angle < 90 || ray_angle > 270)
    {
        ray.position.x = (((int) player_position.x >> (int)log2(CELL_SIZE)) << (int)log2(CELL_SIZE)) + CELL_SIZE;
        ray.position.y = (player_position.x  - ray.position.x) * nTan + player_position.y; 

        ray.offset.x = CELL_SIZE;
    }

    ray.offset.y = -ray.offset.x * nTan;

    // looking striaght up or down will never hit a vertical line
    if(ray_angle == 270 || ray_angle == 90)
    {
        ray.position = player_position;
    }

    return ray;
}

void calculate_ray_distances(Grid map[MAP_SIZE], Player *player, float distances[NRAYS])
{
    float ray_angle = angle_correction(player->angle - 30);

    for(int r = 0; r < NRAYS; r++, ray_angle = angle_correction(ray_angle + 0.50))
    {
        RAY horizontal_ray = get_horizontal_ray(player->position, ray_angle); 
        Vector2 horizontal_collision_point = find_collision_point(map, horizontal_ray);
        float horizontal_distance = Vector2Distance(player->position, horizontal_collision_point);

        RAY vertical_ray = get_vertical_ray(player->position, ray_angle);
        Vector2 vertical_collision_point = find_collision_point(map, vertical_ray);
        float vertical_distance = Vector2Distance(player->position, vertical_collision_point);

        // choose the shortest collision path to draw
        float final_distance;
        // Vector2 collision_point;
        
        if(horizontal_distance < vertical_distance)
        {
            final_distance = horizontal_distance;
            // collision_point = horizontal_collision_point;
        }

        else
        {
            final_distance = vertical_distance;
            // collision_point = vertical_collision_point;
        }
        
        // 2D ray render
        // DrawLineEx(player->position, collision_point, 1, ORANGE);
        
        // prevent fisheye effect
        float angle_difference = angle_correction(player->angle - ray_angle);
        final_distance *= cos(angle_difference * DEG2RAD);
        
        distances[r] = final_distance;
    }
}

typedef enum
{
    _2D = 0,
    _3D = 1,
} Dimension;

void change_dimension(Dimension *dimension)
{
    if(IsKeyPressed(KEY_KP_2))
    {
        EnableCursor();
        *dimension = _2D;
    }

    else if(IsKeyPressed(KEY_KP_3))
    {
        DisableCursor();
        *dimension = _3D;
    }
}

void init()
{
    InitWindow(SCR, SCR, "Raycast Engine");
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(FPS); 
}

int main() 
{
    init();

    Grid map[MAP_SIZE]; init_map(map);
    Player player = init_player();
    Dimension dimension = _2D;
    float distances[NRAYS];

    while(!WindowShouldClose())
    {
        // only calculate ray distances during change in angle or player position
        if(abs((int)GetMouseDelta().x) > 0 || !Vector2Equals(Vector2Subtract(player.position, player.previous_position), Vector2Zero()))
        {
            calculate_ray_distances(map, &player, distances); 
        }
        change_dimension(&dimension);
        movement_controls(map, &player);

        BeginDrawing();
            ClearBackground(BLACK);
            switch(dimension)
            {
                case _2D: 
                    edit_grid(map, player.position, player.radius);
                    draw_grid(map);
                    draw_player(&player);
                    break;
                
                case _3D:
                    draw_world(distances);
                    DrawCircleSector(CENTER, 3, 0, 360, 1, GREEN);
                    DrawFPS(0, 0);
                    break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// TODO
// change movement behavior when in 2d and 3d
// clean 3d render