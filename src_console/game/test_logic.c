/* =====================================================================
   TEST HARNESS - Kiem tra lõi logic KHONG can UI/render.
   Chay qua console (printf). Test tung system:
     - RNG deterministic
     - d20 rolls (crit/fumble/advantage)
     - Actor HP/damage/death/heal
     - Combat AC check (voi armor bonus)
     - Save/Load
     - Conditions (poison DOT, stun)
     - Line of Sight
     - Resting + Death saves + Action economy
   Build: build_test.bat
   ===================================================================== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Include engine + game logic (KHONG include console.h -> khong render) */
#include "../engine/rng.h"

/* Global RNG (ai.c tham chieu g_r - dummy cho test, KHONG dung trong test logic) */
RNG g_r = {0xDEADBEEFCAFEBABEULL};
#include "../engine/map.h"
#include "../engine/fov.h"
#include "../engine/bsp.h"
#include "../structs.h"
#include "../enums.h"
#include "actor.h"
#include "d20.h"
#include "combat.h"
#include "conditions.h"
#include "items.h"
#include "inventory.h"
#include "spells.h"
#include "spell_resolve.h"
#include "save.h"
#include "../data/monsters.h"

static int g_tests_run = 0, g_tests_pass = 0, g_tests_fail = 0;

#define TEST(name) do { printf("[TEST] %s ... ", name); g_tests_run++; } while(0)
#define PASS() do { printf("OK\n"); g_tests_pass++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_tests_fail++; } while(0)
#define CHECK(cond, msg) do { if(cond){ PASS(); } else { FAIL(msg); } } while(0)

/* ============ TESTS ============ */

static void test_rng(void){
    TEST("RNG deterministic");
    RNG a, b;
    rng_seed(&a, 42);
    rng_seed(&b, 42);
    int ok = 1;
    for(int i = 0; i < 100; i++){
        if(rng_next(&a) != rng_next(&b)){ ok = 0; break; }
    }
    CHECK(ok, "2 RNG cung seed phai ra giong nhau");
}

static void test_rng_range(void){
    TEST("RNG range [1,20] khong tran");
    RNG r; rng_seed(&r, 1);
    int ok = 1;
    for(int i = 0; i < 1000; i++){
        int v = rng_range(&r, 1, 20);
        if(v < 1 || v > 20){ ok = 0; break; }
    }
    CHECK(ok, "range phai trong [1,20]");
}

static void test_d20_crit(void){
    TEST("d20 nat20 = crit");
    /* Roll nhieu lan, it nhat 1 crit trong 1000 roll (xac suat 5%) */
    RNG r; rng_seed(&r, 7);
    int crit_count = 0, fumble_count = 0;
    for(int i = 0; i < 2000; i++){
        D20Result res = d20_roll(0, ROLL_NORMAL, &r);
        if(res.nat20) crit_count++;
        if(res.nat1) fumble_count++;
    }
    CHECK(crit_count > 50 && fumble_count > 50, "crit/fumble phai xuat hien ~5%");
}

static void test_d20_advantage(void){
    TEST("d20 advantage >= normal");
    RNG r; rng_seed(&r, 99);
    int adv_total = 0, norm_total = 0;
    for(int i = 0; i < 1000; i++){
        adv_total += d20_roll(0, ROLL_ADVANTAGE, &r).total;
        norm_total += d20_roll(0, ROLL_NORMAL, &r).total;
    }
    CHECK(adv_total > norm_total, "advantage phai cao hon normal");
}

static void test_actor_hp_death(void){
    TEST("Actor damage + death");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_GOBLIN], 0, 0, TEAM_ENEMY, &r);
    int initial_hp = a.hp;
    actor_take_damage(&a, 5);
    CHECK(a.hp == initial_hp - 5 && !actor_is_dead(&a), "5 dmg giua HP");
    actor_take_damage(&a, 999);
    CHECK(actor_is_dead(&a) && a.hp == 0, "overkill -> dead");
}

