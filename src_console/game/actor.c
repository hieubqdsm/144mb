/* =====================================================================
   ACTOR - Implementation
   ===================================================================== */
#include "actor.h"
#include <string.h>

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
