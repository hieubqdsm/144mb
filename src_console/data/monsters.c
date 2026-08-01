/* =====================================================================
   DATA - Monster definitions (const compiled table, NetHack style).
   Them monster = them 1 entry. Khong can parser, 0 byte runtime cost.
   Stats lay tu D&D 5e SRD (goblin, skeleton, wolf, orc, ogre, dragon).
   ===================================================================== */
#include "monsters.h"
#include "../game/ai.h"

/* --- Goblin attacks --- */
static const MonsterAction goblin_actions[] = {
    { "Scimitar", 4, 5,0,  {1,6,2}, DMG_SLASHING },
    { "Shortbow", 4, 80,320, {1,6,2}, DMG_PIERCING },
};

/* --- Player attacks --- */
static const MonsterAction player_actions[] = {
    { "Longsword", 5, 5,0, {1,8,3}, DMG_SLASHING },
    { "Fire Bolt", 5, 120,0, {1,10,0}, DMG_FIRE },  /* cantrip backup */
};

/* --- Skeleton attacks (ranged archer) --- */
static const MonsterAction skeleton_actions[] = {
    { "Shortsword", 4, 5,0, {1,6,2}, DMG_PIERCING },
    { "Shortbow", 4, 80,320, {1,6,2}, DMG_PIERCING },
};

/* --- Wolf (fast melee, pack hunter) --- */
static const MonsterAction wolf_actions[] = {
    { "Bite", 4, 5,0, {1,6,2}, DMG_PIERCING },
};

/* --- Orc (brute melee) --- */
static const MonsterAction orc_actions[] = {
    { "Greataxe", 5, 5,0, {1,12,3}, DMG_SLASHING },  /* 1d12+3 - nguy hiem! */
    { "Javelin", 5, 30,120, {1,6,3}, DMG_PIERCING },
};

/* --- Ogre (tanky heavy hitter) --- */
static const MonsterAction ogre_actions[] = {
    { "Greatclub", 6, 5,0, {2,8,4}, DMG_BLUDGEONING },  /* 2d8+4 */
    { "Javelin", 6, 30,120, {1,10,4}, DMG_PIERCING },
};

/* --- Dragon (boss, multiattack) --- */
static const MonsterAction dragon_actions[] = {
    { "Bite", 8, 5,0, {2,10,6}, DMG_PIERCING },
    { "Fire Breath", 8, 0,0, {8,6,0}, DMG_FIRE },  /* cone, treated ranged */
};

/* --- Zombie (slow tanky undead) --- */
static const MonsterAction zombie_actions[] = {
    { "Slam", 3, 5,0, {1,6,1}, DMG_BLUDGEONING },
};

/* Monster table (const). */
const MonsterType MONSTERS[N_MONSTERS] = {  /* N_MONSTERS = #define = 7 */
    /* [ID_GOBLIN] - CR 1/4 */
    {
        .name = "Goblin", .size = 2, .type = 0, .ac = 15,
        .hp_dice = {2,6,0}, .speed = 6,
        .scores = { 8, 14, 10, 10, 8, 8 },
        .cr = 1, .xp = 50, .glyph = 'g', .glyph_color = 12,
        .actions = goblin_actions, .n_actions = 2,
        .ai = ai_melee_chaser,
    },
    /* [ID_PLAYER] */
    {
        .name = "Hero", .size = 3, .type = 0, .ac = 16,
        .hp_dice = {1,10,3}, .speed = 6,
        .scores = { 16, 12, 14, 10, 10, 12 },
        .cr = 0, .xp = 0, .glyph = '@', .glyph_color = 10,
        .actions = player_actions, .n_actions = 2,
        .ai = NULL,
    },
    /* [ID_SKELETON] - CR 1/4, ranged archer */
    {
        .name = "Skeleton", .size = 3, .type = 1, .ac = 13,
        .hp_dice = {2,8,2}, .speed = 6,
        .scores = { 10, 14, 15, 6, 8, 5 },
        .cr = 1, .xp = 50, .glyph = 's', .glyph_color = 7,
        .actions = skeleton_actions, .n_actions = 2,
        .ai = ai_ranged,
    },
    /* [ID_WOLF] - CR 1/4, fast melee */
    {
        .name = "Wolf", .size = 2, .type = 2, .ac = 13,
        .hp_dice = {2,8,2}, .speed = 8,   /* nhanh hon */
        .scores = { 12, 15, 12, 3, 12, 6 },
        .cr = 1, .xp = 50, .glyph = 'w', .glyph_color = 8,
        .actions = wolf_actions, .n_actions = 1,
        .ai = ai_melee_chaser,
    },
    /* [ID_ORC] - CR 1/2, brute */
    {
        .name = "Orc", .size = 3, .type = 0, .ac = 13,
        .hp_dice = {2,8,6}, .speed = 6,
        .scores = { 16, 12, 16, 7, 11, 10 },
        .cr = 2, .xp = 100, .glyph = 'o', .glyph_color = 12,
        .actions = orc_actions, .n_actions = 2,
        .ai = ai_melee_chaser,
    },
    /* [ID_OGRE] - CR 2, tanky heavy */
    {
        .name = "Ogre", .size = 4, .type = 3, .ac = 11,
        .hp_dice = {7,10,7}, .speed = 6,
        .scores = { 19, 8, 16, 5, 7, 7 },
        .cr = 8, .xp = 450, .glyph = 'O', .glyph_color = 13,
        .actions = ogre_actions, .n_actions = 2,
        .ai = ai_melee_chaser,
    },
    /* [ID_DRAGON] - boss, CR 5-ish */
    {
        .name = "Dragon", .size = 6, .type = 4, .ac = 17,
        .hp_dice = {12,10,36}, .speed = 6,
        .scores = { 23, 12, 19, 14, 13, 15 },
        .cr = 20, .xp = 1800, .glyph = 'D', .glyph_color = 12,
        .actions = dragon_actions, .n_actions = 2,
        .ai = ai_boss,
    },
    /* [ID_ZOMBIE] - CR 1/4, slow undead tank */
    {
        .name = "Zombie", .size = 3, .type = 1, .ac = 8,
        .hp_dice = {3,8,9}, .speed = 4,   /* cham (20ft), HP cao (3d8+9 ~ 22) */
        .scores = { 13, 6, 16, 3, 6, 5 },
        .cr = 1, .xp = 50, .glyph = 'z', .glyph_color = 2,  /* xanh la (undead) */
        .actions = zombie_actions, .n_actions = 1,
        .ai = ai_melee_chaser,
    },
};
/* N_MONSTERS la #define trong monsters.h (= 8). */
