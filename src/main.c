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

    gfx_SetDrawBuffer();

    uint8_t position = 0;
    
    while (kb_Data[6] != kb_Clear) {
        drawMap();
        gfx_TransparentSprite(dart1, 0, 0);
        gfx_TransparentSprite(base, position++, 120);
        gfx_BlitBuffer();
    }
    
    gfx_End();
    return 0;
}