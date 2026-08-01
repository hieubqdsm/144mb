/* =====================================================================
   COMBAT - Implementation
   ===================================================================== */
#include "combat.h"
#include "actor.h"
#include "conditions.h"
#include "d20.h"
#include "../enums.h"
#include "../structs.h"
#include <stdio.h>

AtkResult combat_resolve_attack(const MonsterAction *action, int mod_to_hit,
                                int target_ac, RollMode mode, RNG *rng){
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
    } else if(roll.total >= target_ac){
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

int combat_concentration_check(Actor *caster, int damage, RNG *rng){
    /* 5e: DC = max(10, damage/2). Save = CON mod + d20 >= DC. */
    if(!actor_has_condition(caster, (Condition)COND_INCAPACITATED)){
        /* Concentration tracked via flag/condition - check neu dang concentrate */
    }
    int dc = damage / 2;
    if(dc < 10) dc = 10;
    int con = caster->type->scores[AB_CON];
    int mod = actor_ability_mod(con);
    D20Result r = d20_roll(mod, ROLL_NORMAL, rng);
    return r.total < dc;   /* 1 = mat concentrate */
}
