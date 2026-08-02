/* =====================================================================
   ACTOR - Helpers cho Actor instance lifecycle.
   Type/instance split: MonsterType (data) -> Actor (instance).
   ===================================================================== */
#ifndef CE_ACTOR_H
#define CE_ACTOR_H

#include "../structs.h"
#include "../enums.h"

/* Modifier: floor((score - 10) / 2) */
int actor_ability_mod(int score);

/* HP tu hp_dice formula (roll khi tao). */
int actor_roll_hp(const MonsterType *t, RNG *rng);

/* Tao Actor instance tu MonsterType tai (x,y). HP duoc roll. */
Actor actor_spawn(const MonsterType *type, int x, int y, Team team, RNG *rng);

/* Refresh action economy dau moi turn (reset move/action/bonus). */
void actor_refresh_turn(Actor *a);

/* Damage / heal */
void actor_take_damage(Actor *a, int dmg);
void actor_heal(Actor *a, int hp);

/* Query */
int actor_is_dead(const Actor *a);
int actor_has_condition(const Actor *a, Condition c);
void actor_set_condition(Actor *a, Condition c);
void actor_clear_condition(Actor *a, Condition c);

/* Glyph/color de render (lay tu type). */
uint16_t actor_glyph(const Actor *a);
int actor_color(const Actor *a);

/* Ten hien thi theo ngon ngu hien tai (g_lang, song ngu VI/EN).
   Tra ve type->name_vi neu co, nguoc lai type->name. */
const char *monster_name(const Actor *a);

/* AC hieu qua = base AC (tu type) + ac_bonus (tu conditions/equipment).
   Caller truyen ac_bonus (vd tu inv_total_ac_bonus + cond_ac_bonus). */
int actor_effective_ac(const Actor *a, int ac_bonus);

/* Resting */
void actor_short_rest(Actor *a);   /* heal 1/4 max HP, clear short conditions */
void actor_long_rest(Actor *a);    /* full heal, clear all conditions */

/* Action economy checks (5e: action, bonus action, reaction, move). */
typedef enum { ECON_ACTION=0, ECON_BONUS, ECON_REACTION, ECON_MOVE } EconomySlot;
int actor_can_act(const Actor *a, EconomySlot slot);   /* 1 = con slot */
void actor_consume(Actor *a, EconomySlot slot);        /* danh dau da dung */

/* Death saves (player only) */
typedef struct {
    int successes;   /* 0-3 */
    int failures;    /* 0-3 */
    int stable;      /* 1 = stable (3 successes) */
} DeathSaves;
void actor_death_save(Actor *a, RNG *rng, DeathSaves *ds);   /* roll d20 */
int actor_is_dying(const Actor *a);   /* HP==0 but not dead yet (death saves active) */

#endif /* CE_ACTOR_H */
