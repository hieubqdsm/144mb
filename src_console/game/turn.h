/* =====================================================================
   TURN - Initiative system + round-based turn loop.
   D&D 5e: roll d20 + DEX mod cho moi participant, sap xep giam dan.
   Round robin: turn_idx tang, reset khi het round.
   ===================================================================== */
#ifndef CE_TURN_H
#define CE_TURN_H

#include "../structs.h"
#include "../enums.h"
#include "../engine/rng.h"

/* Roll initiative cho moi actor (d20 + DEX mod). Sap xep order[] giam dan. */
void turn_roll_initiative(Actor *actors, int n, RNG *rng);

/* Lay actor hien tai trong turn order. */
Actor *turn_current(Actor *actors, int n);

/* Sang turn tiep theo. Tra ve 1 neu sang round moi (de resolve effects). */
int turn_advance(Actor *actors, int n);

/* Reset turn system (cho combat moi). */
void turn_reset(void);

#endif /* CE_TURN_H */
