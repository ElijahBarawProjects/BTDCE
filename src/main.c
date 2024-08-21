 /*
 *--------------------------------------
 * Program Name: BTDCE
 * Author: Everyday Code & EKB
 * License:
 * Description: "BTD remake for the TI-84 Plus CE."
 *--------------------------------------
*/

// standard libraries
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

// graphing calc libraries
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <debug.h> // for dbg_printf()

// converted graphics files
#include "gfx/gfx.h"

// our code
#include "structs.h"
#include "path.h"
#include "utils.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// in the future, add type of monkey being initialized
tower_t* initTower(game_t *game) {
    tower_t *tower = safe_malloc(sizeof(tower_t), __LINE__);
    tower->attack_speed = 1;
    tower->position = game->cursor;
    tower->target = FIRST;
    tower->radius = 20;

    return tower;
}

bloon_t* initBloon(game_t* game) {
    bloon_t *bloon = safe_malloc(sizeof(bloon_t), __LINE__);
    bloon->position.x = 0 - base->width;
    bloon->position.y = ((game->path)->points)[0].y;
    bloon->speed = 3;
    bloon->segment = 0;

    return bloon;
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
    gfx_Circle(x + (dart1->width / 2), y + (dart1->height / 2) , 50);
}

void drawMap(game_t* game) {
    gfx_SetColor(158);
    // x, y, width, height
    gfx_FillRectangle(0,0,320,240);
}

void drawStats(game_t *game) {
    const size_t PADDING = 3;
    size_t chars_drawn = 0;
    size_t x_start = 10;
    size_t x_off = x_start;
    size_t y_off = 10;
    const size_t number_width = gfx_GetStringWidth("xxx");

    const char* hearts_msg = "Hearts: ";
    gfx_PrintStringXY(hearts_msg, x_off, y_off);
    x_off += gfx_GetStringWidth(hearts_msg);
    gfx_PrintInt(game->hearts, 1);
    x_off += number_width + PADDING;

    const char* round_msg = "Round: ";
    gfx_PrintStringXY(round_msg, x_off, y_off);
    x_off += gfx_GetStringWidth(round_msg);
    gfx_PrintInt(game->round, 1);
    x_off += number_width + PADDING;

    const char* coinsMsg = "Coins: ";
    gfx_PrintStringXY(coinsMsg, x_off, y_off);
    x_off += gfx_GetStringWidth(coinsMsg);
    gfx_PrintInt(game->coins, 1);
    x_off += number_width + PADDING;

    // make a new line
    const uint8_t FONT_HEIGHT = 8;
    y_off += FONT_HEIGHT * 2;
    x_off = x_start;
    
    const char* bloonsMSg = "Bloons: ";
    gfx_PrintStringXY(bloonsMSg, x_off, y_off * 3);
    x_off += gfx_GetStringWidth(bloonsMSg);

    size_t num_bloons = 0;
    int len = 0;
    bloon_t *tmp_bloon = game->bloons;
    while (tmp_bloon != NULL) {
        num_bloons++;
        len++;
        tmp_bloon = tmp_bloon->next;
    }
    dbg_printf("There are %lu bloons on screen\n", num_bloons);
    gfx_PrintInt(len, 1);

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
        gfx_TransparentSprite(base, tmp_bloon->position.x, tmp_bloon->position.y - (base->height / 2));

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
    const int NUM_ROUNDS = 50;
    round_t* rounds = safe_malloc(sizeof(round_t) * NUM_ROUNDS, __LINE__);

    size_t total_num_bloons = 0;

    const int ROUND_ONE_BLOONS = 3;
    
    for (int i = 0; i < NUM_ROUNDS; i++) {
        rounds[i].num_bloons = 0;
        if (i == 0) {
            rounds[i].max_bloons = ROUND_ONE_BLOONS;
        } else {
            rounds[i].max_bloons =  50; //(double) (1.5 * rounds[i - 1].max_bloons);
        }

        rounds[i].delay = 10;
        rounds[i].tick = 0;
        rounds[i].bloons = safe_malloc(sizeof(bloon_t) * rounds[i].max_bloons, __LINE__);

        if (rounds[i].bloons == NULL) {
            dbg_printf("MALLOCED %lu BALOONS FUCK\n", total_num_bloons);
        }

        total_num_bloons += rounds[i].max_bloons;

        for (int j = 0; j < rounds[i].max_bloons; j++) {
            rounds[i].bloons[j].sprite = base;
        }
    }

    dbg_printf("MALLOCED %lu BALOONS OK\n", total_num_bloons);


    return rounds;
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

void updateBloons(game_t *game) {
    bloon_t *tmp_bloon = game->bloons;
    bloon_t *prev_bloon = NULL;

    const int BLOON_VALUE = 1;
    const int num_points = game->path->num_points;

    while (tmp_bloon != NULL) {
        int segment = tmp_bloon->segment;

        if (segment < num_points - 1) { // Ensure there's a next segment to target
            int x_target = game->path->points[segment + 1].x;
            int y_target = game->path->points[segment + 1].y;
            int x = tmp_bloon->position.x;
            int y = tmp_bloon->position.y;

            // Calculate direction vector to the next point
            int dx = x_target - x;
            int dy = y_target - y;
            float distance = sqrt(dx * dx + dy * dy);

            // Normalize the direction vector and move the bloon along it
            if (distance != 0) {
                tmp_bloon->position.x += (int)(tmp_bloon->speed * dx / distance);
                tmp_bloon->position.y += (int)(tmp_bloon->speed * dy / distance);
            }

            // Check if bloon has reached (or passed) the target point
            if (abs(tmp_bloon->position.x - x_target) <= tmp_bloon->speed &&
                abs(tmp_bloon->position.y - y_target) <= tmp_bloon->speed) {
                tmp_bloon->segment++; // Move to the next segment

                // If no more segments, the bloon reaches the end
                if (tmp_bloon->segment >= num_points - 1) {
                    game->hearts -= BLOON_VALUE;
                    // Remove the bloon from the list
                    if (prev_bloon != NULL) {
                        prev_bloon->next = tmp_bloon->next;
                    } else {
                        game->bloons = tmp_bloon->next;
                    }
                    bloon_t *to_free = tmp_bloon;
                    tmp_bloon = tmp_bloon->next;
                    free(to_free);
                    continue;
                }
            }
        }

        prev_bloon = tmp_bloon;
        tmp_bloon = tmp_bloon->next;
    }
}



void handleGame(game_t* game) {
    handleKeys(game);

    if (game->hearts <= 0) game->exit = true;

    if (game->rounds[game->round].num_bloons == game->rounds[game->round].max_bloons) {
        game->round++;   
    }
    
    if (game->round == 101) game->exit = true;

    game->rounds[game->round].tick++;
    if ((game->rounds[game->round].tick % game->rounds[game->round].delay == 0) && 
        game->rounds[game->round].num_bloons < game->rounds[game->round].max_bloons) {
        // initBloon
        bloon_t *tmp_bloon = game->bloons;
        bloon_t *new_bloon = initBloon(game);

        game->bloons = new_bloon;
        new_bloon->next = tmp_bloon;
        
        game->rounds[game->round].num_bloons += 1;
    }

    // move bloons, remove any that go off screen by reducing game health by 1
    updateBloons(game);

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

        handleGame(game);

        drawMap(game);
        drawGamePath(game);
        drawStats(game);
        drawTowers(game);
        drawBloons(game);
        drawProjectiles(game);
        drawCursor(game);

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