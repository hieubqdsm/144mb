/* =====================================================================
   COMBAT - Attack resolution (D&D 5e style).
   resolve_attack: roll to-hit vs target AC, neu hit thi roll damage.
   Tra ve AtkResult (hit?, damage, crit?, fumble?).
   ===================================================================== */
#ifndef CE_COMBAT_H
#define CE_COMBAT_H

#include "../structs.h"
#include "../enums.h"
#include "../engine/rng.h"

typedef struct {
    int hit;        /* 1 = trung, 0 = hut */
    int damage;     /* total damage (0 neu hut) */
    uint8_t crit    : 1;
    uint8_t fumble  : 1;
} AtkResult;

/* Source attack target bang MonsterAction.
   mod_to_hit = bonus cuong击 (STR/DEX mod + proficiency).
   advantage = advantage on attack roll. */
AtkResult combat_resolve_attack(const MonsterAction *action, int mod_to_hit,
                                const Actor *target, RollMode mode, RNG *rng);

/* Ap dung damage vao target (giam HP, set DEAD). */
void combat_apply_damage(Actor *target, int damage);

#endif /* CE_COMBAT_H */
