/* =====================================================================
   PARTY DEMO - DM Narrator + 4 Hero Party (LMoP intro).
   Console app (nhu battle_sim): demo engine that thuc.
   Flow: Narration intro -> 4 hero pre-made -> passive Perception check
         -> goblin ambush -> roll initiative (engine) -> 1 combat round.
   DUNG ENGINE: rng/actor/d20/turn/combat + MONSTERS[ID_GOBLIN].
   ===================================================================== */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "../engine/rng.h"
#include "../structs.h"
#include "../enums.h"
#include "../engine/console.h"   /* CE_GREEN, CE_YEL, CE_CYAN, CE_RED */
#include "actor.h"
#include "d20.h"
#include "combat.h"
#include "turn.h"
#include "../data/monsters.h"

RNG g_r;   /* global RNG (ai.c reference) */

/* =====================================================================
   LOGGER - mọi printf cũng ghi ra party_demo.log (không cần chép tay)
   ===================================================================== */
static FILE *g_log = NULL;
/* Override printf: ghi cả console + file. Macro giữ nguyên tên printf
   để code dưới không phải sửa từng dòng. */
#undef printf
#define printf log_printf
static int log_printf(const char *fmt, ...){
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fputs(buf, stdout);          /* console */
    fflush(stdout);
    if(g_log){                   /* file log */
        fputs(buf, g_log);
        fflush(g_log);
    }
    return n;
}

/* =====================================================================
   PHẦN 1: LỜI THOẠI DM (data-only) — PHẦN CHÍNH ĐO KB
   Intro LMoP: Gundren Rockseeker thuê party hộ tống xe bò tới Phandalin.
   ===================================================================== */
static const char *NARRATION_INTRO[] = {
    "═══════════════════════════════════════════════════════════",
    "DM: Thành phố Neverwinter, bên bờ biển sóng vỗ.",
    "DM: Tại quán trọ Đồng Xuôi Vàng, một người lùn râu đỏ ngồi đợi các ngươi.",
    "DM: Hắn tên Gundren Rockseeker — một thợ mỏ thăm dò kỳ cựu.",
    "DM: Gundren nói: Ta tìm thấy rồi! Một lối vào mỏ cổ... Wave Echo Cave!",
    "DM: Các ngươi hãy hộ tống xe bò chở đồ tiếp tế tới Phandalin cho ta.",
    "DM: Ta sẽ đi trước ngựa. Gặp lại ở tiệm Barthen khi các ngươi tới.",
    "DM: Phần thưởng: 10 đồng vàng mỗi người. Khoản đặt cọc 2 gp đã trao.",
    "DM: Đội các ngươi gật đầu đồng ý. Cuộc phiêu lưu bắt đầu.",
    "DM: ··· Ba ngày đi trên con đường đất Triboar Trail ···",
    "DM: Buổi sáng thứ tư. Rừng im lặng một cách bất thường.",
    "DM: Không tiếng chim. Không gió lay cành. Chỉ có bánh xe bò kẽo kẹt.",
    "DM: Bỗng nhiên — các ngươi thấy hai con ngựa chết chắn ngang đường.",
    "DM: Chúng mang yên ghế của Neverwinter. Đây là ngựa của Gundren!",
    "DM: Cảnh tượng quá đáng ngờ. Rừng nín thở.",
    "═══════════════════════════════════════════════════════════",
};
#define N_INTRO  (int)(sizeof(NARRATION_INTRO)/sizeof(NARRATION_INTRO[0]))

/* Chuỗi DM trong combat / passive check */
static const char *NARRATION_AMBUSH[] = {
    "DM: Từ trong bụi rậm, bóng nhỏ xanh lao ra — GOBLIN!",
    "DM: Bốn con goblin phục kích! Rút vũ khí, mau!",
    "DM: ··· Cuộc chiến bắt đầu ···",
};
#define N_AMBUSH  (int)(sizeof(NARRATION_AMBUSH)/sizeof(NARRATION_AMBUSH[0]))

/* =====================================================================
   PHẦN 2: 4 HERO PRE-MADE (data-only, dùng DSL STATS/DICE)
   Stats theo D&D 5e Standard Array (15,14,13,12,10,8).
   ===================================================================== */

