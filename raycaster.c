#include <stdio.h>
#include <stdlib.h>

#include "headers/raylib.h"
#include "headers/raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

#define CELL_SIZE 64
#define NRAYS 180
#define ROW 16
#define COL 16
#define MAP_SIZE (ROW * COL)

const int FPS = 60;
const float SCR = CELL_SIZE * ROW;
const Vector2 CENTER = {(SCR / 2.0), (SCR / 2.0)};

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
    for(int i = 0; i < ROW; i++)
    {
        for(int j = 0; j < COL; j++)
        {
            DrawRectangleRec(map[(i * ROW) + j].bounds, map[(i * ROW) + j].type == WALL ? RAYWHITE : GRAY);
            DrawLine((j * CELL_SIZE), 0, (j * CELL_SIZE), SCR, BLACK);
        }

        DrawLine(0, (i * CELL_SIZE), SCR, (i * CELL_SIZE), BLACK);
    }
}

typedef enum
{
    HORIZONTAL = 0,
    VERTICAL = 1,
} Collision;

void draw_world(float distances[NRAYS], Collision collision_type[NRAYS], float vertical_offset, float fov)
{
    const float LINE_THICKNESS = (SCR / (NRAYS - (fov * 2)));

    for(int i = fov; i < (NRAYS - fov); i++)
    {
        // the length of each line drawn will be the height of the block proportional to the screens height divided by how far the player is from the wall
        float line_height = (CELL_SIZE * GetScreenHeight() * 0.50) / distances[i]; 

        // starting point of line grows downward
        float line_start = vertical_offset + (GetScreenHeight() / 2.00) - (line_height / 2.0);
            
        // 3D render
        // grass
        DrawLineEx((Vector2){((i - fov) * LINE_THICKNESS), line_start}, (Vector2){((i - fov) * LINE_THICKNESS),  SCR}, LINE_THICKNESS, GREEN);
        // sky
        DrawLineEx((Vector2){((i - fov) * LINE_THICKNESS), (line_start + line_height)}, (Vector2){((i - fov) * LINE_THICKNESS),  0.00}, LINE_THICKNESS, BLUE);   
        // wall
        DrawLineEx((Vector2){((i - fov) * LINE_THICKNESS), line_start}, (Vector2){((i - fov) * LINE_THICKNESS),  (line_start + line_height)}, LINE_THICKNESS, (collision_type[i] == HORIZONTAL) ? RED : MAROON);   
    }
}

typedef struct
{
    Vector2 position;
    Vector2 previous_position;
    float angle;
    float radius;
    float speed;
    float horizontal_turn_speed;
    float vertical_turn_speed;
    float vertical_offset;
} Player;

Player init_player()
{
    Player player;

    player.position = CENTER;
    player.angle = 0.00;
    player.radius = 5.00;
    player.speed = 250.00;
    player.horizontal_turn_speed = 1.50;
    player.vertical_turn_speed = 7.50;
    player.vertical_offset = 0.00;

    return player;
}

