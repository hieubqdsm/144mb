/* =====================================================================
   SPELL RESOLVE - Implementation
   ===================================================================== */
#include "spell_resolve.h"
#include "d20.h"
#include "combat.h"
#include <stdio.h>
#include <string.h>

int spell_cast(int spell_id, Actor *caster, Actor *target,
               int mod_spell, RNG *rng, char *log_buf, int log_size){
    if(spell_id < 0 || spell_id >= N_SPELLS){ return 0; }
    const SpellDef *s = &SPELLS[spell_id];
    log_buf[0] = 0;

    switch(s->kind){
        case SP_ATK_RANGED: {
            D20Result r = d20_roll(mod_spell, ROLL_NORMAL, rng);
            if(r.nat20){
                int dmg = d20_roll_damage(s->damage, 1, rng);
                combat_apply_damage(target, dmg);
                snprintf(log_buf, log_size, "%s CRIT %s voi %s: %d damage!",
                         caster->type->name, target->type->name, s->name, dmg);
            } else if(r.total >= target->type->ac){
                int dmg = d20_roll_damage(s->damage, 0, rng);
                combat_apply_damage(target, dmg);
                snprintf(log_buf, log_size, "%s ban %s vao %s: %d damage.",
                         caster->type->name, s->name, target->type->name, dmg);
            } else {
                snprintf(log_buf, log_size, "%s ban %s: hut (AC %d).",
                         caster->type->name, s->name, target->type->ac);
                return 0;
            }
            break;
        }
        case SP_MAGIC_MISSILE: {
            /* Auto-hit, no save */
            int dmg = d20_roll_damage(s->damage, 0, rng);
            combat_apply_damage(target, dmg);
            snprintf(log_buf, log_size, "%s ban %s: %d damage (auto-hit).",
                     caster->type->name, s->name, dmg);
            break;
        }
        case SP_SAVE_HALF: {
            int dmg = d20_roll_damage(s->damage, 0, rng);
            if(s->save != SAVE_NONE){
                int score = target->type->scores[s->save - SAVE_STR];
                int mod = actor_ability_mod(score);
                D20Result r = d20_roll(mod, ROLL_NORMAL, rng);
                if(r.total >= 12){   /* DC 12 default */
                    dmg /= 2;
                    snprintf(log_buf, log_size, "%s ban %s: %s save, %d damage (half).",
                             caster->type->name, s->name, target->type->name, dmg);
                } else {
                    snprintf(log_buf, log_size, "%s ban %s: %s %d damage day!",
                             caster->type->name, s->name, target->type->name, dmg);
                }
            } else {
                snprintf(log_buf, log_size, "%s ban %s: %d damage.",
                         caster->type->name, s->name, dmg);
            }
            combat_apply_damage(target, dmg);
            break;
        }
        case SP_BUFF_AC: {
            Effect e = {0};
            e.id = spell_id + 100;  /* distinguish from monster */
            e.rounds_left = 60;     /* ~1 hour in rounds (game) */
            e.ac_bonus = (int8_t)s->ac_bonus;
            cond_add(caster, e);
            snprintf(log_buf, log_size, "%s cast %s: +%d AC.",
                     caster->type->name, s->name, s->ac_bonus);
            break;
        }
        case SP_HEAL: {
            int heal = d20_roll_damage(s->damage, 0, rng);
            actor_heal(target, heal);
            snprintf(log_buf, log_size, "%s cast %s len %s: +%d HP (%d/%d).",
                     caster->type->name, s->name, target->type->name, heal,
                     target->hp, target->max_hp);
            break;
        }
        case SP_POISON: {
            /* Save CON neu fail -> poison DOT + initial damage */
            int dmg = d20_roll_damage(s->damage, 0, rng);
            if(s->save != SAVE_NONE){
                int score = target->type->scores[s->save - SAVE_STR];
                int mod = actor_ability_mod(score);
                D20Result r = d20_roll(mod, ROLL_NORMAL, rng);
                if(r.total >= 13){   /* DC 13 */
                    snprintf(log_buf, log_size, "%s cast %s: %s save (DC13), khong bi poison.",
                             caster->type->name, s->name, target->type->name);
                    return 1;
                }
            }
            combat_apply_damage(target, dmg);
            /* Them poison DOT effect (1d4/turn, 10 rounds) */
            Effect pe = {0};
            pe.id = spell_id + 100;
            pe.condition = COND_POISONED;
            pe.rounds_left = 10;
            pe.dmg_per_turn = 3;   /* ~1d4 poison/turn */
            pe.dmg_type = DMG_POISON;
            cond_add(target, pe);
            snprintf(log_buf, log_size, "%s cast %s: %s bi POISONED (%d dmg + DOT 10 rounds).",
                     caster->type->name, s->name, target->type->name, dmg);
            break;
        }
        case SP_STUN: {
            /* Save neu fail -> stunned (mất turn) */
            if(s->save != SAVE_NONE){
                int score = target->type->scores[s->save - SAVE_STR];
                int mod = actor_ability_mod(score);
                D20Result r = d20_roll(mod, ROLL_NORMAL, rng);
                if(r.total >= 13){
                    snprintf(log_buf, log_size, "%s cast %s: %s save (DC13).",
                             caster->type->name, s->name, target->type->name);
                    return 1;
                }
            }
            Effect se = {0};
            se.id = spell_id + 100;
            se.condition = COND_STUNNED;
            se.rounds_left = 3;    /* save moi turn de thoat */
            se.save_stat = (Ability)s->save;
            se.save_dc = 13;
            cond_add(target, se);
            snprintf(log_buf, log_size, "%s cast %s: %s bi STUNNED (save 13 de thoat).",
                     caster->type->name, s->name, target->type->name);
            break;
        }
        case SP_DEBUFF:
            /* Generic: save negates, apply condition */
            snprintf(log_buf, log_size, "%s cast %s (debuff).", caster->type->name, s->name);
            break;
        default:
            snprintf(log_buf, log_size, "%s cast %s (chua ho tro).", caster->type->name, s->name);
            return 0;
    }
    return 1;
}

