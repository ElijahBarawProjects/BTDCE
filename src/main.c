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
#include <stdbool.h>
#include <stdlib.h>

// graphing calc libraries
#include <debug.h>  // for dbg_printf()
#include <fileioc.h>
#include <graphx.h>
#include <keypadc.h>

// converted graphics files
#include "gfx/gfx.h"

// our code
#include "list.h"
#include "path.h"
#include "structs.h"
#include "utils.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

tower_t *initTower(game_t *game) {
    // in the future, add type of monkey being initialized
    tower_t *tower = safe_malloc(sizeof(tower_t), __LINE__);

    tower->position = game->cursor;
    tower->attack_speed = 1;
    tower->target = FIRST;
    tower->radius = 20;
    return tower;
}

bloon_t *initBloon(game_t *game) {
    bloon_t *bloon = safe_malloc(sizeof(bloon_t), __LINE__);

    bloon->position.x = 0 - base->width;
    bloon->position.y = ((game->path)->points)[0].y;
    bloon->speed = 3;
    bloon->segment = 0;
    return bloon;
}

void handleKeys(game_t *game) {
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
        int base_cost = 100;

        if ((game->cursor_type == SELECTED) &&
            (game->coins >= base_cost)) {  // player wants to place a tower &
                                           // has sufficient funds

            // charge the player
            game->coins -= base_cost;

            // TODO: check (in_bounds() && not_on_path())

            // make a new tower & insert into game->towers
            // TODO: check the tower type
            tower_t *tower = initTower(game);
            queue_insert_head(game->towers, (void *)tower);
            game->cursor_type = NONE;
        }
    }

    if (kb_Data[6] & kb_Add) {
        game->cursor_type = SELECTED;
    }

    if (kb_Data[1] & kb_Mode) game->SHOW_STATS = !game->SHOW_STATS;

    if (kb_Data[6] & kb_Clear) game->exit = true;
}

void drawCursor(game_t *game) {
    int x = game->cursor.x;
    int y = game->cursor.y;
    gfx_TransparentSprite(dart1, x, y);
    gfx_SetColor(255);
    // replace with selected tower radius
    // center it based on the tower width/height
    gfx_Circle(x + (dart1->width / 2), y + (dart1->height / 2), 50);
}

void drawMap(game_t *game) {
    gfx_SetColor(158);
    // x, y, width, height
    gfx_FillRectangle(0, 0, 320, 240);
}

void drawStats(game_t *game) {
    const size_t PADDING = 3;
    size_t chars_drawn = 0;
    size_t x_start = 10;
    size_t x_off = x_start;
    size_t y_off = 10;
    const size_t number_width = gfx_GetStringWidth("xxx");

    const char *hearts_msg = "Hearts: ";
    gfx_PrintStringXY(hearts_msg, x_off, y_off);
    x_off += gfx_GetStringWidth(hearts_msg);
    gfx_PrintInt(game->hearts, 1);
    x_off += number_width + PADDING;

    const char *round_msg = "Round: ";
    gfx_PrintStringXY(round_msg, x_off, y_off);
    x_off += gfx_GetStringWidth(round_msg);
    gfx_PrintInt(game->round, 1);
    x_off += number_width + PADDING;

    const char *coinsMsg = "Coins: ";
    gfx_PrintStringXY(coinsMsg, x_off, y_off);
    x_off += gfx_GetStringWidth(coinsMsg);
    gfx_PrintInt(game->coins, 1);
    x_off += number_width + PADDING;

    // make a new line
    const uint8_t FONT_HEIGHT = 8;
    y_off += FONT_HEIGHT * 2;
    x_off = x_start;

    const char *bloonsMSg = "Bloons: ";
    gfx_PrintStringXY(bloonsMSg, x_off, y_off * 3);
    x_off += gfx_GetStringWidth(bloonsMSg);

    size_t num_bloons = queue_size(game->bloons);
    dbg_printf("There are %zu bloons on screen\n", num_bloons);
    gfx_PrintInt((int)num_bloons, 1);
}

void drawTowers(game_t *game) {
    list_ele_t *curr_elem = game->towers->head;
    while (curr_elem != NULL) {
        tower_t *tower = (tower_t *)(curr_elem->value);
        gfx_TransparentSprite(dart1, tower->position.x, tower->position.y);
        curr_elem = curr_elem->next;
    }
}

void drawBloons(game_t *game) {
    list_ele_t *curr_elem = game->bloons->head;
    while (curr_elem != NULL) {
        bloon_t *bloon = (bloon_t *)(curr_elem->value);
        gfx_TransparentSprite(base, bloon->position.x,
                              bloon->position.y - (base->height / 2));

        curr_elem = curr_elem->next;
    }
}

void drawProjectiles(game_t *game) {}

void drawExitScreen(game_t *game) {
    // draw the amount of bloons that got through
    // count bloons

    // draw button
}

