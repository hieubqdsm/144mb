/* =====================================================================
   SAVE/LOAD - Serialize game state (player + inventory + depth).
   Format binary nho gon (khong phuc tap).
   ===================================================================== */
#ifndef CE_SAVE_H
#define CE_SAVE_H

#include "actor.h"
#include "inventory.h"

typedef struct {
    int hp, max_hp, ac;
    int str, dex, con, intl, wis, cha;
    int xp, level;
    int x, y;
    int depth;
    int n_potions;
    int has_weapon;
    int has_armor;
} SaveData;

/* Luu game state ra file. Tra ve 1 neu thanh cong. */
int save_game(const char *path, const SaveData *data);

/* Load game state tu file. Tra ve 1 neu thanh cong. */
int load_game(const char *path, SaveData *data);

/* Kiem tra save file co ton tai khong. */
int save_exists(const char *path);

#endif /* CE_SAVE_H */
