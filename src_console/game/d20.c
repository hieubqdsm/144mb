/* =====================================================================
   D20 - Implementation
   ===================================================================== */
#include "d20.h"

D20Result d20_roll(int mod, RollMode mode, RNG *rng){
    D20Result r = {0};
    int a = rng_range(rng, 1, 20);
    int b = rng_range(rng, 1, 20);
    int die;
    if(mode == ROLL_ADVANTAGE)      die = (a > b) ? a : b;
    else if(mode == ROLL_DISADVANTAGE) die = (a < b) ? a : b;
    else die = a;
    r.die = (uint8_t)die;
    r.nat20 = (die == 20);
    r.nat1  = (die == 1);
    r.total = die + mod;
    return r;
}

int d20_roll_damage(DiceFormula f, int crit, RNG *rng){
    int total = 0;
    int count = f.count;
    if(crit) count *= 2;   /* 5e: double dice, khong double mod */
    for(int i = 0; i < count; i++){
        total += rng_range(rng, 1, f.sides);
    }
    total += f.mod;   /* mod chi cong 1 lan */
    return total;
}
