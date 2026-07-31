/* =====================================================================
   CONDITIONS - Timed effects resolution (DOT, buffs, save-ends).
   Effect = intrusive linked list tren Actor. Resolve dau turn (tick down).
   ===================================================================== */
#ifndef CE_CONDITIONS_H
#define CE_CONDITIONS_H

#include "../structs.h"
#include "../enums.h"
#include "../engine/rng.h"

typedef struct Effect {
    uint16_t id;              /* spell/feature id de hien ten */
    Condition condition;      /* COND_* apply, hoac 0 cho pure-stat */
    int8_t rounds_left;       /* -1 = permanent, 0 = expired */
    Ability save_stat;        /* SAVE_NONE = no save */
    int8_t save_dc;           /* DC cua save */
    int8_t dmg_per_turn;      /* DOT damage (poison), 0 = none */
    DamageType dmg_type;
    int8_t atk_bonus, ac_bonus;  /* flat modifiers while active */
    struct Effect *next;
} Effect;

/* Them effect vao actor (push front). rounds=duration. */
void cond_add(Actor *a, Effect e);

/* Resolve tat ca effects dau turn cua actor (tick, dmg, save, expire).
   log_buf: nhan thong bao (vd "take 4 poison damage"). Tra ve so effects expired. */
int cond_resolve_turn(Actor *a, RNG *rng, char *log_buf, int log_buf_size);

/* Dem so effect dang active. */
int cond_count(const Actor *a);

/* Bonus AC tu effects (mage armor, shield, etc.). */
int cond_ac_bonus(const Actor *a);
int cond_atk_bonus(const Actor *a);

/* Clear all effects (death / rest). */
void cond_clear_all(Actor *a);

#endif /* CE_CONDITIONS_H */
