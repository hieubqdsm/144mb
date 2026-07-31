/* =====================================================================
   DATA - Item definitions (const compiled table).
   ===================================================================== */
#include "../game/items.h"

const ItemType ITEMS[] = {
    /* [ID_LONGSWORD] */
    {
        .name = "Longsword",
        .glyph = '/', .glyph_color = 7,        /* grey */
        .category = CAT_WEAPON, .slot = SLOT_WEAPON,
        .damage = {1,8,0},                      /* 1d8 */
        .atk_bonus = 0, .ac_bonus = 0,
        .heal = 0,
        .stackable = 0, .weight = 3,
    },
    /* [ID_CHAINSHIRT] */
    {
        .name = "Chain Shirt",
        .glyph = ']', .glyph_color = 7,
        .category = CAT_ARMOR, .slot = SLOT_ARMOR,
        .damage = {0,1,0}, .atk_bonus = 0, .ac_bonus = 3,  /* +3 AC */
        .heal = 0,
        .stackable = 0, .weight = 20,
    },
    /* [ID_HEAL_POTION] */
    {
        .name = "Healing Potion",
        .glyph = '!', .glyph_color = 12,       /* red */
        .category = CAT_POTION, .slot = SLOT_POTION,
        .damage = {0,1,0}, .atk_bonus = 0, .ac_bonus = 0,
        .heal = 8,                              /* heal 2d4+2 ~ 8 avg */
        .stackable = 1, .weight = 0,
    },
    /* [ID_DAGGER] */
    {
        .name = "Dagger",
        .glyph = '-', .glyph_color = 7,
        .category = CAT_WEAPON, .slot = SLOT_WEAPON,
        .damage = {1,4,0},                      /* 1d4 */
        .atk_bonus = 0, .ac_bonus = 0,
        .heal = 0,
        .stackable = 0, .weight = 1,
    },
};
const int N_ITEMS = (int)(sizeof(ITEMS)/sizeof(ITEMS[0]));