const int NUM_ROUNDS = 50;
round_t *initRounds(void) {
    round_t *rounds = safe_malloc(sizeof(round_t) * NUM_ROUNDS, __LINE__);

    size_t total_num_bloons = 0;

    const int ROUND_ONE_BLOONS = 3;

    for (int i = 0; i < NUM_ROUNDS; i++) {
        rounds[i].num_bloons = 0;
        if (i == 0) {
            rounds[i].max_bloons = ROUND_ONE_BLOONS;
        } else {
            rounds[i].max_bloons =
                50;  //(double) (1.5 * rounds[i - 1].max_bloons);
        }

        rounds[i].delay = 10;
        rounds[i].tick = 0;
        rounds[i].bloons =
            safe_malloc(sizeof(bloon_t) * rounds[i].max_bloons, __LINE__);

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

void freeRounds(round_t *rounds) {
    for (int i = 0; i < NUM_ROUNDS; i++) free(rounds[i].bloons);
    free(rounds);
}

/// @brief updates `current_position` to move in the direction of `target`,
/// until `max_dist` is reached or `target` is reached
/// @return the distance left to move (0 if no movement left)
double moveTowards(position_t *current_position, const position_t target,
                   double max_dist) {
    // move vertically
    if (current_position->x == target.x) {
        int dy = target.y - current_position->y;
        if (abs(dy) <= max_dist) {  // can move to target
            current_position->y = target.y;
            return max_dist - abs(dy);
        }

        // can move towards target but not reach it
        current_position->y += (int)(dy < 0 ? -max_dist : max_dist);
        return 0.0;
    }

    // move horizontally
    if (current_position->y == target.y) {
        int dx = target.x - current_position->x;
        if (abs(dx) <= max_dist) {  // can move to target
            current_position->x = target.x;
            return max_dist - abs(dx);
        }

        // can move towards target but not reach it
        current_position->x += (int)(dx < 0 ? -max_dist : max_dist);
        return 0.0;
    }

    // move on a diagonal (slow)
    // Calculate direction vector to next point
    int dx = target.x - current_position->x;
    int dy = target.y - current_position->y;
    double dist = sqrt(dx * dx + dy * dy);  // distance to point

    if (dist <= max_dist) {
        // can reach the point
        current_position->x = target.x;
        current_position->y = target.y;
        return max_dist - dist;
    }

    // can't quite reach the point

    // Normalize the direction vector and move the bloon along it
    current_position->x += (int)((max_dist / dist) * dx);
    current_position->y += (int)((max_dist / dist) * dy);
    return 0.0;
}

/// @brief move bloon based on its current location and speed
/// @return segment bloon is on after moving (equals bloon->segment)
int moveBloon(game_t *game, bloon_t *bloon) {
    int numSegments = game->path->num_points - 1;

    double movement_left = (double)bloon->speed;
    while (movement_left > 0.0) {
        int currSeg = bloon->segment;
        if (currSeg >= numSegments) {
            // on non-existent segment
            return currSeg;
        }

        // move towards end-point of current segment
        position_t target = (game->path->points)[currSeg + 1];
        movement_left = moveTowards(&(bloon->position), target, movement_left);
        if (bloon->position.x == target.x && bloon->position.y == target.y) {
            bloon->segment++;
        }
    }

    return bloon->segment;
}

game_t *newGame(position_t *points, size_t num_points) {
    game_t *game = safe_malloc(sizeof(game_t), __LINE__);
    game->path = newPath(points, num_points, DEFAULT_PATH_WIDTH);
    // choose hearts based on game difficulty, to be added
    game->hearts = 100;
    game->coins = 500;

    game->towers = queue_new();
    game->bloons = queue_new();
    game->projectiles = queue_new();

    game->exit = false;

    position_t start_cursor = {160, 120};
    game->cursor = start_cursor;
    game->cursor_type = NONE;

    game->round = 1;
    dbg_printf("Called init rounds!\n");

    game->rounds = initRounds();

    game->AUTOPLAY = false;
    game->SHOW_STATS = false;

    dbg_printf("Returned new game_t\n");

    return game;
}

void updateBloons(game_t *game) {
    list_ele_t *curr_elem = game->bloons->head;
    list_ele_t *tmp = NULL;

    const int BLOON_VALUE = 1;
    const int num_points = game->path->num_points;
    const int num_segments = num_points - 1;

    while (curr_elem != NULL) {
        bloon_t *curr_bloon = (bloon_t *)(curr_elem->value);
        int segBeforeMove = curr_bloon->segment;
        if (segBeforeMove >= num_segments ||             // off board now
            moveBloon(game, curr_bloon) >= num_segments  // off after moving
        ) {
            // delete bloon & count against health
            game->hearts -= BLOON_VALUE;
            tmp = curr_elem->next;

            remove_and_delete(game->bloons, curr_elem,
                              free);  // frees list_elem_t and bloon_t
            curr_elem = tmp;
            continue;
        }

        curr_elem = curr_elem->next;
    }
}

void handleGame(game_t *game) {
    handleKeys(game);

    if (game->hearts <= 0) game->exit = true;

    if (game->rounds[game->round].num_bloons ==
        game->rounds[game->round].max_bloons) {
        game->round++;
    }

    if (game->round == NUM_ROUNDS) game->exit = true;

    game->rounds[game->round].tick++;
    if ((game->rounds[game->round].tick % game->rounds[game->round].delay ==
         0) &&
        game->rounds[game->round].num_bloons <
            game->rounds[game->round].max_bloons) {

        queue_insert_head(game->bloons, initBloon(game));
        game->rounds[game->round].num_bloons += 1;
    }

    // move bloons, remove any that go off screen by reducing game health by 1
    updateBloons(game);
}

void exitGame(game_t *game) {
    // free bloons, towers, projectiles
    queue_free(game->bloons, free);
    queue_free(game->towers, free);
    queue_free(game->projectiles, free);

    freePath(game->path);
    freeRounds(game->rounds);
    
    // save any game elements before freeing

    free(game);
}

void runGame(void) {
    game_t *game = newGame(NULL, 0);  // use default path

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