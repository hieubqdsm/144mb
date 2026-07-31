/* =====================================================================
   AI - Monster AI hooks (goi tu monster_type->ai).
   Don gian: melee chaser (di gan player, tan cong khi adjacent).
   ===================================================================== */
#ifndef CE_AI_H
#define CE_AI_H

#include "../structs.h"

/* AI melee chaser: di chuyen 1 buoc gan target, tan cong neu adjacent.
   target = actor dau tien co team != self->team (don gian). */
void ai_melee_chaser(Actor *self, Actor *all, int n, void (*log)(const char*));

#endif /* CE_AI_H */
