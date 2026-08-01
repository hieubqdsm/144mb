/* =====================================================================
   DATA - Item definitions (const compiled table).
   =====================================================================
   CACH DOC:
     .damage   = DICE(1,8,0)       <- "1d8+0"
     .slot     = SLOT_WEAPON       <- thay so
     .category = CAT_WEAPON        <- thay so
   ===================================================================== */
#include "../game/items.h"
#include "../structs.h"
#include "../engine/console.h"   /* CE_* colors */

const ItemType ITEMS[] = {

    /* [ID_LONGSWORD] - melee weapon chính */
    [ID_LONGSWORD] = {
        .name        = "Longsword",
        .glyph       = '/',
        .glyph_color = CE_GREY,
        .category    = CAT_WEAPON,
        .slot        = SLOT_WEAPON,
        .damage      = DICE(1, 8, 0),                  /* 1d8 */
        .atk_bonus   = 0,
        .ac_bonus    = 0,
        .heal        = 0,
        .stackable   = 0,
        .weight      = 3,
    },

    /* [ID_CHAINSHIRT] - medium armor (+3 AC) */
    [ID_CHAINSHIRT] = {
        .name        = "Chain Shirt",
        .glyph       = ']',
        .glyph_color = CE_GREY,
        .category    = CAT_ARMOR,
        .slot        = SLOT_ARMOR,
        .damage      = DICE(0, 1, 0),                  /* khong phai vu khi */
        .atk_bonus   = 0,
        .ac_bonus    = 3,                               /* +3 AC */
        .heal        = 0,
        .stackable   = 0,
        .weight      = 20,
    },

    /* [ID_HEAL_POTION] - consumable, heal 2d4+2 */
    [ID_HEAL_POTION] = {
        .name        = "Healing Potion",
        .glyph       = '!',
        .glyph_color = CE_RED,
        .category    = CAT_POTION,
        .slot        = SLOT_POTION,
        .damage      = DICE(0, 1, 0),
        .atk_bonus   = 0,
        .ac_bonus    = 0,
        .heal        = 8,                               /* ~2d4+2 trung binh */
        .stackable   = 1,
        .weight      = 0,
    },

    /* [ID_DAGGER] - light weapon */
    [ID_DAGGER] = {
        .name        = "Dagger",
        .glyph       = '-',
        .glyph_color = CE_GREY,
        .category    = CAT_WEAPON,
        .slot        = SLOT_WEAPON,
        .damage      = DICE(1, 4, 0),                  /* 1d4 */
        .atk_bonus   = 0,
        .ac_bonus    = 0,
        .heal        = 0,
        .stackable   = 0,
        .weight      = 1,
    },
};
const int N_ITEMS = (int)(sizeof(ITEMS)/sizeof(ITEMS[0]));
