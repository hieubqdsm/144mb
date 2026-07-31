/* =====================================================================
   MAP - Implementation
   ===================================================================== */
#include "map.h"
#include <stdlib.h>
#include <string.h>

/* Glyph dung CP437/Unicode. Wall = block, floor = cham nho. */
const TileDef TILE_DEFS[TILE_COUNT] = {
    /* TILE_WALL */        { 0, 0, 0x2593, 7, 0 },   /* ▓ grey-on-black */
    /* TILE_FLOOR */       { 1, 1, 0x00B7, 8, 0 },   /* · darkgrey */
    /* TILE_DOOR_OPEN */   { 1, 1, 0x0027, 14, 0 },  /* ' yellow */
    /* TILE_DOOR_CLOSED */ { 0, 0, 0x002B, 14, 0 },  /* + yellow */
    /* TILE_STAIRS_DOWN */ { 1, 1, 0x003E, 11, 0 },  /* > cyan */
};

Map *map_create(int w, int h){
    Map *m = (Map*)malloc(sizeof(Map));
    if(!m) return NULL;
    m->w = w; m->h = h;
    size_t n = (size_t)w * h;
    m->tiles       = (uint8_t*)calloc(n, 1);
    m->walkable    = (uint8_t*)calloc(n, 1);
    m->transparent = (uint8_t*)calloc(n, 1);
    m->seen        = (uint8_t*)calloc(n, 1);
    m->visible     = (uint8_t*)calloc(n, 1);
    if(!m->tiles || !m->walkable || !m->transparent || !m->seen || !m->visible){
        map_destroy(m); return NULL;
    }
    return m;
}

void map_destroy(Map *m){
    if(!m) return;
    free(m->tiles); free(m->walkable); free(m->transparent);
    free(m->seen); free(m->visible);
    free(m);
}

void map_fill(Map *m, TileType t){
    size_t n = (size_t)m->w * m->h;
    for(size_t i=0;i<n;i++){
        m->tiles[i] = (uint8_t)t;
        m->walkable[i]    = (uint8_t)TILE_DEFS[t].walkable;
        m->transparent[i] = (uint8_t)TILE_DEFS[t].transparent;
    }
}

void map_set(Map *m, int x, int y, TileType t){
    if(!map_in_bounds(m, x, y)) return;
    size_t i = (size_t)y * m->w + x;
    m->tiles[i] = (uint8_t)t;
    m->walkable[i]    = (uint8_t)TILE_DEFS[t].walkable;
    m->transparent[i] = (uint8_t)TILE_DEFS[t].transparent;
}

TileType map_get(const Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return TILE_WALL;
    return (TileType)m->tiles[(size_t)y * m->w + x];
}

int map_in_bounds(const Map *m, int x, int y){
    return x>=0 && y>=0 && x<m->w && y<m->h;
}
int map_walkable(const Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return 0;
    return m->walkable[(size_t)y * m->w + x];
}
int map_transparent(const Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return 0;
    return m->transparent[(size_t)y * m->w + x];
}
int map_seen(const Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return 0;
    return m->seen[(size_t)y * m->w + x];
}
int map_visible(const Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return 0;
    return m->visible[(size_t)y * m->w + x];
}

void map_clear_visible(Map *m){
    size_t n = (size_t)m->w * m->h;
    memset(m->visible, 0, n);
}

void map_mark_visible(Map *m, int x, int y){
    if(!map_in_bounds(m, x, y)) return;
    size_t i = (size_t)y * m->w + x;
    m->visible[i] = 1;
    m->seen[i] = 1;   /* once seen, stays seen (fog of war) */
}
