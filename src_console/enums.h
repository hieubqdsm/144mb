/* =====================================================================
   ENUMS - Shared enums cho game (D&D-lite).
   Hoc tu NetHack: 1 file enums de de tim.
   ===================================================================== */
#ifndef CE_ENUMS_H
#define CE_ENUMS_H

#include <stdint.h>

/* ---------- Ability scores (6 thong so D&D) ---------- */
typedef enum { AB_STR=0, AB_DEX, AB_CON, AB_INT, AB_WIS, AB_CHA, AB_COUNT } Ability;
#define ABILITY_NAME(a) ((const char*[]){"STR","DEX","CON","INT","WIS","CHA"}[a])

/* ---------- Damage types ---------- */
typedef enum {
    DMG_SLASHING=0, DMG_PIERCING, DMG_BLUDGEONING,
    DMG_FIRE, DMG_COLD, DMG_LIGHTNING, DMG_ACID, DMG_POISON,
    DMG_PSYCHIC,
    DMG_FORCE, DMG_NECROTIC, DMG_RADIANT,
    DMG_COUNT
} DamageType;

/* ---------- Conditions (bitmask, 15 cai nhu D&D 5e) ---------- */
typedef enum {
    COND_NONE = 0,
    COND_BLINDED     = 1 << 0,
    COND_CHARMED      = 1 << 1,
    COND_DEAFENED     = 1 << 2,
    COND_FRIGHTENED   = 1 << 3,
    COND_GRAPPLED     = 1 << 4,
    COND_INCAPACITATED= 1 << 5,
    COND_INVISIBLE    = 1 << 6,
    COND_PARALYZED    = 1 << 7,
    COND_PETRIFIED    = 1 << 8,
    COND_POISONED     = 1 << 9,
    COND_PRONE        = 1 << 10,
    COND_RESTRAINED   = 1 << 11,
    COND_STUNNED      = 1 << 12,
    COND_UNCONSCIOUS  = 1 << 13,
    COND_EXHAUSTION   = 1 << 14,
} Condition;
#define COND_HAS(c, mask) (((c) & (mask)) != 0)

/* ---------- Team (player vs enemy vs neutral) ---------- */
typedef enum { TEAM_PLAYER=0, TEAM_ENEMY=1, TEAM_NEUTRAL=2 } Team;

/* ---------- Entity flags ---------- */
typedef enum {
    EF_DEAD        = 1 << 0,
    EF_PLAYER      = 1 << 1,
    EF_NPC         = 1 << 2,
    EF_UNCONSCIOUS = 1 << 3,
} EntityFlags;

/* ---------- Save types (D&D saving throws) ---------- */
typedef enum {
    SAVE_NONE=0, SAVE_STR, SAVE_DEX, SAVE_CON, SAVE_INT, SAVE_WIS, SAVE_CHA
} SaveType;

/* ---------- Roll modes ---------- */
typedef enum { ROLL_NORMAL=0, ROLL_ADVANTAGE, ROLL_DISADVANTAGE } RollMode;

#endif /* CE_ENUMS_H */
