/* =====================================================================
   SAVE/LOAD - Serialize game state (player + inventory + depth).
   Format binary nho gon (khong phuc tap).
   ===================================================================== */
#ifndef CE_SAVE_H
#define CE_SAVE_H

#include "actor.h"
#include "inventory.h"
#include <stdint.h>

typedef struct {
    int hp, max_hp, ac;
    int str, dex, con, intl, wis, cha;
    int xp, level;
    int x, y;
    int depth;
    int n_potions;
    int has_weapon;
    int has_armor;
    /* v2 additions: class (Fighter/Mage) + RNG state (de continue deterministic) */
    int class_idx;          /* 0 = Fighter, 1 = Mage */
    uint64_t rng_state;     /* Xorshift64 state */
} SaveData;

/* Luu game state ra file. Tra ve 1 neu thanh cong. */
int save_game(const char *path, const SaveData *data);

/* Load game state tu file. Tra ve 1 neu thanh cong. */
int load_game(const char *path, SaveData *data);

/* Kiem tra save file co ton tai khong. */
int save_exists(const char *path);

/* ---------- Quick save/load (default path) ---------- */
#define SAVE_PATH "save.dat"
int save_quick(const SaveData *data);     /* wrapper save_game(SAVE_PATH, ..) */
int load_quick(SaveData *data);           /* wrapper load_game(SAVE_PATH, ..) */
int save_quick_exists(void);              /* wrapper save_exists(SAVE_PATH) */

#endif /* CE_SAVE_H */
