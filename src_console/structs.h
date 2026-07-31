/* =====================================================================
   STRUCTS - Shared struct definitions cho D&D-lite.
   Hoc tu NetHack permonst/monst: TYPE (static, shared) vs INSTANCE (dynamic).
   ===================================================================== */
#ifndef CE_STRUCTS_H
#define CE_STRUCTS_H

#include <stdint.h>
#include "enums.h"
#include "engine/rng.h"

/* Forward declarations (tranh circular include).
   Map duoc typedef day du trong engine/map.h; o day dung struct tag. */
typedef struct Actor Actor;
typedef struct MonsterAction MonsterAction;
typedef struct MonsterType MonsterType;
typedef struct Battle Battle;
struct Map;

/* ---------- Dice formula ("2d6+2") ---------- */
typedef struct { int8_t count; uint8_t sides; int8_t mod; } DiceFormula;
/* Helper macro: DC(2,6,2) = 2d6+2 */
#define DC(c,s,m) ((DiceFormula){(c),(s),(m)})

/* ---------- Monster attack/action (TYPE-level, shared) ---------- */
struct MonsterAction {
    const char *name;        /* "Scimitar", "Shortbow" */
    int8_t atk_bonus;        /* +4 */
    uint8_t reach;           /* 5 = melee, hoac range ft */
    uint8_t range_max;       /* 0 = melee */
    DiceFormula damage;      /* DC(1,6,2) */
    DamageType dmg_type;
};

/* ---------- Monster TYPE (static definition, shared giua instances) ---------- */
struct MonsterType {
    const char *name;        /* "Goblin" */
    uint8_t size;            /* enum: tiny..gargantuan (don gian 1-6) */
    uint8_t type;            /* humanoid/beast/dragon... (index) */
    uint8_t ac;              /* Armor Class */
    DiceFormula hp_dice;     /* DC(2,6) -> 2d6 HP */
    uint8_t speed;           /* squares/turn (30ft = 6 squares) */
    uint8_t scores[AB_COUNT];/* STR/DEX/CON/INT/WIS/CHA */
    uint8_t cr;              /* challenge rating x4 (0.25 -> 1) */
    uint16_t xp;
    uint16_t glyph;          /* WCHAR de render ('g', 'O', 'D') */
    int glyph_color;         /* CE_Color */
    const MonsterAction *actions;
    uint8_t n_actions;
    /* AI hook: monster chon hanh dong gi trong turn.
       self = monster dang act, all = tat ca actors, n = count, log = log callback. */
    void (*ai)(Actor *self, Actor *all, int n, void (*log)(const char*));
};

/* ---------- Actor INSTANCE (dynamic, per-entity) ---------- */
struct Actor {
    const MonsterType *type;   /* ptr to shared definition */
    uint16_t hp, max_hp;
    int8_t x, y;               /* grid position (-1 = off-grid) */
    Team team;
    uint64_t conditions;       /* bitmask Condition */
    uint32_t flags;            /* bitmask EntityFlags */
    /* Action economy (refresh moi turn) */
    uint8_t move_left;         /* squares con lai trong turn */
    uint8_t action_used  : 1;
    uint8_t bonus_used   : 1;
    uint8_t reaction_used: 1;
    /* Initiative / turn order */
    int8_t initiative;         /* d20 + DEX mod */
    /* XP/level (player only, 0 cho monster) */
    uint16_t xp;
    uint8_t level;
};

/* ---------- Battle (combat state) ---------- */
typedef struct {
    struct Map *map;           /* map cua combat (grid) */
    Actor **order;             /* initiative-sorted actors */
    uint16_t order_count;
    uint16_t turn_idx;         /* current position in order[] */
    uint16_t round;
    uint8_t in_combat;
    Actor *actors;             /* array of all actors (caller owns) */
    uint16_t n_actors;
} BattleData;

struct Battle {
    BattleData data;
};

#endif /* CE_STRUCTS_H */
