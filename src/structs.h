#ifndef STRUCTS_H
#define STRUCTS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <graphx.h>
#include <stdbool.h>

/*
Point on the canvas
(0, 0) => top left corner of canvas
(319, 239) => bottom right corner of canvas
*/
typedef struct {
    int16_t x;
    int16_t y;
} position_t;

typedef struct {
    // four points
    position_t upper_left;
    position_t lower_left;
    position_t upper_right;
    position_t lower_right;

    enum {HORZ, VERT, DIAG} kind;

} rectangle_t;

typedef struct {
    position_t* points; // the points which make up the piecewise path
    rectangle_t* rectangles;
    size_t num_points; // length of points
    int length; // sum of lengths of line segments
    int width; // width of the path
} path_t;

typedef struct bloon_t {
   position_t position;
   struct bloon_t *next;
   gfx_sprite_t *sprite;
   int speed;
   uint16_t segment;    // index of the current path segment
   uint16_t progress_along_segment; // how far along the current path segment the bloon is 
} bloon_t;

typedef struct  {
    struct tower_t *next;
    position_t position;
    int attack_speed;
    int angle;
    int tick;
    int pop_count;
    int pierce;
    enum { FIRST, LAST, STRONG, CLOSE } target;
    int radius;
} tower_t;

typedef struct {
    struct projectile_t *next;
    position_t position;
    gfx_sprite_t *sprite;
    int speed;
    int angle;
    int pierce;
    // need the vector (speed + direction)
    int direction;
} projectile_t;

typedef struct  {
    int max_bloons;
    int num_bloons;
    int delay;
    int tick;
    // dynamic sized array based on number of bloons in that round
    bloon_t *bloons;
} round_t;

typedef struct {
    path_t* path;
    int hearts;
    int coins;
    tower_t* towers;
    bloon_t* bloons;
    projectile_t* projectiles;
    round_t* rounds;
    bool exit;
    enum {SELECTED, INFO, NONE} cursor_type;
    position_t cursor;
    int round;

    bool AUTOPLAY;
    bool SHOW_STATS;
} game_t;

#ifdef __cplusplus
}
#endif

#endif