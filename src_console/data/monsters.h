/* =====================================================================
   DATA - Monster table declarations.
   ===================================================================== */
#ifndef CE_DATA_MONSTERS_H
#define CE_DATA_MONSTERS_H

#include "../structs.h"

extern const MonsterType MONSTERS[];

/* Number of monsters (compile-time cho array sizing) */
#define N_MONSTERS 19   /* 8 base + 11 LMoP */

/* IDs - base 8 (co san) */
#define ID_GOBLIN    0
#define ID_PLAYER    1
#define ID_SKELETON  2
#define ID_WOLF      3
#define ID_ORC       4
#define ID_OGRE      5
#define ID_DRAGON    6
#define ID_ZOMBIE    7

/* IDs - LMoP 11 moi */
#define ID_BUGBEAR     8   /* Bugbear - CR 1, Cragmaw Hideout/Castle */
#define ID_REDBRAND    9   /* Redbrand Ruffian - CR 1/2, Phandalin/Hideout */
#define ID_HOBGOBLIN  10   /* Hobgoblin - CR 1/2, wilderness encounter */
#define ID_OWBEAR     11   /* Owlbear - CR 3, wilderness encounter */
#define ID_GRICK      12   /* Grick - CR 2, Wave Echo Cave */
#define ID_SPECTATOR  13   /* Spectator - CR 3, Wave Echo Cave guardian */
#define ID_OCHRE_JELLY 14  /* Ochre Jelly - CR 2, Wave Echo Cave */
#define ID_WRAITH     15   /* Mormesk Wraith - CR 4, Wave Echo Cave */
#define ID_NEZZNAR    16   /* Black Spider - CR 2, FINAL BOSS Wave Echo Cave */
#define ID_SILDAR     17   /* Sildar Hallwinter - CR 1, NPC rescue */
#define ID_VENOMFANG  18   /* Young Green Dragon - CR 4, Thundertree optional boss */

#endif /* CE_DATA_MONSTERS_H */
