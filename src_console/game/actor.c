/* =====================================================================
   ACTOR - Implementation
   ===================================================================== */
#include "actor.h"
#include "i18n.h"      /* Lang enum + g_lang extern declaration */
#include "../engine/rng.h"
#include <string.h>

/* Song ngu: g_lang owner o day (single definition, shared toan bo game).
   Default tieng Viet. Caller chi include i18n.h de dung (extern). */
Lang g_lang = LANG_VI;

int actor_ability_mod(int score){
    /* floor((score-10)/2). C integer division lam tron ve 0, can xu ly am. */
    int m = (score - 10) / 2;
    if((score - 10) % 2 != 0 && (score - 10) < 0) m--;
    return m;
}

int actor_roll_hp(const MonsterType *t, RNG *rng){
    int total = 0;
    for(int i = 0; i < t->hp_dice.count; i++){
        total += rng_range(rng, 1, t->hp_dice.sides);
    }
    total += t->hp_dice.mod;
    if(total < 1) total = 1;
    return total;
}

Actor actor_spawn(const MonsterType *type, int x, int y, Team team, RNG *rng){
    Actor a;
    memset(&a, 0, sizeof(a));
    a.type = type;
    a.max_hp = (uint16_t)actor_roll_hp(type, rng);
    a.hp = a.max_hp;
    a.x = (int8_t)x; a.y = (int8_t)y;
    a.team = team;
    a.conditions = COND_NONE;
    a.flags = (team == TEAM_PLAYER) ? EF_PLAYER : 0;
    a.move_left = type->speed;
    a.xp = type->xp;
    a.level = 0;
    return a;
}

void actor_refresh_turn(Actor *a){
    a->move_left = a->type ? a->type->speed : 0;
    a->action_used = 0;
    a->bonus_used = 0;
    /* reaction khong refresh o day - refresh dau ROUND, khong dau turn */
}

void actor_take_damage(Actor *a, int dmg){
    if(dmg <= 0 || actor_is_dead(a)) return;
    if((uint16_t)dmg >= a->hp){
        a->hp = 0;
        a->flags |= EF_DEAD;
        actor_set_condition(a, COND_UNCONSCIOUS);
    } else {
        a->hp = (uint16_t)(a->hp - dmg);
    }
}

void actor_heal(Actor *a, int hp){
    if(hp <= 0 || actor_is_dead(a)) return;
    a->hp += hp;
    if(a->hp > a->max_hp) a->hp = a->max_hp;
    if(a->hp > 0){
        a->flags &= ~EF_DEAD;
        actor_clear_condition(a, COND_UNCONSCIOUS);
    }
}

int actor_is_dead(const Actor *a){
    return (a->flags & EF_DEAD) || a->hp == 0;
}

int actor_has_condition(const Actor *a, Condition c){
    return (a->conditions & c) != 0;
}
void actor_set_condition(Actor *a, Condition c){
    a->conditions |= c;
}
void actor_clear_condition(Actor *a, Condition c){
    a->conditions &= ~(uint64_t)c;
}

uint16_t actor_glyph(const Actor *a){
    return a->type ? a->type->glyph : (uint16_t)'?';
}
int actor_color(const Actor *a){
    return a->type ? a->type->glyph_color : 7;
}

int actor_effective_ac(const Actor *a, int ac_bonus){
    int ac = a->type ? a->type->ac : 10;
    return ac + ac_bonus;
}

void actor_short_rest(Actor *a){
    /* Heal 1/4 max HP (5e: Hit Dice). Clear short-duration conditions. */
    if(actor_is_dead(a)) return;
    int heal = a->max_hp / 4;
    actor_heal(a, heal);
}

void actor_long_rest(Actor *a){
    /* Full heal, clear all conditions (5e long rest). */
    if(actor_is_dead(a)) return;
    a->hp = a->max_hp;
    a->conditions = COND_NONE;
    a->flags &= ~EF_UNCONSCIOUS;
}

int actor_can_act(const Actor *a, EconomySlot slot){
    switch(slot){
        case ECON_ACTION:    return !a->action_used;
        case ECON_BONUS:     return !a->bonus_used;
        case ECON_REACTION:  return !a->reaction_used;
        case ECON_MOVE:      return a->move_left > 0;
    }
    return 0;
}

void actor_consume(Actor *a, EconomySlot slot){
    switch(slot){
        case ECON_ACTION:    a->action_used = 1; break;
        case ECON_BONUS:     a->bonus_used = 1; break;
        case ECON_REACTION:  a->reaction_used = 1; break;
        case ECON_MOVE:      if(a->move_left > 0) a->move_left--; break;
    }
}

void actor_death_save(Actor *a, RNG *rng, DeathSaves *ds){
    /* 5e: d20. 10+ = success, <10 = fail. nat20 = 2 successes, nat1 = 2 fails. */
    (void)a;
    int roll = rng_range(rng, 1, 20);
    if(roll == 20){ ds->successes += 2; }
    else if(roll == 1){ ds->failures += 2; }
    else if(roll >= 10){ ds->successes += 1; }
    else { ds->failures += 1; }
    if(ds->successes >= 3){ ds->stable = 1; ds->successes = 3; }
    if(ds->failures >= 3){
        ds->failures = 3;
        a->flags |= EF_DEAD;   /* 3 fail = death */
    }
}

int actor_is_dying(const Actor *a){
    /* Dying = HP 0 but death saves chua 3 fail (not fully dead yet) */
    return (a->hp == 0) && !(a->flags & EF_DEAD);
}

const char *monster_name(const Actor *a){
    /* Song ngu: lay name_vi neu co va dang tieng Viet, nguoc lai name (EN) */
    if(!a || !a->type) return "?";
    if(g_lang == LANG_VI && a->type->name_vi) return a->type->name_vi;
    return a->type->name;
}
