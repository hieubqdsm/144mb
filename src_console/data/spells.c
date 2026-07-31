/* =====================================================================
   DATA - Spell definitions (const table).
   ===================================================================== */
#include "../game/spells.h"

const SpellDef SPELLS[] = {
    /* [SPELL_FIRE_BOLT] cantrip */
    {
        .name = "Fire Bolt",
        .level = 0,
        .kind = SP_ATK_RANGED,
        .damage = {1,10,0},          /* 1d10 */
        .dmg_type = DMG_FIRE,
        .save = SAVE_NONE,
        .ac_bonus = 0,
        .glyph = '*', .glyph_color = 12,
    },
    /* [SPELL_MAGIC_MISSILE] level 1 */
    {
        .name = "Magic Missile",
        .level = 1,
        .kind = SP_MAGIC_MISSILE,
        .damage = {3,4,3},           /* 3d4+3 (3 darts) */
        .dmg_type = DMG_FORCE,
        .save = SAVE_NONE,
        .ac_bonus = 0,
        .glyph = '!', .glyph_color = 11,
    },
    /* [SPELL_FIREBALL] level 3 */
    {
        .name = "Fireball",
        .level = 3,
        .kind = SP_SAVE_HALF,
        .damage = {8,6,0},           /* 8d6 */
        .dmg_type = DMG_FIRE,
        .save = SAVE_DEX,
        .ac_bonus = 0,
        .glyph = '@', .glyph_color = 12,
    },
    /* [SPELL_MAGE_ARMOR] level 1 */
    {
        .name = "Mage Armor",
        .level = 1,
        .kind = SP_BUFF_AC,
        .damage = {0,1,0}, .dmg_type = DMG_FORCE,
        .save = SAVE_NONE,
        .ac_bonus = 3,               /* +3 AC */
        .glyph = '#', .glyph_color = 11,
    },
    /* [SPELL_CURE_WOUNDS] level 1 */
    {
        .name = "Cure Wounds",
        .level = 1,
        .kind = SP_HEAL,
        .damage = {1,8,3},           /* 1d8+3 heal */
        .dmg_type = DMG_RADIANT,
        .save = SAVE_NONE,
        .ac_bonus = 0,
        .glyph = '+', .glyph_color = 10,
    },
};
const int N_SPELLS = (int)(sizeof(SPELLS)/sizeof(SPELLS[0]));
