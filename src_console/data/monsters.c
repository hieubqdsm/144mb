/* =====================================================================
   DATA - Monster definitions (const compiled table, NetHack style).
   Them monster = them 1 entry. Khong can parser, 0 byte runtime cost.
   ===================================================================== */
#include "monsters.h"
#include "../game/ai.h"

/* --- Goblin attacks --- */
static const MonsterAction goblin_actions[] = {
    { /* [0] Scimitar (melee) */
        .name = "Scimitar",
        .atk_bonus = 4,
        .reach = 5, .range_max = 0,
        .damage = {1,6,2},        /* 1d6+2 (count, sides, mod) */
        .dmg_type = DMG_SLASHING,
    },
    { /* [1] Shortbow (ranged) */
        .name = "Shortbow",
        .atk_bonus = 4,
        .reach = 80, .range_max = 320,
        .damage = {1,6,2},
        .dmg_type = DMG_PIERCING,
    },
};

/* --- Player "type" (dung nhu template cho player instance) --- */
static const MonsterAction player_actions[] = {
    { /* [0] Longsword */
        .name = "Longsword",
        .atk_bonus = 5,           /* +2 prof, +3 STR */
        .reach = 5, .range_max = 0,
        .damage = {1,8,3},        /* 1d8+3 */
        .dmg_type = DMG_SLASHING,
    },
};

/* Monster table (const). */
const MonsterType MONSTERS[N_MONSTERS] = {  /* N_MONSTERS = #define = 2 */
    /* [ID_GOBLIN] */
    {
        .name = "Goblin",
        .size = 2,                  /* small */
        .type = 0,                  /* humanoid */
        .ac = 15,
        .hp_dice = {2,6,0},       /* 2d6 (avg 7) */
        .speed = 6,                 /* 30ft */
        .scores = { 8, 14, 10, 10, 8, 8 },  /* STR DEX CON INT WIS CHA */
        .cr = 1,                    /* CR 1/4 (x4) */
        .xp = 50,
        .glyph = 'g',
        .glyph_color = 12,          /* CE_RED */
        .actions = goblin_actions,
        .n_actions = 2,
        .ai = ai_melee_chaser,
    },
    /* [ID_PLAYER] */
    {
        .name = "Hero",
        .size = 3,                  /* medium */
        .type = 0,
        .ac = 16,                   /* chain shirt */
        .hp_dice = {1,10,3},      /* 1d10+3 (fighter) */
        .speed = 6,
        .scores = { 16, 12, 14, 10, 10, 12 },  /* STR 16 = +3 */
        .cr = 0,
        .xp = 0,
        .glyph = '@',
        .glyph_color = 10,          /* CE_GREEN */
        .actions = player_actions,
        .n_actions = 1,
        .ai = NULL,
    },
};
/* N_MONSTERS la #define trong monsters.h (= 2). */
