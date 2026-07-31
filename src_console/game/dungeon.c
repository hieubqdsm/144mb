/* =====================================================================
   DUNGEON - Implementation
   ===================================================================== */
#include "dungeon.h"
#include "actor.h"
#include "../engine/bsp.h"
#include "../game/items.h"
#include "../data/monsters.h"   /* MONSTERS[] table */
#include <string.h>

void dungeon_generate(Floor *f, int depth, RNG *rng){
    if(!f) return;
    if(!f->map) f->map = map_create(60, 32);
    Room rooms[16];
    int n = bsp_generate(f->map, rng, 6 + depth, rooms);

    f->depth = depth;
    f->n_monsters = 0;
    f->n_floor_items = 0;

    if(n <= 0) return;

    /* Player spawn o center phong dau */
    f->player_start_x = rooms[0].cx;
    f->player_start_y = rooms[0].cy;

    /* Stairs xuong o center phong cuoi */
    f->stairs_x = rooms[n-1].cx;
    f->stairs_y = rooms[n-1].cy;
    map_set(f->map, f->stairs_x, f->stairs_y, TILE_STAIRS_DOWN);

    /* Monster spawn o phong giua (room 1..n-2) */
    for(int i = 1; i < n - 1 && f->n_monsters < 16; i++){
        int nmon = rng_range(rng, 0, 1 + depth/2);
        for(int m = 0; m < nmon && f->n_monsters < 16; m++){
            int mx = rooms[i].x + rng_range(rng, 0, rooms[i].w - 1);
            int my = rooms[i].y + rng_range(rng, 0, rooms[i].h - 1);
            /* Monster kho难度 tăng theo depth */
            int mid = (depth >= 2 && rng_chance(rng, 0.3f)) ? ID_GOBLIN : ID_GOBLIN;
            (void)mid;
            f->monsters[f->n_monsters] = actor_spawn(&MONSTERS[ID_GOBLIN], mx, my, TEAM_ENEMY, rng);
            f->n_monsters++;
        }
    }

    /* Item nam san (potion) o phong random */
    int nitems = rng_range(rng, 1, 3);
    for(int i = 0; i < nitems && f->n_floor_items < 32; i++){
        int r = rng_range(rng, 1, n - 1);
        int ix = rooms[r].x + rng_range(rng, 0, rooms[r].w - 1);
        int iy = rooms[r].y + rng_range(rng, 0, rooms[r].h - 1);
        f->floor_items[f->n_floor_items].x = ix;
        f->floor_items[f->n_floor_items].y = iy;
        f->floor_items[f->n_floor_items].type = &ITEMS[ID_HEAL_POTION];
        f->floor_items[f->n_floor_items].qty = 1;
        f->n_floor_items++;
    }
}

void dungeon_free(Floor *f){
    if(!f) return;
    if(f->map){ map_destroy(f->map); f->map = NULL; }
}