/* AoE spell: hit tất cả actor trong radius từ target center. */
int spell_cast_aoe(int spell_id, Actor *caster, Actor *target,
                   Actor *all_actors, int n_actors,
                   int mod_spell, RNG *rng, char *log_buf, int log_size){
    if(spell_id < 0 || spell_id >= N_SPELLS){ return 0; }
    const SpellDef *s = &SPELLS[spell_id];
    if(s->aoe_ft == 0){
        /* Không phải AoE → fallback single target */
        return spell_cast(spell_id, caster, target, mod_spell, rng, log_buf, log_size);
    }
    int hit_count = 0;
    char buf[256];
    snprintf(log_buf, log_size, "%s cast %s (AoE %dft):", caster->type->name, s->name, s->aoe_ft);
    for(int i = 0; i < n_actors; i++){
        Actor *t = &all_actors[i];
        if(t == caster) continue;
        if(actor_is_dead(t)) continue;
        if(t->team == caster->team) continue;
        /* Trong radius? */
        int dx = t->x - target->x; if(dx < 0) dx = -dx;
        int dy = t->y - target->y; if(dy < 0) dy = -dy;
        int cheb = dx > dy ? dx : dy;
        if(cheb * 5 <= s->aoe_ft){
            char single[128];
            spell_cast(spell_id, caster, t, mod_spell, rng, single, sizeof(single));
            snprintf(buf, sizeof(buf), " %s;", single);
            if((int)strlen(log_buf) + (int)strlen(buf) < log_size - 1){
                strcat(log_buf, buf);
            }
            hit_count++;
        }
    }
    if(hit_count == 0){
        snprintf(log_buf, log_size, "%s cast %s: khong co target trong AoE.",
                 caster->type->name, s->name);
    }
    return hit_count;
}
