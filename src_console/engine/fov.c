/* =====================================================================
   FOV - Recursive shadowcasting implementation
   Source: standard roguelike algorithm (8 octants, multipliers).
   ===================================================================== */
#include "fov.h"

/* 8 octant multipliers */
static const int MULT[8][4] = {
    { 1, 0,  0, 1 },
    { 0, 1,  1, 0 },
    { 0,-1,  1, 0 },
    {-1, 0,  0, 1 },
    {-1, 0,  0,-1 },
    { 0,-1, -1, 0 },
    { 0, 1, -1, 0 },
    { 1, 0,  0,-1 },
};

static void cast_light(Map *m, int cx, int cy, int row, float start, float end,
                       int radius, int xx, int xy, int yx, int yy){
    if(start < end) return;
    float new_start = 0.0f;
    int radius_sq = radius * radius;
    for(int r = row; r <= radius; r++){
        int dx = -r - 1;
        int dy = -r;
        float blocked = 0.0f;
        while(dx <= 0){
            int X = cx + dx * xx + dy * xy;
            int Y = cy + dx * yx + dy * yy;
            float l_slope = (dx - 0.5f) / (dy + 0.5f);
            float r_slope = (dx + 0.5f) / (dy - 0.5f);
            if(start < r_slope) { dx++; continue; }
            if(end > l_slope) break;
            /* cell trong FOV */
            if((int)(dx*dx + dy*dy) <= radius_sq){
                map_mark_visible(m, X, Y);
            }
            if(blocked){
                if(!map_transparent(m, X, Y)){
                    new_start = r_slope;
                    dx++;
                    continue;
                } else {
                    blocked = 0;
                    start = new_start;
                }
            }
            if(!map_transparent(m, X, Y) && r < radius){
                blocked = 1;
                cast_light(m, cx, cy, r+1, start, l_slope, radius, xx, xy, yx, yy);
                new_start = r_slope;
            }
            dx++;
        }
        if(blocked) break;
    }
}

void fov_compute(Map *m, int cx, int cy, int radius){
    map_clear_visible(m);
    /* Center luon visible */
    map_mark_visible(m, cx, cy);
    /* Cast 8 octants */
    for(int oct = 0; oct < 8; oct++){
        cast_light(m, cx, cy, 1, 1.0f, 0.0f, radius,
                   MULT[oct][0], MULT[oct][1], MULT[oct][2], MULT[oct][3]);
    }
}
