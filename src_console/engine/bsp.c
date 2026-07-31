/* =====================================================================
   BSP - Implementation (random rooms + L-corridors)
   ===================================================================== */
#include "bsp.h"

static int rooms_overlap(const Room *a, const Room *b, int pad){
    /* Overlap if intersecting rectangles (with padding) */
    return (a->x - pad < b->x + b->w) &&
           (a->x + a->w + pad > b->x) &&
           (a->y - pad < b->y + b->h) &&
           (a->y + a->h + pad > b->y);
}

static void carve_room(Map *m, const Room *r){
    for(int y = r->y; y < r->y + r->h; y++){
        for(int x = r->x; x < r->x + r->w; x++){
            map_set(m, x, y, TILE_FLOOR);
        }
    }
}

static void carve_corridor(Map *m, int x1, int y1, int x2, int y2){
    /* L-shape: di ngang truoc, roi doc */
    int x = x1;
    while(x != x2){
        if(map_get(m, x, y1) == TILE_WALL) map_set(m, x, y1, TILE_FLOOR);
        x += (x < x2) ? 1 : -1;
    }
    int y = y1;
    while(y != y2){
        if(map_get(m, x2, y) == TILE_WALL) map_set(m, x2, y, TILE_FLOOR);
        y += (y < y2) ? 1 : -1;
    }
}

int bsp_generate(Map *m, RNG *rng, int n_rooms, Room *rooms_out){
    if(!m || n_rooms <= 0) return 0;
    /* Toan wall truoc */
    map_fill(m, TILE_WALL);

    int placed = 0;
    int attempts = 0;
    int max_attempts = n_rooms * 10;

    while(placed < n_rooms && attempts < max_attempts){
        attempts++;
        int rw = rng_range(rng, 4, 9);
        int rh = rng_range(rng, 4, 7);
        int rx = rng_range(rng, 1, m->w - rw - 2);
        int ry = rng_range(rng, 1, m->h - rh - 2);
        Room r = { rx, ry, rw, rh, rx + rw/2, ry + rh/2 };

        /* Kiem tra overlap voi phong da dat (pad 1) */
        int ok = 1;
        for(int i = 0; i < placed; i++){
            if(rooms_overlap(&r, &rooms_out[i], 1)){ ok = 0; break; }
        }
        if(!ok) continue;

        rooms_out[placed] = r;
        carve_room(m, &r);
        /* Noi voi phong truoc (center to center) */
        if(placed > 0){
            Room *prev = &rooms_out[placed - 1];
            carve_corridor(m, prev->cx, prev->cy, r.cx, r.cy);
        }
        placed++;
    }
    return placed;
}