/* --- Fighter: Longsword melee --- */
static const MonsterAction fighter_actions[] = {
    { .name="Kiếm dài", .atk_bonus=2, .reach=5, .range_max=0,
      .damage=DICE(1,8,3), .dmg_type=DMG_SLASHING },
};
static const MonsterType HERO_FIGHTER = {
    .name="Chiến binh", .size=SIZE_MEDIUM, .type=TYPE_HUMANOID,
    .ac=16, .hp_dice=DICE(1,10,3), .speed=6,
    .scores=STATS(STR(16), DEX(12), CON(14), INT(8),  WIS(10), CHA(10)),
    .cr=CR(0), .xp=0, .glyph='F', .glyph_color=CE_RED,
    .actions=fighter_actions, .n_actions=1, .ai=NULL,
};

/* --- Cleric: Mace melee + heal (đơn giản hóa: chỉ mace) --- */
static const MonsterAction cleric_actions[] = {
    { .name="Chùy", .atk_bonus=2, .reach=5, .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_BLUDGEONING },
};
static const MonsterType HERO_CLERIC = {
    .name="Tu sĩ", .size=SIZE_MEDIUM, .type=TYPE_HUMANOID,
    .ac=16, .hp_dice=DICE(1,8,2), .speed=6,
    .scores=STATS(STR(14), DEX(10), CON(13), INT(11), WIS(16), CHA(12)),
    .cr=CR(0), .xp=0, .glyph='C', .glyph_color=CE_GREEN,
    .actions=cleric_actions, .n_actions=1, .ai=NULL,
};

/* --- Rogue: Shortsword melee, DEX-based --- */
static const MonsterAction rogue_actions[] = {
    { .name="Kiếm ngắn", .atk_bonus=2, .reach=5, .range_max=0,
      .damage=DICE(1,6,3), .dmg_type=DMG_PIERCING },
};
static const MonsterType HERO_ROGUE = {
    .name="Trộm", .size=SIZE_MEDIUM, .type=TYPE_HUMANOID,
    .ac=14, .hp_dice=DICE(1,8,2), .speed=6,
    .scores=STATS(STR(9),  DEX(16), CON(14), INT(12), WIS(10), CHA(13)),
    .cr=CR(0), .xp=0, .glyph='R', .glyph_color=CE_YEL,
    .actions=rogue_actions, .n_actions=1, .ai=NULL,
};

/* --- Wizard: Fire Bolt cantrip (ranged) --- */
static const MonsterAction wizard_actions[] = {
    { .name="Mũi lửa", .atk_bonus=2, .reach=120, .range_max=0,
      .damage=DICE(1,10,0), .dmg_type=DMG_FIRE },
};
static const MonsterType HERO_WIZARD = {
    .name="Pháp sư", .size=SIZE_MEDIUM, .type=TYPE_HUMANOID,
    .ac=11, .hp_dice=DICE(1,6,1), .speed=6,
    .scores=STATS(STR(8),  DEX(14), CON(12), INT(16), WIS(12), CHA(10)),
    .cr=CR(0), .xp=0, .glyph='W', .glyph_color=CE_CYAN,
    .actions=wizard_actions, .n_actions=1, .ai=NULL,
};

/* =====================================================================
   PHẦN 3: CONSOLE HELPERS (copy pattern tu battle_sim.c)
   ===================================================================== */