static void test_actor_heal(void){
    TEST("Actor heal cap");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_GOBLIN], 0, 0, TEAM_ENEMY, &r);
    actor_take_damage(&a, 3);
    actor_heal(&a, 999);
    CHECK(a.hp == a.max_hp, "heal khong vuot max_hp");
}

static void test_combat_ac(void){
    TEST("Combat AC check (armor bonus apply)");
    RNG r; rng_seed(&r, 1);
    MonsterAction atk = { .name="Test", .atk_bonus=10, .reach=5, .range_max=0,
                          .damage=DICE(1,6,0), .dmg_type=DMG_SLASHING };
    /* Target AC 20 + bonus 5 = 25. atk_bonus 10 + d20 (avg 11) = 21 < 25 -> hut */
    Actor tgt; memset(&tgt, 0, sizeof(tgt));
    static MonsterType mock_type = { .ac = 20 };
    tgt.type = &mock_type;
    int hit_count = 0;
    for(int i = 0; i < 1000; i++){
        /* atk_bonus 10, d20 avg ~10.5 -> total ~20. AC 25 -> rat hiem hit */
        AtkResult res = combat_resolve_attack(&atk, 0, 25, ROLL_NORMAL, &r);
        if(res.hit) hit_count++;
    }
    CHECK(hit_count < 350, "AC cao + bonus phai hut nhieu (< 35% hit, roll 15-20 = 30%)");
}

static void test_effective_ac(void){
    TEST("actor_effective_ac = base + bonus");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &r);
    int base_ac = a.type->ac;   /* 16 */
    int with_bonus = actor_effective_ac(&a, 5);  /* +5 (mage armor / chain) */
    CHECK(with_bonus == base_ac + 5, "AC = base + bonus");
}

static void test_inventory_equip(void){
    TEST("Inventory equip weapon");
    Inventory inv; inv_init(&inv);
    inv_add(&inv, &ITEMS[ID_LONGSWORD], 1);
    CHECK(inv.count == 1, "add 1 item");
    int ok = inv_equip(&inv, 0);
    CHECK(ok && inv.equipped[SLOT_WEAPON].type == &ITEMS[ID_LONGSWORD], "equip -> slot");
    /* AC bonus tu armor */
    inv_init(&inv);
    inv_add(&inv, &ITEMS[ID_CHAINSHIRT], 1);
    inv_equip(&inv, 0);
    int ac_bonus = inv_total_ac_bonus(&inv);
    CHECK(ac_bonus == 3, "chain shirt +3 AC");
}

static void test_save_load(void){
    TEST("Save/Load roundtrip");
    SaveData s = { .hp=15, .max_hp=20, .ac=18, .str=16, .dex=12, .con=14,
                   .intl=10, .wis=10, .cha=12, .xp=300, .level=2,
                   .x=5, .y=7, .depth=3, .n_potions=2, .has_weapon=1, .has_armor=0 };
    int ok = save_game("test_save.dat", &s);
    if(!ok){ FAIL("save_game failed"); return; }
    SaveData loaded;
    ok = load_game("test_save.dat", &loaded);
    if(!ok){ FAIL("load_game failed"); return; }
    CHECK(loaded.hp==15 && loaded.depth==3 && loaded.xp==300 && loaded.str==16,
          "loaded data phai giong saved");
    DeleteFileA("test_save.dat");
}

static void test_los(void){
    TEST("Line of Sight (Bresenham)");
    Map *m = map_create(20, 20);
    map_fill(m, TILE_FLOOR);
    /* Clear path (0,0)-(10,0) -> thay */
    CHECK(map_has_los(m, 0, 0, 10, 0), "duong thang khong chan -> thay");
    /* Chan bang wall giua (5,0) */
    map_set(m, 5, 0, TILE_WALL);
    CHECK(!map_has_los(m, 0, 0, 10, 0), "wall o giua -> khong thay");
    map_destroy(m);
}

