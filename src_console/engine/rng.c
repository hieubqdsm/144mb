/* =====================================================================
   RNG - Implementation (xorshift64)
   ===================================================================== */
#include "rng.h"

/* Global RNG cho game don luong. */
RNG g_rng = { 0xDEADBEEFCAFEBABEULL };

void rng_seed(RNG *r, uint64_t seed){
    /* Tranh state=0 (xorshift se stuck o 0). */
    r->state = seed ? seed : 0xDEADBEEFCAFEBABEULL;
}

uint64_t rng_next(RNG *r){
    uint64_t x = r->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    r->state = x;
    return x;
}

int rng_range(RNG *r, int lo, int hi){
    if(hi < lo){ int t=lo; lo=hi; hi=t; }
    /* khoang [lo, hi] co (hi-lo+1) gia tri */
    unsigned int range = (unsigned int)(hi - lo + 1);
    return lo + (int)(rng_next(r) % range);
}

float rng_float(RNG *r){
    /* 53-bit precision: lay 53 bit cao chia 2^53 */
    return (float)((rng_next(r) >> 11) * (1.0/9007199254740992.0));
}

int rng_chance(RNG *r, float p){
    return rng_float(r) < p;
}

/* Global helpers */
void grng_seed(uint64_t seed){ rng_seed(&g_rng, seed); }
int  grng_range(int lo, int hi){ return rng_range(&g_rng, lo, hi); }
float grng_float(void){ return rng_float(&g_rng); }
int  grng_chance(float p){ return rng_chance(&g_rng, p); }
