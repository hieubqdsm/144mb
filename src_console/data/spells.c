/* =====================================================================
   DATA - Spell definitions (const compiled table).
   =====================================================================
   CACH DOC:
     .damage   = DICE(1,10,0)      <- "1d10+0"
     .kind     = SP_ATK_RANGED     <- loai resolve (roll vs AC)
     .save     = SAVE_DEX          <- saving throw (SAVE_NONE = atk roll)
     .level    = 0                 <- cantrip (0 = khong ton slot)
   ===================================================================== */
#include "../game/spells.h"
#include "../structs.h"
#include "../engine/console.h"

const SpellDef SPELLS[] = {

    /* [SPELL_FIRE_BOLT] - cantrip, atk roll vs AC, 1d10 fire */
    [SPELL_FIRE_BOLT] = {
        .name        = "Fire Bolt",
        .level       = 0,                               /* cantrip */
        .kind        = SP_ATK_RANGED,
        .damage      = DICE(1, 10, 0),                  /* 1d10 */
        .dmg_type    = DMG_FIRE,
        .save        = SAVE_NONE,
        .ac_bonus    = 0,
        .glyph       = '*',
        .glyph_color = CE_RED,
    },

    /* [SPELL_MAGIC_MISSILE] - lv1, auto-hit, 3d4+3 force */
    [SPELL_MAGIC_MISSILE] = {
        .name        = "Magic Missile",
        .level       = 1,
        .kind        = SP_MAGIC_MISSILE,
        .damage      = DICE(3, 4, 3),                   /* 3d4+3 (3 darts) */
        .dmg_type    = DMG_FORCE,
        .save        = SAVE_NONE,
        .ac_bonus    = 0,
        .glyph       = '!',
        .glyph_color = CE_CYAN,
    },

    /* [SPELL_FIREBALL] - lv3, save DEX for half, 8d6 fire */
    [SPELL_FIREBALL] = {
        .name        = "Fireball",
        .level       = 3,
        .kind        = SP_SAVE_HALF,
        .damage      = DICE(8, 6, 0),                   /* 8d6 */
        .dmg_type    = DMG_FIRE,
        .save        = SAVE_DEX,
        .ac_bonus    = 0,
        .glyph       = '@',
        .glyph_color = CE_RED,
    },

    /* [SPELL_MAGE_ARMOR] - lv1, buff +3 AC */
    [SPELL_MAGE_ARMOR] = {
        .name        = "Mage Armor",
        .level       = 1,
        .kind        = SP_BUFF_AC,
        .damage      = DICE(0, 1, 0),
        .dmg_type    = DMG_FORCE,
        .save        = SAVE_NONE,
        .ac_bonus    = 3,                               /* +3 AC */
        .glyph       = '#',
        .glyph_color = CE_CYAN,
    },

    /* [SPELL_CURE_WOUNDS] - lv1, heal 1d8+3 */
    [SPELL_CURE_WOUNDS] = {
        .name        = "Cure Wounds",
        .level       = 1,
        .kind        = SP_HEAL,
        .damage      = DICE(1, 8, 3),                   /* 1d8+3 heal */
        .dmg_type    = DMG_RADIANT,
        .save        = SAVE_NONE,
        .ac_bonus    = 0,
        .glyph       = '+',
        .glyph_color = CE_GREEN,
    },

    /* [SPELL_POISON_SPRAY] - cantrip, save CON neu fail -> poison DOT */
    [SPELL_POISON_SPRAY] = {
        .name        = "Poison Spray",
        .level       = 0,                               /* cantrip */
        .kind        = SP_POISON,
        .damage      = DICE(1, 12, 0),                  /* 1d12 ngay + DOT */
        .dmg_type    = DMG_POISON,
        .save        = SAVE_CON,
        .ac_bonus    = 0,
        .glyph       = '#',
        .glyph_color = CE_DGREEN,
    },

    /* [SPELL_HOLD_PERSON] - lv2, save WIS neu fail -> stunned (mất turn) */
    [SPELL_HOLD_PERSON] = {
        .name        = "Hold Person",
        .level       = 2,
        .kind        = SP_STUN,
        .damage      = DICE(0, 1, 0),
        .dmg_type    = DMG_FORCE,
        .save        = SAVE_WIS,
        .ac_bonus    = 0,
        .glyph       = '*',
        .glyph_color = CE_CYAN,
    },
};
const int N_SPELLS = (int)(sizeof(SPELLS)/sizeof(SPELLS[0]));