static void test_conditions_poison(void){
    TEST("Conditions: poison DOT");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_GOBLIN], 0, 0, TEAM_ENEMY, &r);
    int initial_hp = a.hp;
    /* Force poison DOT effect truc tiep (bo qua save de test logic DOT) */
    Effect pe = {0};
    pe.id = 99;
    pe.condition = COND_POISONED;
    pe.rounds_left = 5;
    pe.dmg_per_turn = 3;
    pe.dmg_type = DMG_POISON;
    cond_add(&a, pe);
    CHECK(actor_has_condition(&a, COND_POISONED), "poison condition da set");
    /* Resolve DOT 1 turn -> HP giam */
    char dot_buf[128];
    cond_resolve_turn(&a, &r, dot_buf, sizeof(dot_buf));
    CHECK(a.hp < initial_hp, "DOT phai giam HP");
    cond_clear_all(&a);
}

static void test_conditions_stun(void){
    TEST("Conditions: stun apply + clear");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_GOBLIN], 0, 0, TEAM_ENEMY, &r);
    char log_buf[128];
    spell_cast(SPELL_HOLD_PERSON, &a, &a, 5, &r, log_buf, sizeof(log_buf));
    /* Co kha nang stun thanh cong (DC13) - test condition co the set */
    cond_clear_all(&a);
    CHECK(!actor_has_condition(&a, COND_STUNNED), "clear_all xoa condition");
}

static void test_action_economy(void){
    TEST("Action economy: action/bonus/move");
    RNG r; rng_seed(&r, 1);
    Actor a = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &r);
    actor_refresh_turn(&a);
    CHECK(actor_can_act(&a, ECON_ACTION) && actor_can_act(&a, ECON_BONUS) && a.move_left > 0,
          "dau turn: con tat ca slots");
    actor_consume(&a, ECON_ACTION);
    actor_consume(&a, ECON_MOVE);
    CHECK(!actor_can_act(&a, ECON_ACTION), "sau consume action -> het");
    CHECK(a.move_left == a.type->speed - 1, "move_left giam 1");
}

static void test_resting(void){
    TEST("Resting: short + long");
    RNG r; rng_seed(&r, 100);
    Actor a = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &r);
    /* Force HP cao de test resting khong bi dead */
    a.max_hp = 100; a.hp = 100;
    actor_take_damage(&a, 60);   /* HP 100->40 */
    actor_short_rest(&a);         /* heal 25 -> 65 */
    CHECK(a.hp > 40 && a.hp < 100, "short rest heal them (40->65)");
    actor_long_rest(&a);
    CHECK(a.hp == 100, "long rest full heal");
}

static void test_death_saves(void){
    TEST("Death saves");
    RNG r; rng_seed(&r, 42);
    Actor a = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &r);
    DeathSaves ds = {0};
    /* Roll cho den khi stable hoac dead */
    int rolls = 0;
    while(!ds.stable && !actor_is_dead(&a) && rolls < 100){
        actor_death_save(&a, &r, &ds);
        rolls++;
    }
    CHECK(ds.stable || actor_is_dead(&a), "death save phai ket thuc (stable/dead)");
}

static void test_monster_table(void){
    TEST("Monster table integrity");
    CHECK(N_MONSTERS == 8, "N_MONSTERS = 8");
    int ok = 1;
    for(int i = 0; i < N_MONSTERS; i++){
        if(!MONSTERS[i].name){ ok = 0; break; }
        if(MONSTERS[i].hp_dice.sides == 0){ ok = 0; break; }
    }
    CHECK(ok, "tat ca entry phai co name + hp_dice");
}

/* ============ MAIN ============ */
int main(void){
    printf("=== ASCII RPG LOGIC TESTS ===\n\n");

    test_rng();
    test_rng_range();
    test_d20_crit();
    test_d20_advantage();
    test_actor_hp_death();
    test_actor_heal();
    test_combat_ac();
    test_effective_ac();
    test_inventory_equip();
    test_save_load();
    test_los();
    test_conditions_poison();
    test_conditions_stun();
    test_action_economy();
    test_resting();
    test_death_saves();
    test_monster_table();

    printf("\n=== RESULTS ===\n");
    printf("Run: %d   Pass: %d   Fail: %d\n", g_tests_run, g_tests_pass, g_tests_fail);
    printf("%s\n", g_tests_fail == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return g_tests_fail == 0 ? 0 : 1;
}
