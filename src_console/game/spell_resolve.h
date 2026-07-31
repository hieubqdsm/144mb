/* =====================================================================
   SPELL RESOLVE - Cast spell, apply effect.
   ===================================================================== */
#ifndef CE_SPELL_RESOLVE_H
#define CE_SPELL_RESOLVE_H

#include "spells.h"
#include "actor.h"
#include "conditions.h"
#include "../engine/rng.h"
#include "../structs.h"

/* Cast spell_id tu caster len target. mod_spell = spell attack bonus.
   log_buf nhan thong bao. Tra ve 1 neu cast thanh cong. */
int spell_cast(int spell_id, Actor *caster, Actor *target,
               int mod_spell, RNG *rng, char *log_buf, int log_size);

#endif /* CE_SPELL_RESOLVE_H */
