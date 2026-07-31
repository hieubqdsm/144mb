/* =====================================================================
   INVENTORY - Implementation
   ===================================================================== */
#include "inventory.h"
#include "actor.h"
#include "d20.h"
#include <string.h>
#include <stdio.h>

void inv_init(Inventory *inv){
    memset(inv, 0, sizeof(*inv));
}

int inv_add(Inventory *inv, const ItemType *type, int qty){
    if(!type || qty <= 0) return 0;
    /* Stack vao existing */
    if(type->stackable){
        for(int i = 0; i < inv->count; i++){
            if(inv->items[i].type == type){
                inv->items[i].qty += qty;
                return 1;
            }
        }
    }
    if(inv->count >= INV_MAX) return 0;
    inv->items[inv->count].type = type;
    inv->items[inv->count].qty = qty;
    inv->items[inv->count].enchant = 0;
    inv->items[inv->count].charges = 0;
    inv->count++;
    return 1;
}

int inv_equip(Inventory *inv, int idx){
    if(idx < 0 || idx >= inv->count) return 0;
    Item *it = &inv->items[idx];
    if(!it->type) return 0;
    EquipSlot slot = it->type->slot;
    if(slot == SLOT_NONE || slot == SLOT_POTION) return 0;
    /* Hoan doi: item cu ve backpack (giu nguyen vi tri idx) */
    Item swap = inv->equipped[slot];
    inv->equipped[slot] = *it;
    if(swap.type){
        *it = swap;
    } else {
        /* Xoa khoi backpack (shift) */
        for(int i = idx; i < inv->count - 1; i++) inv->items[i] = inv->items[i+1];
        inv->count--;
        memset(&inv->items[inv->count], 0, sizeof(Item));
    }
    return 1;
}

int inv_unequip(Inventory *inv, EquipSlot slot){
    if(slot <= SLOT_NONE || slot >= SLOT_COUNT) return 0;
    Item *eq = &inv->equipped[slot];
    if(!eq->type) return 0;
    if(inv->count >= INV_MAX) return 0;
    inv->items[inv->count++] = *eq;
    memset(eq, 0, sizeof(Item));
    return 1;
}

int inv_use_potion(Inventory *inv, Actor *healer, RNG *rng, char *log_buf, int log_size){
    for(int i = 0; i < inv->count; i++){
        if(inv->items[i].type && inv->items[i].type->category == CAT_POTION){
            int heal = inv->items[i].type->heal;
            /* Randomize: 2d4+2 */
            heal = rng_range(rng, 1,4) + rng_range(rng, 1,4) + 2;
            actor_heal(healer, heal);
            snprintf(log_buf, log_size, "%s drink potion, heal %d HP. (%d/%d)",
                     healer->type->name, heal, healer->hp, healer->max_hp);
            /* Consume */
            inv->items[i].qty--;
            if(inv->items[i].qty <= 0){
                for(int j = i; j < inv->count-1; j++) inv->items[j] = inv->items[j+1];
                inv->count--;
            }
            return 1;
        }
    }
    snprintf(log_buf, log_size, "Khong co potion!");
    return 0;
}

int inv_total_ac_bonus(const Inventory *inv){
    int bonus = 0;
    for(int s = SLOT_ARMOR; s <= SLOT_SHIELD; s++){
        if(inv->equipped[s].type) bonus += inv->equipped[s].type->ac_bonus;
    }
    if(inv->equipped[SLOT_RING].type) bonus += inv->equipped[SLOT_RING].type->ac_bonus;
    return bonus;
}

int inv_total_atk_bonus(const Inventory *inv){
    int bonus = 0;
    if(inv->equipped[SLOT_WEAPON].type) bonus += inv->equipped[SLOT_WEAPON].type->atk_bonus;
    return bonus;
}

int inv_count(const Inventory *inv, int item_id){
    int n = 0;
    for(int i = 0; i < inv->count; i++){
        if(inv->items[i].type == &ITEMS[item_id]) n += inv->items[i].qty;
    }
    return n;
}
