/* =====================================================================
   DATA - Monster definitions (const compiled table, NetHack style).
   =====================================================================
   CACH DOC (sau refactor - tu giai thich):
     .hp_dice   = DICE(2,6,0)      <- "2d6+0"  (count, sides, mod)
     .scores    = STATS(STR(8), DEX(14), CON(10), INT(10), WIS(8), CHA(8))
     .cr        = CR(0.25)         <- Challenge Rating (luu x4 internal)
     .size      = SIZE_SMALL       <- thay so 2
     .type      = TYPE_HUMANOID    <- thay so 0
     .glyph_color = CE_RED         <- thay so 12

   THEM/SUA MONSTER = edit 1 entry nay. Khong can parser, 0 byte file.
   Stats lay tu D&D 5e SRD.
   ===================================================================== */
#include "monsters.h"
#include "../structs.h"   /* DICE, STATS, CR, SIZE_*, TYPE_*, enum macros */
#include "../engine/console.h"  /* CE_RED, CE_GREEN, ... */
#include "../game/ai.h"

/* =====================================================================
   ATTACKS (mang const, shared giua instances cung loai)
   Dung designated init de doc ro.
   ===================================================================== */

/* --- Goblin: Scimitar (melee) + Shortbow (ranged) --- */
static const MonsterAction goblin_actions[] = {
    { .name="Scimitar", .atk_bonus=4, .reach=5,  .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_SLASHING },
    { .name="Shortbow", .atk_bonus=4, .reach=80, .range_max=320,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Player: Longsword (melee) + Fire Bolt (cantrip ranged) --- */
static const MonsterAction player_actions[] = {
    { .name="Longsword", .atk_bonus=5, .reach=5,   .range_max=0,
      .damage=DICE(1,8,3), .dmg_type=DMG_SLASHING },
    { .name="Fire Bolt", .atk_bonus=5, .reach=120, .range_max=0,
      .damage=DICE(1,10,0), .dmg_type=DMG_FIRE },
};

/* --- Skeleton: Shortsword (melee) + Shortbow (ranged archer) --- */
static const MonsterAction skeleton_actions[] = {
    { .name="Shortsword", .atk_bonus=4, .reach=5,  .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
    { .name="Shortbow",   .atk_bonus=4, .reach=80, .range_max=320,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Wolf: Bite (fast melee, pack hunter) --- */
static const MonsterAction wolf_actions[] = {
    { .name="Bite", .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Orc: Greataxe (1d12+3 nguy hiem!) + Javelin (ranged) --- */
static const MonsterAction orc_actions[] = {
    { .name="Greataxe", .atk_bonus=5, .reach=5,  .range_max=0,
      .damage=DICE(1,12,3), .dmg_type=DMG_SLASHING },
    { .name="Javelin",  .atk_bonus=5, .reach=30, .range_max=120,
      .damage=DICE(1,6,3), .dmg_type=DMG_PIERCING },
};

/* --- Ogre: Greatclub (2d8+4 tanky) + Javelin --- */
static const MonsterAction ogre_actions[] = {
    { .name="Greatclub", .atk_bonus=6, .reach=5,  .range_max=0,
      .damage=DICE(2,8,4), .dmg_type=DMG_BLUDGEONING },
    { .name="Javelin",   .atk_bonus=6, .reach=30, .range_max=120,
      .damage=DICE(1,10,4), .dmg_type=DMG_PIERCING },
};

/* --- Dragon (boss): Bite + Fire Breath (multiattack 2/turn) --- */
static const MonsterAction dragon_actions[] = {
    { .name="Bite",        .atk_bonus=8, .reach=5, .range_max=0,
      .damage=DICE(2,10,6), .dmg_type=DMG_PIERCING },
    { .name="Fire Breath", .atk_bonus=8, .reach=0, .range_max=0,
      .damage=DICE(8,6,0), .dmg_type=DMG_FIRE },
};

/* --- Zombie: Slam (slow, tanky undead) --- */
static const MonsterAction zombie_actions[] = {
    { .name="Slam", .atk_bonus=3, .reach=5, .range_max=0,
      .damage=DICE(1,6,1), .dmg_type=DMG_BLUDGEONING },
};

/* =====================================================================
   MONSTER TABLE - moi entry = 1 monster type.
   Design flow: copy 1 entry, doi ten/stats. Them ID o monsters.h.
   ===================================================================== */
const MonsterType MONSTERS[N_MONSTERS] = {

    /* [ID_GOBLIN] - CR 1/4, melee chaser -------------------- */
    [ID_GOBLIN] = {
        .name       = "Goblin",
        .size       = SIZE_SMALL,
        .type       = TYPE_HUMANOID,
        .ac         = 15,
        .hp_dice    = DICE(2, 6, 0),                  /* 2d6 (avg 7) */
        .speed      = 6,                               /* 30 ft/turn */
        .scores     = STATS(STR(8),  DEX(14), CON(10),
                            INT(10), WIS(8),  CHA(8)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'g',
        .glyph_color= CE_RED,
        .actions    = goblin_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_PLAYER] - Hero template --------------------------- */
    [ID_PLAYER] = {
        .name       = "Hero",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 16,
        .hp_dice    = DICE(1, 10, 3),                  /* 1d10+3 (fighter) */
        .speed      = 6,
        .scores     = STATS(STR(16), DEX(12), CON(14),
                            INT(10), WIS(10), CHA(12)),
        .cr         = CR(0),
        .xp         = 0,
        .glyph      = '@',
        .glyph_color= CE_GREEN,
        .actions    = player_actions,
        .n_actions  = 2,
        .ai         = NULL,
    },

    /* [ID_SKELETON] - CR 1/4, ranged archer ----------------- */
    [ID_SKELETON] = {
        .name       = "Skeleton",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_UNDEAD,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 2),                   /* 2d8+2 (avg 11) */
        .speed      = 6,
        .scores     = STATS(STR(10), DEX(14), CON(15),
                            INT(6),  WIS(8),  CHA(5)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 's',
        .glyph_color= CE_GREY,
        .actions    = skeleton_actions,
        .n_actions  = 2,
        .ai         = ai_ranged,
    },

    /* [ID_WOLF] - CR 1/4, fast melee ------------------------ */
    [ID_WOLF] = {
        .name       = "Wolf",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_BEAST,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 2),                   /* 2d8+2 (avg 11) */
        .speed      = 8,                               /* 40 ft - nhanh! */
        .scores     = STATS(STR(12), DEX(15), CON(12),
                            INT(3),  WIS(12), CHA(6)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'w',
        .glyph_color= CE_DGREY,
        .actions    = wolf_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },

    /* [ID_ORC] - CR 1/2, brute melee ------------------------ */
    [ID_ORC] = {
        .name       = "Orc",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 6),                   /* 2d8+6 (avg 15) */
        .speed      = 6,
        .scores     = STATS(STR(16), DEX(12), CON(16),
                            INT(7),  WIS(11), CHA(10)),
        .cr         = CR(0.5),
        .xp         = 100,
        .glyph      = 'o',
        .glyph_color= CE_RED,
        .actions    = orc_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_OGRE] - CR 2, tanky heavy hitter ------------------ */
    [ID_OGRE] = {
        .name       = "Ogre",
        .size       = SIZE_LARGE,
        .type       = TYPE_GIANT,
        .ac         = 11,
        .hp_dice    = DICE(7, 10, 7),                  /* 7d10+7 (avg 45) */
        .speed      = 6,
        .scores     = STATS(STR(19), DEX(8),  CON(16),
                            INT(5),  WIS(7),  CHA(7)),
        .cr         = CR(2),
        .xp         = 450,
        .glyph      = 'O',
        .glyph_color= CE_MAG,
        .actions    = ogre_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_DRAGON] - CR 5, boss (multiattack) ---------------- */
    [ID_DRAGON] = {
        .name       = "Dragon",
        .size       = SIZE_HUGE,
        .type       = TYPE_DRAGON,
        .ac         = 17,
        .hp_dice    = DICE(12, 10, 36),                /* 12d10+36 (avg 102) */
        .speed      = 6,
        .scores     = STATS(STR(23), DEX(12), CON(19),
                            INT(14), WIS(13), CHA(15)),
        .cr         = CR(5),
        .xp         = 1800,
        .glyph      = 'D',
        .glyph_color= CE_RED,
        .actions    = dragon_actions,
        .n_actions  = 2,
        .ai         = ai_boss,
    },

    /* [ID_ZOMBIE] - CR 1/4, slow undead tank ---------------- */
    [ID_ZOMBIE] = {
        .name       = "Zombie",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_UNDEAD,
        .ac         = 8,
        .hp_dice    = DICE(3, 8, 9),                   /* 3d8+9 (avg 22) */
        .speed      = 4,                               /* 20 ft - cham */
        .scores     = STATS(STR(13), DEX(6),  CON(16),
                            INT(3),  WIS(6),  CHA(5)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'z',
        .glyph_color= CE_DGREEN,                       /* xanh la (undead) */
        .actions    = zombie_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },
};
