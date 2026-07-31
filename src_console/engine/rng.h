/* =====================================================================
   RNG - Xorshift64 deterministic random number generator.
   Nhan hon rand(), reproducible (seed), khong thread-unsafe global.
   ===================================================================== */
#ifndef CE_RNG_H
#define CE_RNG_H

#include <stdint.h>

typedef struct {
    uint64_t state;   /* != 0 */
} RNG;

/* Khoi tao seed. state=0 se duoc sua thanh 1. */
void rng_seed(RNG *r, uint64_t seed);

/* Tra ve uint64 ngau nhien. */
uint64_t rng_next(RNG *r);

/* Tra ve int trong [lo, hi] (khep kin). */
int rng_range(RNG *r, int lo, int hi);

/* Tra ve float trong [0.0, 1.0). */
float rng_float(RNG *r);

/* Tra ve 1 (true) voi xac suat p (0.0 - 1.0). */
int rng_chance(RNG *r, float p);

/* Convenience: global RNG (cho game don luong). */
extern RNG g_rng;
void grng_seed(uint64_t seed);
int  grng_range(int lo, int hi);
float grng_float(void);
int  grng_chance(float p);

#endif /* CE_RNG_H */
