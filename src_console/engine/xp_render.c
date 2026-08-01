/* =====================================================================
   XP RENDER - Implementation
   ===================================================================== */
#include "xp_render.h"
#include "console.h"
#include <math.h>

/* 16-color CGA palette RGB (match gdi_renderer.c CE_RGB) */
static const int PAL_R[16] = {0,0,0,0,168,168,168,168,84,84,84,84,252,252,252,252};
static const int PAL_G[16] = {0,0,168,168,0,0,84,168,84,84,252,252,84,84,252,252};
static const int PAL_B[16] = {0,168,0,168,0,168,0,168,84,252,84,252,84,252,84,252};

int xp_rgb_to_ce(int r, int g, int b){
    int best = 0, best_d = 0x7FFFFFFF;
    for(int i = 0; i < 16; i++){
        int dr = r - PAL_R[i], dg = g - PAL_G[i], db = b - PAL_B[i];
        int d = dr*dr + dg*dg + db*db;
        if(d < best_d){ best_d = d; best = i; }
    }
    return best;
}

void xp_draw_layer(int ox, int oy, const XpLayer *layer){
    if(!layer) return;
    for(int y = 0; y < layer->h; y++){
        for(int x = 0; x < layer->w; x++){
            const XpCell *c = &layer->cells[(size_t)y * layer->w + x];
            if(c->cp == 0) continue;   /* empty */
            /* Background: magenta = transparent, khong ve bg */
            int transparent_bg = (c->br == 255 && c->bg == 0 && c->bb == 255);
            int fg = xp_rgb_to_ce(c->fr, c->fg, c->fb);
            int bg = transparent_bg ? 0 : xp_rgb_to_ce(c->br, c->bg, c->bb);
            /* Dung ce_put: neu bg transparent, van de bg black (limit 16-color) */
            ce_put(ox + x, oy + y, (WCHAR)c->cp, fg, bg);
        }
    }
}

void xp_draw_file(int ox, int oy, const XpFile *f){
    if(!f) return;
    /* Composite bottom-up: layer sau de len layer truoc */
    for(int L = 0; L < f->layer_count; L++){
        xp_draw_layer(ox, oy, &f->layers[L]);
    }
}
