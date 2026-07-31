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
        default:
            snprintf(log_buf, log_size, "%s cast %s (chua ho tro).", caster->type->name, s->name);
            return 0;
    }
    return 1;
}
