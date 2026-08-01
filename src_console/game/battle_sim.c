/* =====================================================================
   BATTLE SIMULATOR - Chay console, HIEN DICE ROLLS.
   Hero (Fighter) vs Dragon - turn-based combat co in tung roll.
   KHONG render graphics, chi printf. Build: build_sim.bat
   ===================================================================== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "../engine/rng.h"
#include "../structs.h"
#include "../enums.h"
#include "actor.h"
#include "d20.h"
#include "combat.h"
#include "conditions.h"
#include "spell_resolve.h"
#include "../data/monsters.h"

RNG g_r;   /* global (ai.c reference, KHONG dung trong sim nay) */

/* Helper: in damage dice chi tiet "[5+3+6=14]" hoac "[5]=5" */
static void print_dice(DamageDetail d){
    printf("[");
    for(int i = 0; i < d.n_rolls; i++){
        printf("%d", d.rolls[i]);
        if(i < d.n_rolls - 1) printf("+");
    }
    printf("]");
    if(d.mod != 0){
        if(d.mod > 0) printf("+%d", d.mod);
        else printf("%d", d.mod);
    }
    printf(" = %d", d.total);
    if(d.crit) printf(" (CRIT x2 dice!)");
}

/* Helper: in 1 attack action voi day du dice */
static void do_attack_print(Actor *src, Actor *tgt, int action_idx, RNG *rng){
    const MonsterAction *atk = &src->type->actions[action_idx];
    int is_ranged = (atk->range_max > 0);
    Ability ab = is_ranged ? AB_DEX : AB_STR;
    int mod = actor_ability_mod(src->type->scores[ab]);

    printf("  %s tan cong %s bang %s:\n", src->type->name, tgt->type->name, atk->name);

    /* d20 attack roll */
    D20Result roll = d20_roll(mod + atk->atk_bonus, ROLL_NORMAL, rng);
    int target_ac = actor_effective_ac(tgt, cond_ac_bonus(tgt));
    printf("    d20: rolled %d (raw %d%s) + atk %+d = %d  vs AC %d\n",
           roll.die, roll.die,
           roll.nat20 ? " NAT20!" : roll.nat1 ? " NAT1!" : "",
           mod + atk->atk_bonus, roll.total, target_ac);

    if(roll.nat1){
        printf("    -> FUMBLE! Hut te.\n\n");
        return;
    }
    int hit = roll.nat20 || (roll.total >= target_ac);
    if(!hit){
        printf("    -> HUT (AC %d > %d)\n\n", target_ac, roll.total);
        return;
    }
    /* Damage roll */
    DamageDetail dmg = d20_roll_damage_detail(atk->damage, roll.nat20, rng);
    printf("    -> %s! Damage: ", roll.nat20 ? "CRIT HIT" : "HIT");
    print_dice(dmg);
    printf("\n");
    actor_take_damage(tgt, dmg.total);
    printf("    %s HP: %d/%d\n\n", tgt->type->name, tgt->hp, tgt->max_hp);
}

/* Helper: in HP bar */
static void print_hp_bar(const Actor *a){
    printf("  %s: [", a->type->name);
    int w = 30;
    int fill = a->hp * w / a->max_hp;
    if(fill > w) fill = w;
    for(int i = 0; i < w; i++) printf(i < fill ? "=" : ".");
    printf("] %d/%d\n", a->hp, a->max_hp);
}

int main(void){
    rng_seed(&g_r, (uint64_t)GetTickCount());

    /* Spawn: Hero (level 5 fighter, buff cho demo) vs Dragon (giam HP) */
    Actor hero = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &g_r);
    hero.max_hp = 60; hero.hp = 60;   /* level 5 fighter, buff demo */
    Actor dragon = actor_spawn(&MONSTERS[ID_DRAGON], 0, 0, TEAM_ENEMY, &g_r);
    dragon.max_hp = 50; dragon.hp = 50;   /* giam HP de demo lau hon */

    printf("============================================================\n");
    printf("  BATTLE SIMULATOR - D&D 5e Dice Rolls\n");
    printf("============================================================\n");
    printf("  HERO   (Fighter Lv3): HP %d, AC %d, STR %d (%+d)\n",
           hero.hp, hero.type->ac, hero.type->scores[AB_STR],
           actor_ability_mod(hero.type->scores[AB_STR]));
    printf("  DRAGON (Boss):       HP %d, AC %d\n\n",
           dragon.hp, dragon.type->ac);

    int round = 1;
    int turn = 0;   /* 0 = hero, 1 = dragon */

    while(!actor_is_dead(&hero) && !actor_is_dead(&dragon)){
        printf("---- ROUND %d - %s TURN ----\n", round, turn==0 ? "HERO" : "DRAGON");
        print_hp_bar(&hero);
        print_hp_bar(&dragon);
        printf("\n");

        if(turn == 0){
            /* Hero turn: 2 attacks (multiattack fighter lv5-ish), + use potion neu thap HP */
            if(hero.hp < 10){
                printf("  HERO uong potion (heal 2d4+2):\n    ");
                DamageDetail heal;
                heal.rolls[0] = rng_range(&g_r, 1, 4);
                heal.rolls[1] = rng_range(&g_r, 1, 4);
                heal.n_rolls = 2; heal.mod = 2;
                heal.total = heal.rolls[0] + heal.rolls[1] + 2;
                print_dice(heal);
                actor_heal(&hero, heal.total);
                printf("\n    HERO HP: %d/%d\n\n", hero.hp, hero.max_hp);
            } else {
                do_attack_print(&hero, &dragon, 0, &g_r);  /* Longsword */
            }
        } else {
            /* Dragon turn: multiattack (2x) - Bite + Fire Breath */
            do_attack_print(&dragon, &hero, 0, &g_r);   /* Bite (2d10+6) */
            if(!actor_is_dead(&hero)){
                do_attack_print(&dragon, &hero, 1, &g_r);  /* Fire Breath */
            }
        }

        /* Resolve conditions (DOT) */
        char dot_buf[128];
        Actor *current = turn == 0 ? &hero : &dragon;
        int expired = cond_resolve_turn(current, &g_r, dot_buf, sizeof(dot_buf));
        if(dot_buf[0]) printf("  [EFFECT] %s (expired %d)\n\n", dot_buf, expired);

        turn = 1 - turn;
        if(turn == 0) round++;
        Sleep(400);   /* tam de doc output */
    }

    printf("============================================================\n");
    if(actor_is_dead(&dragon)){
        printf("  HERO THANG! (Dragon down sau %d rounds)\n", round);
    } else {
        printf("  DRAGON THANG! (Hero down sau %d rounds)\n", round);
    }
    printf("============================================================\n");
    printf("\nFinal state:\n");
    print_hp_bar(&hero);
    print_hp_bar(&dragon);
    return 0;
}
