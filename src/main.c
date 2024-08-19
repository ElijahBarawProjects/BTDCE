 /*
 *--------------------------------------
 * Program Name: BTD
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

// converted graphics files
#include "gfx/gfx.h"


void handleKeys() {
    kb_Scan();

}

void drawMap() {
    gfx_SetColor(158);
    // x, y, width, height
    gfx_FillRectangle(0,0,320,240);
    
}

void drawMonkeys() {
    
}

void drawBloons() {

}

void drawProjectiles() {

} 

int main(void) {
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(1);
    drawMap();
    gfx_TransparentSprite(dart1, 0, 0);
    gfx_TransparentSprite(base, 160, 120);
    gfx_End();
    return 0;
}