/* =====================================================================
   AI - Implementation
   ===================================================================== */
#include "ai.h"
#include "actor.h"
#include "combat.h"
#include "conditions.h"
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
        int target_ac = actor_effective_ac(target, cond_ac_bonus(target));
        AtkResult r = combat_resolve_attack(atk, mod, target_ac, ROLL_NORMAL, &g_r);
        char buf[80];
        if(r.fumble){
            snprintf(buf, sizeof(buf), "%s tan cong hut (NAT 1).", monster_name(self));
        } else if(r.crit){
            combat_apply_damage(target, r.damage);
            snprintf(buf, sizeof(buf), "%s CRIT %s %d damage!", monster_name(self), monster_name(target), r.damage);
        } else if(r.hit){
            combat_apply_damage(target, r.damage);
            snprintf(buf, sizeof(buf), "%s danh %s %d damage.", monster_name(self), monster_name(target), r.damage);
        } else {
            snprintf(buf, sizeof(buf), "%s tan cong hut (AC %d).", monster_name(self), target->type->ac);
        }
        log(buf);
        return;
    }

    /* Di chuyen gan target cho den khi adjacent HOAC het move_left (speed).
       Wolf speed=8 se di 8 ô/luot, Zombie speed=4 chi 4 ô. */
    while(self->move_left > 0 && (abs(self->x-target->x) > 1 || abs(self->y-target->y) > 1)){
        int mx = 0, my = 0;
        int ddx = abs(self->x - target->x), ddy = abs(self->y - target->y);
        if(ddx > ddy){
            mx = (target->x > self->x) ? 1 : -1;
        } else if(ddy > 0){
            my = (target->y > self->y) ? 1 : -1;
        } else if(ddx > 0){
            mx = (target->x > self->x) ? 1 : -1;
        }
        /* Kiem tra khong di len ô da co monster khac */
        int nx = self->x + mx, ny = self->y + my;
        int blocked = 0;
        for(int i = 0; i < n; i++){
            if(&all[i] != self && all[i].x == nx && all[i].y == ny && !actor_is_dead(&all[i])){
                blocked = 1; break;
            }
        }
        if(blocked) break;
        self->x = (int8_t)nx; self->y = (int8_t)ny;
        self->move_left--;
    }
}

/* Helper: tim target player gan nhat */
static Actor *find_target(Actor *self, Actor *all, int n){
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
    return target;
}

/* Helper: di chuyen 1 buoc (greedy) gan target, tranh monster khac */
static int step_toward(Actor *self, Actor *target, Actor *all, int n){
    if(self->move_left <= 0) return 0;
    int dx = target->x - self->x, dy = target->y - self->y;
    int mx = 0, my = 0;
    if(abs(dx) > abs(dy))      mx = (dx > 0) ? 1 : -1;
    else if(dy != 0)           my = (dy > 0) ? 1 : -1;
    else if(dx != 0)           mx = (dx > 0) ? 1 : -1;
    else return 0;
    int nx = self->x + mx, ny = self->y + my;
    for(int i = 0; i < n; i++){
        if(&all[i] != self && all[i].x == nx && all[i].y == ny && !actor_is_dead(&all[i])) return 0;
    }
    self->x = (int8_t)nx; self->y = (int8_t)ny;
    self->move_left--;
    return 1;
}

/* Helper: thuc hien 1 attack (action[0] hoac [1]) */
static void do_attack(Actor *self, Actor *target, int action_idx, void (*log)(const char*)){
    if(action_idx >= self->type->n_actions) action_idx = 0;
    const MonsterAction *atk = &self->type->actions[action_idx];
    /* Ranged: DEX mod. Melee: STR mod. */
    int is_ranged = (atk->range_max > 0);
    int score = self->type->scores[is_ranged ? AB_DEX : AB_STR];
    int mod = actor_ability_mod(score);
    int target_ac = actor_effective_ac(target, cond_ac_bonus(target));
    AtkResult r = combat_resolve_attack(atk, mod, target_ac, ROLL_NORMAL, &g_r);
    char buf[96];
    if(r.fumble){
        snprintf(buf, sizeof(buf), "%s tan cong hut (NAT 1).", monster_name(self));
    } else if(r.crit){
        combat_apply_damage(target, r.damage);
        snprintf(buf, sizeof(buf), "%s CRIT %s %d damage!", monster_name(self), monster_name(target), r.damage);
    } else if(r.hit){
        combat_apply_damage(target, r.damage);
        snprintf(buf, sizeof(buf), "%s danh %s %d damage.", monster_name(self), monster_name(target), r.damage);
    } else {
        snprintf(buf, sizeof(buf), "%s tan cong %s: hut (AC %d).", monster_name(self), monster_name(target), target->type->ac);
    }
    log(buf);
}

void ai_ranged(Actor *self, Actor *all, int n, void (*log)(const char*)){
    Actor *target = find_target(self, all, n);
    if(!target) return;
    int dist = abs(self->x - target->x) + abs(self->y - target->y);
    /* Xa (>6): tan cong cung. Gan (<3): chay nguoc. Giua: di chuyen. */
    if(dist > 6){
        do_attack(self, target, 1, log);  /* ranged action */
    } else if(dist < 3){
        /* Chay nguoc target 1 buoc */
        int dx = self->x - target->x, dy = self->y - target->y;
        if(self->move_left > 0){
            self->x += (dx >= 0) ? 1 : -1;
            if(self->x < 1) self->x = 1;
            self->move_left--;
        }
        log("(ranged lui lai)");
    } else {
        /* Ban neu con trong tam, hoac tien gan hon */
        if(dist <= 8){
            do_attack(self, target, 1, log);
        } else {
            step_toward(self, target, all, n);
        }
    }
}

void ai_boss(Actor *self, Actor *all, int n, void (*log)(const char*)){
    /* Boss: 2 attacks/turn (multiattack), hon lo melee + ranged */
    Actor *target = find_target(self, all, n);
    if(!target) return;
    int dist = abs(self->x - target->x) + abs(self->y - target->y);
    /* Multiattack: 2 lan */
    for(int a = 0; a < 2; a++){
        if(actor_is_dead(target)) break;
        if(dist <= 1){
            do_attack(self, target, 0, log);  /* melee */
        } else if(dist <= 8){
            do_attack(self, target, 1, log);  /* ranged */
        } else {
            step_toward(self, target, all, n);
            break;
        }
    }
}
