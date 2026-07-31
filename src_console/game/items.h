/* =====================================================================
   ITEMS - Item definitions + instances (type/instance split, NetHack style).
   ItemType = const data (ten, weight, dice, slot).
   Item = runtime instance (qty, charges, enchantment).
   ===================================================================== */
#ifndef CE_ITEMS_H
#define CE_ITEMS_H

#include "../structs.h"
#include "../enums.h"

/* Equip slots */
typedef enum {
    SLOT_NONE = 0,
    SLOT_WEAPON,
    SLOT_ARMOR,
    SLOT_SHIELD,
    SLOT_RING,
    SLOT_POTION,
    SLOT_COUNT
} EquipSlot;

/* Item category */
typedef enum {
    CAT_WEAPON, CAT_ARMOR, CAT_POTION, CAT_CONSUMABLE, CAT_MISC
} ItemCategory;

/* Item TYPE (const, data-driven) */
typedef struct {
    const char *name;
    uint16_t glyph;          /* WCHAR de render */
    int glyph_color;         /* CE_Color */
    ItemCategory category;
    EquipSlot slot;
    /* Weapon: dice + bonus */
    DiceFormula damage;
    int8_t atk_bonus;        /* magic +1 weapon */
    int8_t ac_bonus;         /* armor/shield */
    /* Potion: heal amount */
    int8_t heal;
    uint8_t stackable;       /* 1 = co the xep chong */
    uint8_t weight;
} ItemType;

/* Item INSTANCE */
typedef struct {
    const ItemType *type;
    uint16_t qty;
    int8_t enchant;          /* +1, +2 magic weapon */
    uint8_t charges;         /* wand charges, 0 = N/A */
} Item;

/* Table (defined trong data/items.c) */
extern const ItemType ITEMS[];
extern const int N_ITEMS;
/* IDs */
#define ID_LONGSWORD    0
#define ID_CHAINSHIRT   1
#define ID_HEAL_POTION  2
#define ID_DAGGER       3

#endif /* CE_ITEMS_H */
