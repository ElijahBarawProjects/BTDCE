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

void* safe_malloc(size_t size, int line){
    void* res = malloc(size);
    if (res == NULL){
        dbg_printf("ERROR: call to safe_malloc FAILED on line %d\n", line);
    }
    return res;
}

// in the future, add type of monkey being initialized
tower_t* initTower(game_t *game) {
    tower_t *tower = safe_malloc(sizeof(tower_t), __LINE__);
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
        } 
    } 

    if (kb_Data[6] & kb_Add) {
        game->cursor_type = SELECTED;
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
    // center it based on the tower width/height
    gfx_Circle(x, y, 50);
}

void drawMap(game_t* game) {
    gfx_SetColor(158);
    // x, y, width, height
    gfx_FillRectangle(0,0,320,240);
}

void drawStats(game_t *game) {
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

round_t *initRounds(void) {
    const int NUM_ROUNDS = 100;
    round_t* rounds = safe_malloc(sizeof(round_t) * NUM_ROUNDS, __LINE__);

    size_t total_num_bloons = 0;

    const int ROUND_ONE_BLOONS = 1;
    for (int i = 0; i < NUM_ROUNDS; i++) {
        if (i == 0) {
            rounds[i].num_bloons = ROUND_ONE_BLOONS;
        } else {
            rounds[i].num_bloons = (int) (1.25 * rounds[i - 1].num_bloons);
        }

        rounds[i].delay = 10;
        rounds[i].tick = 0;
        rounds[i].bloons = safe_malloc(sizeof(bloon_t) * rounds[i].num_bloons, __LINE__);

        if (rounds[i].bloons == NULL) {
            dbg_printf("MALLOCED %lu BALOONS FUCK", total_num_bloons);
        }

        total_num_bloons += rounds[i].num_bloons;

        for (int j = 0; j < rounds[i].num_bloons; j++) {
            rounds[i].bloons[j].sprite = base;
        }
    }

    return rounds;
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




rectangle_t* rectangleFromLineSeg(position_t p1, position_t p2, int16_t width) {
    // for points defining returned rectangle
    rectangle_t* rect = safe_malloc(sizeof(rectangle_t), __LINE__);
    position_t* upper_left =  &(rect->upper_left);
    position_t* upper_right =  &(rect->upper_right); 
    position_t* lower_left = &(rect->lower_left);
    position_t* lower_right = &(rect->lower_right);

    position_t tmp;

    // horiz line seg
    if (p1.x == p2.x) {
        rect->kind = HORZ;
        // make p1 upper, p2 lower
        if (p1.y > p2.y) {
            tmp.x = p1.x;
            tmp.y = p1.y;

            p1.x = p2.x;
            p1.y = p2.y;

            p2.x = tmp.x;
            p2.y = tmp.y;

            if (!(p1.y < p2.y)) dbg_printf("ERROR: p1, p2 not swapped successfully! %d\n", __LINE__);
        }
        // p1.y < p2.y => p1 is closer to top of canvas

        upper_left->x = p1.x - (width / 2);
        upper_left->y = p1.y;

        upper_right->x = p1.x + (width / 2);
        upper_right->y = p1.y;

        lower_left->x = p2.x - (width / 2);
        lower_left->y = p2.y;

        lower_right->x = p2.x + (width / 2);
        lower_right->y = p2.y;
    }
    // vert line seg
    else if (p1.y == p2.y) {
        rect->kind = VERT;
        // make p1 left, p2 right
        if (p1.x > p2.x) {
            tmp.x = p1.x;
            tmp.y = p1.y;

            p1.x = p2.x;
            p1.y = p2.y;

            p2.x = tmp.x;
            p2.y = tmp.y;

            if (!(p1.x < p2.x)) dbg_printf("ERROR: p1, p2 not swapped successfully! %d\n", __LINE__);
        }
        // p1.x < p2.x => p1 is left

        upper_left->y = p1.y - (width / 2);
        upper_left->x = p1.x;

        lower_left->y = p1.y + (width / 2);
        lower_left->x = p1.x;

        upper_right->y = p2.y - (width / 2);
        upper_right->x = p2.x;

        lower_right->y = p2.y + (width / 2);
        lower_right->x = p2.x;
    }
    else {
        rect->kind=DIAG;
        // make p1 left, p2 right
        if (p1.x > p2.x) {
            tmp.x = p1.x;
            tmp.y = p1.y;

            p1.x = p2.x;
            p1.y = p2.y;

            p2.x = tmp.x;
            p2.y = tmp.y;
        }
        // p1.x < p2.x => p1 is more left

        // written by chat gpt, not yet checked
        // Calculate slope of the side
        double m = (double)(p1.x - p2.x) / (p2.y - p1.y);

        // Calculate displacements along axes
        double dx = (width / sqrt(1 + (m * m))) * 0.5;
        double dy = m * dx;

        // Rounding the results to get integer coordinates
        if (dy > 0) {
            // adding dy makes it the lower point
            // subing dy makes it the higher point
            
            // left point smaller y
            upper_left->x = round(p1.x - dx);
            upper_left->y = round(p1.y - dy);

            // left point larger y
            lower_left->x = round(p1.x + dx);
            lower_left->y = round(p1.y + dy);

            // right point smaller y
            upper_right->x = round(p2.x - dx);
            upper_right->y = round(p2.y - dy);

            // right point larger y
            lower_right->x = round(p2.x + dx);
            lower_right->y = round(p2.y + dy);

        } else { // dy < 0
            // adding dy makes it the higher point
            // subing dy makes it the lower point

            // left point larger y
            lower_left->x = round(p1.x - dx);
            lower_left->y = round(p1.y - dy);

            // left point smaller y
            upper_left->x = round(p1.x + dx);
            upper_left->y = round(p1.y + dy);

            // right point smaller y
            upper_right->x = round(p2.x + dx);
            upper_right->y = round(p2.y + dy);

            // right point larger y
            lower_right->x = round(p2.x - dx);
            lower_right->y = round(p2.y - dy);
        }
    }


    return rect;
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

path_t* newPath(position_t* points, size_t num_points, int16_t width) {
    if (points == NULL || num_points == 0) {
        // use default path
        points = &default_path;
        num_points =sizeof(default_path) / sizeof(position_t);
    }
    path_t* path = safe_malloc(sizeof(path_t), __LINE__);
    path->width = width;
    path->num_points = num_points;
    path->points = points;
    path->length = pathLength(path);

    // get rectangles from points
    size_t num_rectangles = num_points - 1;
    path->rectangles = safe_malloc(sizeof(rectangle_t) * num_rectangles, __LINE__);
    for (size_t i = 0; i < num_rectangles; i++)
        (path->rectangles)[i] = *rectangleFromLineSeg(path->points[i], path->points[i+1], width);


    #ifdef DEBUG
    dbg_printf("Rectangles on path:\n");
    for (size_t i = 0; i < num_rectangles; i++) {
        rectangle_t r = (path->rectangles)[i];
        dbg_printf(
            "upper_left: (%d, %d) \t upper_right: (%d, %d)\n"
            "lower_left: (%d, %d) \t lower_right: (%d, %d)\n\n",
            r.upper_left.x, r.upper_left.y, r.upper_right.x, r.upper_right.y,
            r.lower_left.x, r.lower_left.y, r.lower_right.x, r.lower_right.y);
    }
    dbg_printf("... Done! (rect on path)\n");

    #endif

    
    

    dbg_printf("The path has total length %d\n", path->length);
    return path;
}

void draw_rectangle(rectangle_t* rect) {
    switch (rect->kind)
    {
    case HORZ:
    case VERT:
        dbg_printf(
            "Filling HORZ/VERT rectangle at at (%d, %d) with width %d and height %d\n",
            rect->upper_left.x,
            rect->upper_left.y,
            rect->upper_right.x -  rect->upper_left.x,
            rect->lower_left.y - rect->upper_left.y);

        gfx_FillRectangle(
            rect->upper_left.x,
            rect->upper_left.y,
            rect->upper_right.x -  rect->upper_left.x,
            rect->lower_left.y - rect->upper_left.y);
        break;

    case DIAG:
        // cut rectangle down center & draw two triangles
        gfx_FillTriangle(
            rect->upper_left.x,
            rect->upper_left.y,
            rect->lower_left.x,
            rect->lower_left.y,
            rect->upper_right.x,
            rect->upper_right.y);

        gfx_FillTriangle(
            rect->upper_right.x,
            rect->upper_right.y,
            rect->lower_right.x,
            rect->lower_right.y,
            rect->lower_left.x,
            rect->lower_left.y);
        break;
    
    default:
        dbg_printf("ERROR: bad rectangle kind\n");
        break;
    }
}

void drawGamePath(game_t* game){
    path_t* path = game->path;
    dbg_printf("Drawing path...\n");
    gfx_SetColor(159);
    size_t numSegments = path->num_points - 1;
    position_t segStart;
    position_t segEnd;
    for (size_t i = 0; i < numSegments; i ++) {
        segStart = path->points[i];
        segEnd = path->points[i+1];

        // draw circle at start of line segment
        gfx_FillCircle(segStart.x, segStart.y, path->width / 2);
        
        // draw segment
        draw_rectangle(&(path->rectangles[i]));

    }

    // draw end circle
    if (numSegments > 0) gfx_FillCircle(segEnd.x, segEnd.y, path->width / 2);
    dbg_printf("... Done drawing path\n");
}


#define DEFAULT_PATH_WIDTH 20 // path width in pixels
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

    game_t* game = safe_malloc(sizeof(game_t), __LINE__);
    game->path = newPath(points, num_points, DEFAULT_PATH_WIDTH);
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
    dbg_printf("Called init rounds!\n");

    game->rounds = initRounds();

    game->AUTOPLAY = false;
    game->SHOW_STATS = false;
    
    dbg_printf("Returned new game_t\n");

    return game;
}

void handleGame(game_t* game) {
    handleKeys(game);

    if (game->hearts <= 0) game->exit = true;

    if (game->rounds[game->round].bloons == game->rounds[game->round].num_bloons) {
        game->round++;   
    }
    
    if (game->rounds == 101) game->exit = true;


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
        drawGamePath(game);
        drawStats(game);
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