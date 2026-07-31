/* =====================================================================
   DUNGEON - Multi-floor dungeon generation (BSP rooms + monster/item spawn).
   ===================================================================== */
#ifndef CE_DUNGEON_H
#define CE_DUNGEON_H

#include "../engine/map.h"
#include "../engine/rng.h"
#include "../structs.h"
#include "items.h"

/* Trang thai 1 floor. */
typedef struct {
    Map *map;
    int depth;              /* tang thu may (1, 2, 3...) */
    int stairs_x, stairs_y;
    int player_start_x, player_start_y;
    Actor monsters[16];
    int n_monsters;
    /* Items nam san */
    struct { int x, y; const ItemType *type; int qty; } floor_items[32];
    int n_floor_items;
} Floor;

/* Tao floor moi (depth). Player spawn o phong dau, stairs o phong cuoi. */
void dungeon_generate(Floor *f, int depth, RNG *rng);

/* Free floor resources. */
void dungeon_free(Floor *f);

#endif /* CE_DUNGEON_H */
