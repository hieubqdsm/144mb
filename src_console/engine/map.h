/* =====================================================================
   MAP - Tile grid + world query (shared data cho FOV, path, render).
   Hoc tu libtcod: 1 struct Map chua tiles + walkable[] + transparent[]
   + seen[] + visible[]. FOV va pathfinding deu doc struct nay.
   ===================================================================== */
#ifndef CE_MAP_H
#define CE_MAP_H

#include <stdint.h>

/* Tile types */
typedef enum {
    TILE_WALL = 0,
    TILE_FLOOR,
    TILE_DOOR_OPEN,
    TILE_DOOR_CLOSED,
    TILE_STAIRS_DOWN,
    TILE_COUNT
} TileType;

/* Tile properties: walkable? block sight? glyph + color de render. */
typedef struct {
    int walkable;       /* co the di len khong */
    int transparent;   /* nhin xuyen duoc khong (wall = 0) */
    uint16_t glyph;     /* WCHAR de render */
    int fg;             /* foreground color (CE_Color) */
    int bg;             /* background color */
} TileDef;

/* Bang thuoc tinh tile (const, data-driven). */
extern const TileDef TILE_DEFS[TILE_COUNT];

/* Map grid. Map co the nho hon SCREEN; render voi offset (camera). */
typedef struct {
    int w, h;
    uint8_t *tiles;          /* w*h, moi cell = TileType */
    uint8_t *walkable;       /* w*h, cache walkable (tile + door state) */
    uint8_t *transparent;    /* w*h, cache transparent */
    uint8_t *seen;           /* w*h, da tung nhin thay (fog of war) */
    uint8_t *visible;        /* w*h, dang nhin thay (FOV hien tai) */
} Map;

/* Lifecycle */
Map *map_create(int w, int h);
void map_destroy(Map *m);

/* Fill toan bo map = tile type. */
void map_fill(Map *m, TileType t);

/* Dat/lay tile tai (x,y). Cap nhat walkable/transparent cache. */
void map_set(Map *m, int x, int y, TileType t);
TileType map_get(const Map *m, int x, int y);

/* Query (out-of-bound = wall/unwalkable/opaque). */
int map_walkable(const Map *m, int x, int y);
int map_transparent(const Map *m, int x, int y);
int map_in_bounds(const Map *m, int x, int y);
int map_seen(const Map *m, int x, int y);
int map_visible(const Map *m, int x, int y);

/* Reset FOV state (clear visible, giu seen). */
void map_clear_visible(Map *m);
/* Danh dau (x,y) da seen + visible (goi tu FOV algorithm). */
void map_mark_visible(Map *m, int x, int y);

/* Line of Sight: kiem tra (x0,y0) co nhin thay (x1,y1) khong.
   Dung Bresenham line, dung lai neu gap tile khong transparent.
   Tra ve 1 neu thay, 0 neu bi chan. */
int map_has_los(const Map *m, int x0, int y0, int x1, int y1);

#endif /* CE_MAP_H */