/* Đợi người dùng nhấn PHÍM BẤT KỲ (any key). */
static void wait_key(const char *prompt){
    printf("%s", prompt);
    fflush(stdout);
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode;
    GetConsoleMode(hin, &oldMode);
    SetConsoleMode(hin, oldMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
    INPUT_RECORD ir; DWORD n;
    while(ReadConsoleInputA(hin, &ir, 1, &n)){
        if(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown){
            break;   /* bất kỳ phím nào */
        }
    }
    SetConsoleMode(hin, oldMode);
    printf("\r%*s\r", (int)strlen(prompt) + 2, "");   /* xóa prompt */
}

/* In đường phân cách. */
static void print_sep(const char *title){
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════\n");
}

/* In 1 dòng dialogue DM với viền dễ theo dõi.
   Format:
     ┌──────────────────────────────────────┐
     │ DM: <nội dung>                        │
     └──────────────────────────────────────┘ */
static void print_dm(const char *line){
    /* Bo trong khung, nội dung giới hạn 52 ký tự cho dễ đọc */
    printf("  ┌──────────────────────────────────────────────────────┐\n");
    /* Nếu chuỗi dài, vẫn in nguyên (không cắt) */
    printf("  │ %s\n", line);
    printf("  └──────────────────────────────────────────────────────┘\n");
}

/* Animation tung d20: số nhấp nháy ~500ms rồi hiện final. */
static void anim_roll_d20(int final, RNG *rng){
    int frames = 8;
    for(int i = 0; i < frames; i++){
        int fake = rng_range(rng, 1, 20);
        printf("\r    [d20 đang lăn...  %2d  ]                ", fake);
        fflush(stdout);
        Sleep(60);
    }
    printf("\r    [d20  ==>  %2d  ]                \n", final);
    fflush(stdout);
}

/* In damage dice chi tiết: [5+3+6=14] hoặc [5]=5. */
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
    if(d.crit) printf("  (CHÍ MẠNG ×2 xúc_xắc!)");
}

/* In HP bar cho 1 actor. */
static void print_hp_bar(const Actor *a){
    printf("  %-10s [", a->type->name);
    int w = 20;
    int fill = (a->max_hp > 0) ? a->hp * w / a->max_hp : 0;
    if(fill > w) fill = w;
    for(int i = 0; i < w; i++) printf(i < fill ? "=" : ".");
    printf("] %d/%d\n", a->hp, a->max_hp);
}

/* In character sheet gọn cho 1 hero. */
static void print_hero_sheet(const Actor *a){
    printf("  ┌─ %s ─┐\n", a->type->name);
    printf("  │ HP %d/%d   AC %d\n", a->hp, a->max_hp, a->type->ac);
    int s = a->type->scores[AB_STR], d = a->type->scores[AB_DEX],
        c = a->type->scores[AB_CON], i = a->type->scores[AB_INT],
        w = a->type->scores[AB_WIS], ch = a->type->scores[AB_CHA];
    printf("  │ S%c %d(%+d) M%c %d(%+d)  V%c %d(%+d)\n",
           ' ', s, actor_ability_mod(s), ' ', d, actor_ability_mod(d), ' ', c, actor_ability_mod(c));
    printf("  │ T%c %d(%+d) N%c %d(%+d)  U%c %d(%+d)\n",
           ' ', i, actor_ability_mod(i), ' ', w, actor_ability_mod(w), ' ', ch, actor_ability_mod(ch));
    printf("  │ Vũ khí: %s\n", a->type->actions[0].name);
    printf("  └────────────────┘\n");
}

/* =====================================================================
   PHẦN 4: MAIN FLOW
   ===================================================================== */
