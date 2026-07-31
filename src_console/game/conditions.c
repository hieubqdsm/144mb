/* =====================================================================
   CONDITIONS - Implementation
   NOTE: Effect list duoc luu ngoai Actor (global arrays) de giu Actor nho.
   Cho demo don gian: luu Effect trong 1 global pool, link theo actor index.
   ===================================================================== */
#include "conditions.h"
#include "actor.h"
#include "d20.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_EFFECTS 128
static Effect g_effect_pool[MAX_EFFECTS];
static int g_effect_owner[MAX_EFFECTS];   /* actor pointer (identity) - hack don gian */

static Effect *find_free_effect(void){
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_pool[i].id == 0 && g_effect_owner[i] == 0) return &g_effect_pool[i];
    }
    return NULL;
}

void cond_add(Actor *a, Effect e){
    Effect *slot = find_free_effect();
    if(!slot) return;
    *slot = e;
    slot->next = NULL;
    int slot_idx = (int)(slot - g_effect_pool);
    g_effect_owner[slot_idx] = (int)(intptr_t)a;
    /* Apply condition bitmask ngay */
    if(e.condition) actor_set_condition(a, e.condition);
}

static Effect *first_effect(Actor *a){
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_owner[i] == (int)(intptr_t)a && g_effect_pool[i].id != 0){
            return &g_effect_pool[i];
        }
    }
    return NULL;
}

int cond_resolve_turn(Actor *a, RNG *rng, char *log_buf, int log_buf_size){
    int expired = 0;
    int total_dmg = 0;
    DamageType dmg_type = DMG_POISON;
    log_buf[0] = 0;
    Effect *e = first_effect(a);
    while(e){
        Effect *next = e->next;
        /* DOT damage */
        if(e->dmg_per_turn > 0){
            total_dmg += e->dmg_per_turn;
            dmg_type = e->dmg_type;
        }
        /* Save check (save-ends effect) */
        if(e->save_stat != SAVE_NONE && e->save_dc > 0){
            int score = a->type->scores[e->save_stat - SAVE_STR];
            int mod = actor_ability_mod(score);
            D20Result r = d20_roll(mod, ROLL_NORMAL, rng);
            if(r.total >= e->save_dc){
                /* Save thanh cong -> expire */
                e->rounds_left = 0;
            }
        }
        /* Tick duration */
        if(e->rounds_left > 0) e->rounds_left--;
        if(e->rounds_left == 0){
            /* Expire: remove condition */
            if(e->condition) actor_clear_condition(a, e->condition);
            e->id = 0;
            g_effect_owner[(int)(e - g_effect_pool)] = 0;
            expired++;
        }
        e = next;
    }
    /* Apply DOT */
    if(total_dmg > 0){
        actor_take_damage(a, total_dmg);
        snprintf(log_buf, log_buf_size, "%s take %d %s damage (DOT)",
                 a->type->name, total_dmg,
                 dmg_type==DMG_POISON?"poison":"damage");
    }
    return expired;
}

int cond_count(const Actor *a){
    int n = 0;
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_owner[i] == (int)(intptr_t)a && g_effect_pool[i].id != 0) n++;
    }
    return n;
}

int cond_ac_bonus(const Actor *a){
    int bonus = 0;
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_owner[i] == (int)(intptr_t)a && g_effect_pool[i].id != 0){
            bonus += g_effect_pool[i].ac_bonus;
        }
    }
    return bonus;
}
int cond_atk_bonus(const Actor *a){
    int bonus = 0;
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_owner[i] == (int)(intptr_t)a && g_effect_pool[i].id != 0){
            bonus += g_effect_pool[i].atk_bonus;
        }
    }
    return bonus;
}

void cond_clear_all(Actor *a){
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(g_effect_owner[i] == (int)(intptr_t)a){
            g_effect_pool[i].id = 0;
            g_effect_owner[i] = 0;
        }
    }
    a->conditions = COND_NONE;
}
