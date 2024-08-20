 /*
 *--------------------------------------
 * Program Name: BTDCE
 * Author: Everyday Code
 * License:
 * Description: "BTD remake for the TI-84 Plus CE."
 *--------------------------------------
*/

#include <math.h>
// #include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug.h> // for dbg_printf()
#include "structs.h"

// converted graphics files
#include "gfx/gfx.h"


position_t default_path[] = {
    {0, 113},
    {64, 113},
    {64, 54},
    {140, 54},
    {140, 174},
    {36, 174},
    {36, 216},
    {288, 216},
    {288, 149},
    {206, 149},
    {206, 94},
    {290, 94},
    {290, 28},
    {180, 28},
    {180, 0}
};

void drawPath(game_t *game_state) {
    return;
}

// in the future, add type of monkey being initialized
tower_t* initTower(game_t *game) {
    tower_t *tower = malloc(sizeof(tower_t));
    tower->attack_speed = 1;
    tower->position = game->cursor;
    tower->target = FIRST;
    tower->radius = 20;

    return tower;
}

void handleKeys(game_t* game) {
    kb_Scan();
    
    if (kb_Data[7] & kb_Up) {
        game->cursor.y--; 
    } else if (kb_Data[7] & kb_Down) {
        game->cursor.y++;
    } else if (kb_Data[7] & kb_Left) {
        game->cursor.x--;
    } else if (kb_Data[7] & kb_Right) {
        game->cursor.x++;
    }
    
    if (kb_Data[6] & kb_Enter) {
        
        if (game->cursor_type == SELECTED) {
            // place the monkey
            // check if the bounds are correct and this does not fall on the path
            // base cost of the tower, replace
            int base_cost = 100;
            if (game->coins >= base_cost) game->coins -= base_cost;
            
            tower_t *tower = initTower(game);
            tower_t *tmp_tower = game->towers;
            if (game->towers == NULL) {
                game->towers = tower;
                tower->next = tmp_tower;
            } else {
                game->towers = tower;
                tower->next = tmp_tower;
            }
            game->cursor_type = NONE;


        } else {
            game->cursor_type = SELECTED;
        }
    } 

    if (kb_Data[1] & kb_Mode) game->SHOW_STATS = !game->SHOW_STATS;

    if (kb_Data[6] & kb_Clear) game->exit = true;
}

void drawCursor(game_t* game) {
    int x = game->cursor.x;
    int y = game->cursor.y;
    gfx_TransparentSprite(dart1, x, y);
    gfx_SetColor(255);
    // replace with selected tower radius
    gfx_Circle(x, y, 100);
}

void drawMap(game_t* game) {
    gfx_SetColor(158);
    // x, y, width, height
    gfx_FillRectangle(0,0,320,240);
    gfx_PrintStringXY("Hearts: ",10, 10);
    gfx_PrintInt(game->hearts, 1);
    gfx_PrintStringXY("Round: ", 10 + 10 + gfx_GetStringWidth("Hearts: 100"), 10);
    gfx_PrintInt(game->round, 1);
    gfx_PrintStringXY("Coins: ", 10 + gfx_GetStringWidth("Hearts: 100") + gfx_GetStringWidth("Round: 100"), 10);
    gfx_PrintInt(game->coins, 1);
}

void drawTowers(game_t *game) {
    tower_t* tmp_tower = game->towers; 

    while (tmp_tower != NULL) {
        gfx_TransparentSprite(dart1, tmp_tower->position.x, tmp_tower->position.y);
        tmp_tower = tmp_tower->next;
    }
}

void drawBloons(game_t *game) {
    bloon_t* tmp_bloon = game->bloons; 

    while (tmp_bloon != NULL) {
        gfx_TransparentSprite(dart1, tmp_bloon->position.x, tmp_bloon->position.y);
        tmp_bloon = tmp_bloon->next;
    }
}

void drawProjectiles(game_t *game) {

} 

void drawExitScreen(game_t *game) {
    // draw the amount of bloons that got through
    // count bloons
    
    // draw button
}

int16_t distance(position_t p1, position_t p2) {
    if (p1.x == p2.x) {
        // vert line
        return (int16_t)abs(p1.y - p2.y);
    } else if (p1.y == p2.y) {
        // horiz line
        return (int16_t)abs(p1.x - p2.x);
    } else {
        // neither
        double dist = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
        return (int16_t)round(dist);
    }
}



int pathLength(path_t* path){
    int len = 0;
    // loop over line segments
    for (size_t i = 0; i < (path->num_points - 1); i ++){
        // i => start of line segment, i+1 => end of line segment
        len += distance((path->points)[i], (path->points)[i+1]);
    }
    return len;
}

path_t* newPath(position_t* points, size_t num_points) {
    if (points == NULL || num_points == 0) {
        // use default path
        points = &default_path;
        num_points =sizeof(default_path) / sizeof(position_t);
    }
    path_t* path = malloc(sizeof(path_t));
    path->num_points = num_points;
    path->points = points;
    path->length = pathLength(path);
    dbg_printf("The path has total length %d", path->length);
    return path;
}


game_t* newGame(position_t* points, size_t num_points){
    /**
     *  typedef struct  {
        path_t* path;
        int hearts;
        int coins;
        tower_t* towers;
        bloon_t* bloons;
        projectile_t* projectiles;
        bool exit;
        enum {SELECTED, INFO, NONE} cursor_type;
        position_t cursor;
        int round;
        bool AUTOPLAY;
        bool DEBUG;
} game_t;
     */

    game_t* game = malloc(sizeof(game_t));
    game->path = newPath(points, num_points);
    // choose hearts based on game difficulty, to be added
    game->hearts = 100;
    game->coins = 500;
    
    game->towers = NULL;
    game->bloons = NULL;
    
    game->projectiles = NULL;
    game->exit=false;

    position_t start_cursor = {160, 120};
    game->cursor=start_cursor;
    game->cursor_type=NONE;


    game->round = 1;

    game->AUTOPLAY = false;
    game->SHOW_STATS = false;
    

    return game;
}

void handleGame(game_t* game) {
    handleKeys(game);

    if (game->hearts <= 0) game->exit = true;

    
}


void exitGame(game_t* game) {
    // loop through bloons, towers, projectiles array
    bloon_t *tmp_bloon = game->bloons;
    bloon_t *curr_bloon = NULL; 
    tower_t *tmp_tower = game->towers;
    tower_t *curr_tower = NULL;
    projectile_t *tmp_projectile = game->projectiles;
    projectile_t *curr_projectile = game->projectiles;

    while (curr_bloon != NULL) {
        tmp_bloon = curr_bloon->next;
        free(curr_bloon);
        curr_bloon = tmp_bloon;
    }

    while (curr_tower != NULL) {
        tmp_tower = curr_tower->next;
        free(curr_tower);
        curr_tower = tmp_tower;
    }

    while (curr_projectile != NULL) {
        tmp_projectile = curr_projectile->next;
        free(curr_projectile);
        curr_projectile = tmp_projectile;
    }

    // save any game elements before freeing
    free(game);
}

void runGame(void) {
    game_t *game = newGame(NULL, 0); // use default path

    

    // ugly code for clarity
    while (game->exit == false) {
        drawMap(game);
        drawTowers(game);
        drawBloons(game);
        drawProjectiles(game);
        drawCursor(game);

        handleGame(game);
        gfx_BlitBuffer();
    }

    drawExitScreen(game);

    exitGame(game);

}

int main(void) {
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(1);

    gfx_SetDrawBuffer();

    runGame();

    gfx_End();

    return 0;
}