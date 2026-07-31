/* =====================================================================
   DATA - Monster table declarations.
   ===================================================================== */
#ifndef CE_DATA_MONSTERS_H
#define CE_DATA_MONSTERS_H

#include "../structs.h"

extern const MonsterType MONSTERS[];

/* Number of monsters (compile-time cho array sizing) */
#define N_MONSTERS 2

/* IDs */
#define ID_GOBLIN  0
#define ID_PLAYER  1

#endif /* CE_DATA_MONSTERS_H */
