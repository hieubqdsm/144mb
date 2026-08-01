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
   target_ac = AC thuc te cua target (base + armor + shield + condition).
   advantage = advantage on attack roll. */
AtkResult combat_resolve_attack(const MonsterAction *action, int mod_to_hit,
                                int target_ac, RollMode mode, RNG *rng);

/* Ap dung damage vao target (giam HP, set DEAD). */
void combat_apply_damage(Actor *target, int damage);

/* Concentration check: khi actor dang concentrate mau damage, phai save
   CON DC = max(10, dmg/2). Tra ve 1 neu mat concentration. */
int combat_concentration_check(Actor *caster, int damage, RNG *rng);

#endif /* CE_COMBAT_H */
