/* =====================================================================
   D20 - Dice resolution (D&D 5e style).
   roll_d20 co advantage/disadvantage, phat hien crit (nat 20) / fumble (nat 1).
   roll_damage nhan DiceFormula, double dice khi crit.
   ===================================================================== */
#ifndef CE_D20_H
#define CE_D20_H

#include "../structs.h"
#include "../enums.h"
#include "../engine/rng.h"

/* Ket qua 1 roll d20. */
typedef struct {
    uint8_t die;       /* raw d20 result (1-20) */
    uint8_t nat20 : 1; /* crit */
    uint8_t nat1  : 1; /* fumble */
    int total;         /* die + mod (sau advantage) */
} D20Result;

/* Roll d20 + mod. mode = advantage (roll 2 keep highest) / disadvantage / normal. */
D20Result d20_roll(int mod, RollMode mode, RNG *rng);

/* Roll damage tu DiceFormula. crit=1 -> double so dice (khong double mod). */
int d20_roll_damage(DiceFormula f, int crit, RNG *rng);

#endif /* CE_D20_H */
