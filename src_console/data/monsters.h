/* =====================================================================
   DATA - Monster table declarations.
   ===================================================================== */
#ifndef CE_DATA_MONSTERS_H
#define CE_DATA_MONSTERS_H

#include "../structs.h"

extern const MonsterType MONSTERS[];

/* Number of monsters (compile-time cho array sizing) */
#define N_MONSTERS 8

/* IDs */
#define ID_GOBLIN    0
#define ID_PLAYER    1
#define ID_SKELETON  2
#define ID_WOLF      3
#define ID_ORC       4
#define ID_OGRE      5
#define ID_DRAGON    6
#define ID_ZOMBIE    7

#endif /* CE_DATA_MONSTERS_H */