void draw_player(Player *player)
{
    DrawCircleSector(player->position, player->radius, 0, 360, 1, BLUE);
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

typedef enum
{
    _2D = 0,
    _3D = 1,
} Dimension;

void change_direction(Player *player, Dimension dimension)
{
    float delta_x = GetMouseDelta().x;
    float delta_y = GetMouseDelta().y;

    switch(dimension)
    {
        case _2D:
            player->angle = get_angle(GetMousePosition(), player->position);
            break;
        
        case _3D:
            if(abs((int)delta_x) > 0)
            { 
                if(delta_x < 0)
                {
                    player->angle -= player->horizontal_turn_speed;
                }

                else
                {
                    player->angle += player->horizontal_turn_speed;
                }
            }

            if(abs((int)delta_y) > 0)
            {
                if(delta_y < 0)
                {
                    player->vertical_offset += player->vertical_turn_speed;
                }

                else
                {
                    player->vertical_offset -= player->vertical_turn_speed;
                }
            }
            break;
    }

    player->angle = angle_correction(player->angle);
}

void movement_controls(Grid map[MAP_SIZE], Player *player)
{
    if(IsKeyDown(KEY_W))
    {
        move_player(map, player, player->angle);
    }

    if(IsKeyDown(KEY_A))
    {
        move_player(map, player, (player->angle - 90.00));
    }

    if(IsKeyDown(KEY_S))
    {
        move_player(map, player,  (player->angle - 180.00));
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
        if(((mp > 0) && (mp < MAP_SIZE)) && (map[mp].type == WALL))
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

void calculate_ray_distances(Grid map[MAP_SIZE], Player *player, Vector2 rays[NRAYS], float distances[NRAYS], Collision collisions[NRAYS])
{
    float ray_angle = angle_correction(player->angle - 45);

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
        Vector2 collision_point;
        
        if(horizontal_distance < vertical_distance)
        {
            final_distance = horizontal_distance;
            collision_point = horizontal_collision_point;
            collisions[r] = HORIZONTAL;
        }

        else
        {
            final_distance = vertical_distance;
            collision_point = vertical_collision_point;
            collisions[r] = VERTICAL;
        }
        
        // prevent fisheye effect
        float angle_difference = angle_correction(player->angle - ray_angle);
        final_distance *= cos(angle_difference * DEG2RAD);
        
        rays[r] = collision_point;
        distances[r] = final_distance;
    }
}

void draw_rays(Vector2 rays[NRAYS], Vector2 player_position, float fov)
{
    for(int i = fov; i < (NRAYS - fov); i++)
    {
        DrawLineV(player_position, rays[i], ORANGE);
    }
}

void change_dimension(Dimension *dimension)
{
    if(IsKeyPressed(KEY_KP_2))
    {
        *dimension = _2D;
    }

    else if(IsKeyPressed(KEY_KP_3))
    {
        *dimension = _3D;
    }
}

void toggle_cursor()
{
    if(IsKeyPressed(KEY_E))
    {
        EnableCursor();
    }

    else if(IsKeyPressed(KEY_Q))
    {
        DisableCursor();
    }
}

void init()
{
    InitWindow(SCR, SCR, "Raycast Engine");
    SetTraceLogLevel(LOG_ERROR);
    SetTargetFPS(FPS); 
}

void settings(float *movement_speed, float *horizontal_turn_speed, float *vertical_turn_speed, float *field_of_view)
{
    char text[10];

    DrawText("SPEED", 10, 10, 10, RAYWHITE);
    sprintf(text, "%.2f", *movement_speed);
    GuiSliderBar((Rectangle){MeasureText("SPEED", 10) + 15,10,100,10}, "", text, movement_speed, 1.00, 500.00);

    DrawText("HORIZONTAL TURN SPEED", 10, 25, 10, RAYWHITE);
    sprintf(text, "%.2f", *horizontal_turn_speed);
    GuiSliderBar((Rectangle){MeasureText("HORIZONTAL TURN SPEED", 10) + 15,25,100,10}, "", text, horizontal_turn_speed, 0.00, 3.00);

    DrawText("VERTICAL TURN SPEED", 10, 40, 10, RAYWHITE);
    sprintf(text, "%.2f", *vertical_turn_speed);
    GuiSliderBar((Rectangle){MeasureText("HORIZONTAL TURN SPEED", 10) + 15,40,100,10}, "", text, vertical_turn_speed, 0.00, 10.00);

    DrawText("FOV", 10, 55, 10, RAYWHITE);
    sprintf(text, "%.2f", *field_of_view);
    GuiSliderBar((Rectangle){MeasureText("FOV", 10) + 15, 55, 100, 10}, "", text, field_of_view, 0.00, 60.00);
    *field_of_view = (int)*field_of_view;
}

int main() 
{
    init();
    float field_of_view = 30;
    Grid map[MAP_SIZE]; init_map(map);
    Player player = init_player();
    Dimension dimension = _2D;
    Vector2 rays[NRAYS];
    Collision collisions[NRAYS];
    float distances[NRAYS];

    while(!WindowShouldClose())
    {
        // only calculate ray distances during change in angle or player position
        if(abs((int)GetMouseDelta().x) > 0 || !Vector2Equals(Vector2Subtract(player.position, player.previous_position), Vector2Zero()))
        {
            calculate_ray_distances(map, &player, rays, distances, collisions); 
        }
        toggle_cursor();
        change_dimension(&dimension);
        movement_controls(map, &player);
        if(IsCursorHidden())
        {
            change_direction(&player, dimension);
        }
        BeginDrawing();
            ClearBackground(BLACK);
            switch(dimension)
            {
                case _2D: 
                    edit_grid(map, player.position, player.radius);
                    draw_grid(map);
                    draw_rays(rays, player.position, field_of_view);
                    draw_player(&player);
                    break;
                case _3D:
                    ClearBackground(BLACK);
                    draw_world(distances, collisions, player.vertical_offset, field_of_view);
                    DrawCircleSector(CENTER, 3, 0, 360, 1, BLACK);
                    settings(&player.speed, &player.horizontal_turn_speed, &player.vertical_turn_speed, &field_of_view);
                    DrawFPS((SCR - 80), 0);
                    break;
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
// change lighting