int main(void){
    /* UTF-8 cho tiếng Việt có dấu */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    rng_seed(&g_r, (uint64_t)GetTickCount());

    /* Mở file log (mọi printf sẽ ghi ra đây) */
    g_log = fopen("party_demo.log", "w");
    if(g_log){
        /* Ghi BOM UTF-8 để file mở đúng trong Notepad */
        fputs("\xEF\xBB\xBF", g_log);
    }
    printf("=== PARTY DEMO LOG — %lu ===\n\n", (unsigned long)GetTickCount());

    /* ===== STEP 1: NARRATION INTRO ===== */
    print_sep("PHẦN MỞ ĐẦU — LOST MINE OF PHANDELVER");
    for(int i = 0; i < N_INTRO; i++){
        /* Bỏ 2 dòng separator ASCII art (dòng đầu & cuối của mảng) */
        if(strstr(NARRATION_INTRO[i], "═══")) continue;
        print_dm(NARRATION_INTRO[i]);
        if(i < N_INTRO - 1){
            wait_key("  [phím bất kỳ để tiếp tục...] ");
        }
    }
    printf("\n");

    /* ===== STEP 2: GIỚI THIỆU PARTY 4 HERO ===== */
    print_sep("ĐỘI HÙNG CỦA BẠN");
    Actor party[4];
    party[0] = actor_spawn(&HERO_FIGHTER, 0, 0, TEAM_PLAYER, &g_r);
    party[1] = actor_spawn(&HERO_CLERIC,  1, 0, TEAM_PLAYER, &g_r);
    party[2] = actor_spawn(&HERO_ROGUE,   2, 0, TEAM_PLAYER, &g_r);
    party[3] = actor_spawn(&HERO_WIZARD,  3, 0, TEAM_PLAYER, &g_r);
    for(int i = 0; i < 4; i++) print_hero_sheet(&party[i]);
    wait_key("\n  [phím bất kỳ để tiếp tục...] ");

    /* ===== STEP 3: PASSIVE PERCEPTION CHECK ===== */
    print_sep("KIỂM TRA NHẬN THỨC (Perception)");
    print_dm("DM: Con đường im lặng. Các ngươi có cảm thấy gì không?");
    printf("\n");
    int dc = 12;   /* Goblin ambush DC (D&D 5e goblin Stealth +6) */
    for(int i = 0; i < 4; i++){
        int wis = party[i].type->scores[AB_WIS];
        int mod = actor_ability_mod(wis);
        int passive = 10 + mod;   /* 5e passive Perception = 10 + WIS mod */
        printf("  %s: Nhận thức thụ động %d  (DC %d)\n", party[i].type->name, passive, dc);
        if(passive >= dc){
            printf("    -> PHÁT HIỆN! Bóng nấp trong bụi.\n\n");
        } else {
            /* Roll active Perception */
            D20Result r = d20_roll(mod, ROLL_NORMAL, &g_r);
            wait_key("    [phím bất kỳ để tung d20...] ");
            anim_roll_d20(r.die, &g_r);
            printf("    d20 %d + WIS %+d = %d   %s\n\n",
                   r.die, mod, r.total,
                   r.total >= dc ? "-> PHÁT HIỆN!" : "-> Không thấy gì.");
        }
    }

    /* ===== STEP 4: GOBLIN AMBUSH + INITIATIVE ===== */
    for(int i = 0; i < N_AMBUSH; i++) print_dm(NARRATION_AMBUSH[i]);
    wait_key("\n  [phím bất kỳ để tung Initiative...] ");

    /* Setup 4 goblin enemy (dùng MONSTERS[ID_GOBLIN] từ engine) */
    Actor enemies[4];
    for(int i = 0; i < 4; i++){
        enemies[i] = actor_spawn(&MONSTERS[ID_GOBLIN], 0, 0, TEAM_ENEMY, &g_r);
    }

    /* Merge party + enemy vào combat[] cho turn system */
    Actor combat[8];
    for(int i = 0; i < 4; i++) combat[i] = party[i];
    for(int i = 0; i < 4; i++) combat[4+i] = enemies[i];

    /* ENGINE: roll initiative (d20 + DEX mod, sort giảm_dần) */
    turn_roll_initiative(combat, 8, &g_r);

    printf("\n  Thứ tự khởi tiết (Initiative):\n");
    /* In theo thứ tự turn system (không phải thứ tự mảng) */
    /* turn.c dùng static g_order, ta phải duyệt qua turn_current/advance */
    printf("  ··· (xem trong combat round dưới đây) ···\n\n");

    /* ===== STEP 5: 1 COMBAT ROUND ===== */
    print_sep("ROUND 1");
    for(int step = 0; step < 8; step++){
        Actor *cur = turn_current(combat, 8);
        if(!cur || actor_is_dead(cur)){
            turn_advance(combat, 8);
            continue;
        }
        const char *team = (cur->team == TEAM_PLAYER) ? "P" : "E";
        printf("  [%s] Lượt của %s (init %d):\n", team, cur->type->name, cur->initiative);

        /* Chọn target: team kia, còn sống */
        Actor *target = NULL;
        Team enemy_team = (cur->team == TEAM_PLAYER) ? TEAM_ENEMY : TEAM_PLAYER;
        for(int j = 0; j < 8; j++){
            if(combat[j].team == enemy_team && !actor_is_dead(&combat[j])){
                target = &combat[j];
                break;
            }
        }
        if(!target){ turn_advance(combat, 8); continue; }

        /* Roll to-hit (engine) */
        const MonsterAction *atk = &cur->type->actions[0];
        int is_ranged = (atk->range_max > 0);
        Ability ab = is_ranged ? AB_DEX : AB_STR;
        int to_hit_mod = actor_ability_mod(cur->type->scores[ab]) + atk->atk_bonus;
        int target_ac = actor_effective_ac(target, 0);
        D20Result roll = d20_roll(to_hit_mod, ROLL_NORMAL, &g_r);

        wait_key("    [phím bất kỳ để tung tấn công...] ");
        anim_roll_d20(roll.die, &g_r);
        printf("    %s đánh %s bằng %s:\n", cur->type->name, target->type->name, atk->name);
        printf("    d20 %d%s + %+d = %d   vs AC %d\n",
               roll.die, roll.nat20 ? " NAT20!" : roll.nat1 ? " NAT1!" : "",
               to_hit_mod, roll.total, target_ac);

        if(roll.nat1){
            printf("    -> LỠ TAY! Hụt.\n\n");
        } else if(roll.nat20 || roll.total >= target_ac){
            printf("    -> %s!\n", roll.nat20 ? "*** CHÍ MẠNG ***" : "TRÚNG ĐÒN");
            DamageDetail dmg = d20_roll_damage_detail(atk->damage, roll.nat20, &g_r);
            wait_key("    [phím bất kỳ để tung sát thương...] ");
            printf("    Sát_thương: ");
            print_dice(dmg);
            printf("\n");
            combat_apply_damage(target, dmg.total);
            printf("    %s HP: %d/%d\n\n", target->type->name, target->hp, target->max_hp);
        } else {
            printf("    -> HỤT (AC %d > %d)\n\n", target_ac, roll.total);
        }
        turn_advance(combat, 8);
    }

    /* ===== KẾT QUẢ SAU ROUND 1 ===== */
    print_sep("KẾT QUẢ SAU ROUND 1");
    printf("  PARTY:\n");
    for(int i = 0; i < 4; i++){
        print_hp_bar(&combat[i]);
    }
    printf("  GOBLIN:\n");
    for(int i = 4; i < 8; i++){
        print_hp_bar(&combat[i]);
    }

    /* ===== THỐNG KÊ SIZE ===== */
    print_sep("THỐNG KÊ DUNG LƯỢNG");
    /* Tính tổng byte của narration strings */
    size_t intro_bytes = 0;
    for(int i = 0; i < N_INTRO; i++) intro_bytes += strlen(NARRATION_INTRO[i]) + 1;
    size_t ambush_bytes = 0;
    for(int i = 0; i < N_AMBUSH; i++) ambush_bytes += strlen(NARRATION_AMBUSH[i]) + 1;
    size_t narration_total = intro_bytes + ambush_bytes;
    size_t hero_types_bytes = sizeof(HERO_FIGHTER) + sizeof(HERO_CLERIC)
                            + sizeof(HERO_ROGUE) + sizeof(HERO_WIZARD);

    printf("  Lời thoại DM intro:     %2d câu, %4zu bytes (%.2f KB)\n",
           N_INTRO, intro_bytes, intro_bytes/1024.0);
    printf("  Lời thoại DM ambush:    %2d câu, %4zu bytes (%.2f KB)\n",
           N_AMBUSH, ambush_bytes, ambush_bytes/1024.0);
    printf("  ----------------------------------------------\n");
    printf("  TỔNG LỜI THOẠI DM:              %4zu bytes (%.2f KB)\n",
           narration_total, narration_total/1024.0);
    printf("  4 Hero MonsterType (data):      %4zu bytes (%.2f KB)\n",
           hero_types_bytes, hero_types_bytes/1024.0);
    printf("\n  → Phần lời thoại dẫn chuyện = %.2f KB\n", narration_total/1024.0);
    printf("  → Ước tính cho ~20 NPC × ~15 dòng = ~%.1f KB (cho full LMoP)\n",
           narration_total/1024.0 * 20.0);

    printf("\n═══ HẾT DEMO. Cảm ơn đã chơi! ═══\n");
    printf("\n  (Log đã lưu vào party_demo.log)\n");
    wait_key("\n  [Bấm phím bất kỳ để thoát...] ");
    if(g_log) fclose(g_log);
    return 0;
}
