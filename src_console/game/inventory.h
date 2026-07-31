/* =====================================================================
   INVENTORY - Inventory + equip slots cho player.
   NetHack style: fixed-size backpack + equip slots.
   ===================================================================== */
#ifndef CE_INVENTORY_H
#define CE_INVENTORY_H

#include "items.h"
#include "../structs.h"
#include "../engine/rng.h"

#define INV_MAX 24

typedef struct {
    Item items[INV_MAX];     /* backpack */
    int count;
    Item equipped[SLOT_COUNT];  /* per-slot */
} Inventory;

/* Tao inventory rong. */
void inv_init(Inventory *inv);

/* Them item vao backpack (stack neu stackable). Tra ve 1 neu thanh cong. */
int inv_add(Inventory *inv, const ItemType *type, int qty);

/* Equip item tai slot. Tra ve item cuoi (ve backpack) neu co. */
int inv_equip(Inventory *inv, int backpack_idx);

/* Bo trang bi tai slot (ve backpack). */
int inv_unequip(Inventory *inv, EquipSlot slot);

/* Dung potion (heal). Tra ve 1 neu dung duoc. */
int inv_use_potion(Inventory *inv, Actor *healer, RNG *rng, char *log_buf, int log_size);

/* Bonus AC tu equipped armor + shield. */
int inv_total_ac_bonus(const Inventory *inv);

/* Bonus atk tu equipped weapon (magic +N). */
int inv_total_atk_bonus(const Inventory *inv);

/* Dem so item theo type id. */
int inv_count(const Inventory *inv, int item_id);

#endif /* CE_INVENTORY_H */
