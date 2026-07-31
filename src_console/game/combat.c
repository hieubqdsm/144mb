/* =====================================================================
   COMBAT - Implementation
   ===================================================================== */
#include "combat.h"
#include "actor.h"
#include "d20.h"

AtkResult combat_resolve_attack(const MonsterAction *action, int mod_to_hit,
                                const Actor *target, RollMode mode, RNG *rng){
    AtkResult res = {0};
    /* 5e: nat 20 luon hit (crit), nat 1 luon miss */
    D20Result roll = d20_roll(mod_to_hit + action->atk_bonus, mode, rng);
    res.crit   = roll.nat20;
    res.fumble = roll.nat1;

    if(roll.nat1){
        res.hit = 0;
        return res;
    }
    if(roll.nat20){
        res.hit = 1;
    } else if(roll.total >= target->type->ac){
        res.hit = 1;
    } else {
        res.hit = 0;
        return res;
    }
    /* Hit -> roll damage */
    res.damage = d20_roll_damage(action->damage, roll.nat20, rng);
    return res;
}

void combat_apply_damage(Actor *target, int damage){
    actor_take_damage(target, damage);
}
