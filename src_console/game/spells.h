/* =====================================================================
   SPELLS - Spell definitions + resolve (D&D-lite).
   SpellKind enum gom ~15 cach resolve, collapse hundreds of spells.
   ===================================================================== */
#ifndef CE_SPELLS_H
#define CE_SPELLS_H

#include "../structs.h"
#include "../enums.h"

typedef enum {
    SP_ATK_RANGED,      /* fire bolt: spell atk vs AC */
    SP_SAVE_HALF,       /* fireball: save for half */
    SP_SAVE_NEGATE,     /* hold person: save or nothing */
    SP_BUFF_AC,         /* mage armor: +AC (concentration) */
    SP_SHIELD,          /* shield: reaction +5 AC */
    SP_HEAL,            /* cure wounds */
    SP_MAGIC_MISSILE,   /* auto-hit, no save */
    SP_POISON,          /* poison: save CON neu fail -> DOT poison */
    SP_STUN,            /* stun: save CON neu fail -> stunned condition */
    SP_DEBUFF,          /* debuff: save negates, gan condition */
} SpellKind;

typedef struct {
    const char *name;
    uint8_t level;              /* 0 = cantrip */
    SpellKind kind;
    DiceFormula damage;
    DamageType dmg_type;
    SaveType save;              /* SAVE_NONE for atk-roll */
    uint8_t ac_bonus;           /* for buff spells */
    uint16_t glyph;
    int glyph_color;
} SpellDef;

extern const SpellDef SPELLS[];
extern const int N_SPELLS;
#define SPELL_FIRE_BOLT    0
#define SPELL_MAGIC_MISSILE 1
#define SPELL_FIREBALL     2
#define SPELL_MAGE_ARMOR   3
#define SPELL_CURE_WOUNDS  4
#define SPELL_POISON_SPRAY 5
#define SPELL_HOLD_PERSON  6

#endif /* CE_SPELLS_H */
