/* =====================================================================
   AI - Implementation
   ===================================================================== */
#include "ai.h"
#include "actor.h"
#include "combat.h"
#include "d20.h"
#include "../engine/rng.h"
#include "../engine/path.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern RNG g_r;

void ai_melee_chaser(Actor *self, Actor *all, int n, void (*log)(const char*)){
    /* Tim target (player) gan nhat, khong cung team, con song */
    Actor *target = NULL;
    int best_d = 99999;
    for(int i = 0; i < n; i++){
        if(all[i].team == self->team) continue;
        if(actor_is_dead(&all[i])) continue;
        int dx = abs(all[i].x - self->x);
        int dy = abs(all[i].y - self->y);
        int d = dx + dy;
        if(d < best_d){ best_d = d; target = &all[i]; }
    }
    if(!target){ log("Khong thay target."); return; }

    int dx = abs(self->x - target->x);
    int dy = abs(self->y - target->y);

    /* Adjacent? tan cong */
    if(dx <= 1 && dy <= 1){
        const MonsterAction *atk = &self->type->actions[0];
        int mod = actor_ability_mod(self->type->scores[AB_STR]);
        AtkResult r = combat_resolve_attack(atk, mod, target, ROLL_NORMAL, &g_r);
        char buf[80];
        if(r.fumble){
            snprintf(buf, sizeof(buf), "%s tan cong hut (NAT 1).", self->type->name);
        } else if(r.crit){
            combat_apply_damage(target, r.damage);
            snprintf(buf, sizeof(buf), "%s CRIT %s %d damage!", self->type->name, target->type->name, r.damage);
        } else if(r.hit){
            combat_apply_damage(target, r.damage);
            snprintf(buf, sizeof(buf), "%s danh %s %d damage.", self->type->name, target->type->name, r.damage);
        } else {
            snprintf(buf, sizeof(buf), "%s tan cong hut (AC %d).", self->type->name, target->type->ac);
        }
        log(buf);
        return;
    }

    /* Di chuyen 1 buoc gan target (greedy) */
    if(self->move_left <= 0){ log("(het move)"); return; }
    int mx = 0, my = 0;
    if(dx > dy){
        mx = (target->x > self->x) ? 1 : -1;
    } else if(dy > 0){
        my = (target->y > self->y) ? 1 : -1;
    } else if(dx > 0){
        mx = (target->x > self->x) ? 1 : -1;
    }
    /* Kiem tra khong di len ô da co monster khac */
    int nx = self->x + mx, ny = self->y + my;
    for(int i = 0; i < n; i++){
        if(&all[i] != self && all[i].x == nx && all[i].y == ny && !actor_is_dead(&all[i])){
            /* ô da co nguoi -> dung im */
            snprintf((char[]){0}, 1, "");
            return;
        }
    }
    self->x = (int8_t)nx; self->y = (int8_t)ny;
    self->move_left--;
}
