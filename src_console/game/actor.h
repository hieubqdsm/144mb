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

#endif /* CE_ACTOR_H */
