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

    /* Monster spawn theo depth (kho难度 tăng).
       Depth 1-2: goblin/skeleton/wolf. 3-4: + orc. 5+: + ogre. 7+: dragon boss. */
    for(int i = 1; i < n - 1 && f->n_monsters < 16; i++){
        int nmon = rng_range(rng, 0, 2 + depth/2);
        for(int m = 0; m < nmon && f->n_monsters < 16; m++){
            int mx = rooms[i].x + rng_range(rng, 0, rooms[i].w - 1);
            int my = rooms[i].y + rng_range(rng, 0, rooms[i].h - 1);
            int mid;
            if(depth >= 7 && i == n-2 && rng_chance(rng, 0.5f)) mid = ID_DRAGON;
            else if(depth >= 5 && rng_chance(rng, 0.3f)) mid = ID_OGRE;
            else if(depth >= 3 && rng_chance(rng, 0.4f)) mid = ID_ORC;
            else {
                int roll = rng_range(rng, 0, 3);
                if(roll==0) mid = ID_GOBLIN;
                else if(roll==1) mid = ID_SKELETON;
                else if(roll==2) mid = ID_WOLF;
                else mid = (depth >= 3) ? ID_ZOMBIE : ID_GOBLIN;  /* zombie depth 3+ */
            }
            f->monsters[f->n_monsters] = actor_spawn(&MONSTERS[mid], mx, my, TEAM_ENEMY, rng);
            f->n_monsters++;
        }
    }

    /* Item nam san: potion + rarely weapon/armor. Cang sau cang nhieu loot. */
    int nitems = rng_range(rng, 1, 2 + depth/2);
    for(int i = 0; i < nitems && f->n_floor_items < 32; i++){
        int r = rng_range(rng, 1, n - 1);
        int ix = rooms[r].x + rng_range(rng, 0, rooms[r].w - 1);
        int iy = rooms[r].y + rng_range(rng, 0, rooms[r].h - 1);
        const ItemType *itype = &ITEMS[ID_HEAL_POTION];
        int roll = rng_range(rng, 1, 10);
        if(roll <= 6) itype = &ITEMS[ID_HEAL_POTION];
        else if(roll <= 8) itype = &ITEMS[ID_DAGGER];
        else itype = &ITEMS[ID_CHAINSHIRT];
        f->floor_items[f->n_floor_items].x = ix;
        f->floor_items[f->n_floor_items].y = iy;
        f->floor_items[f->n_floor_items].type = itype;
        f->floor_items[f->n_floor_items].qty = 1;
        f->n_floor_items++;
    }
}

void dungeon_free(Floor *f){
    if(!f) return;
    if(f->map){ map_destroy(f->map); f->map = NULL; }
}
