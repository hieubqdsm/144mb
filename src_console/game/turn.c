/* =====================================================================
   TURN - Implementation (initiative + round robin)
   ===================================================================== */
#include "turn.h"
#include "actor.h"
#include "../engine/rng.h"

static int g_turn_idx = 0;
static int g_round = 1;
static int g_order[64];   /* index cua actor theo initiative order */

void turn_roll_initiative(Actor *actors, int n, RNG *rng){
    if(n > 64) n = 64;
    for(int i = 0; i < n; i++){
        if(actor_is_dead(&actors[i])){
            actors[i].initiative = -99;
        } else {
            int dex = actors[i].type->scores[AB_DEX];
            actors[i].initiative = (int8_t)(rng_range(rng, 1, 20) + actor_ability_mod(dex));
        }
        g_order[i] = i;
    }
    /* Selection sort giam dan theo initiative (n nho nen OK) */
    for(int i = 0; i < n; i++){
        int best = i;
        for(int j = i+1; j < n; j++){
            if(actors[g_order[j]].initiative > actors[g_order[best]].initiative) best = j;
        }
        int t = g_order[i]; g_order[i] = g_order[best]; g_order[best] = t;
    }
    g_turn_idx = 0;
    g_round = 1;
    if(n > 0) actor_refresh_turn(&actors[g_order[0]]);
}

Actor *turn_current(Actor *actors, int n){
    if(n <= 0) return NULL;
    /* Skip dead actors */
    for(int attempt = 0; attempt < n; attempt++){
        int idx = g_order[g_turn_idx % n];
        if(!actor_is_dead(&actors[idx])) return &actors[idx];
        g_turn_idx++;
    }
    return NULL;
}

int turn_advance(Actor *actors, int n){
    g_turn_idx++;
    if(g_turn_idx % n == 0){
        g_round++;
        /* Refresh reaction cho tat ca dau round */
        for(int i = 0; i < n; i++) actors[i].reaction_used = 0;
    }
    Actor *next = turn_current(actors, n);
    if(next) actor_refresh_turn(next);
    return (g_turn_idx % n == 0);
}

void turn_reset(void){
    g_turn_idx = 0;
    g_round = 1;
}
