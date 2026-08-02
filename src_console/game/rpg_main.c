/* =====================================================================
   RPG MAIN - ASCII Dungeon Crawler (D&D-lite).
   Ket hop: dungeon procedural + turn-based combat + inventory + spells.
   Flow: Title -> Class select -> Dungeon (move, fight, loot) -> descend.
   Controls:
     WASD/arrows = move    SPACE = attack adjacent    I = inventory
     1 = cast Fire Bolt   2 = cast Magic Missile     H = use potion
     > (on stairs) = descend    R = restart    ESC = menu/quit
   ===================================================================== */
#define SCREEN_W 100
#define SCREEN_H 50
#define FONT_W 8
#define FONT_H 8
#include "../engine/console.h"
#include "../engine/rng.h"
#include "../engine/map.h"
#include "../engine/fov.h"
#include "../engine/bsp.h"
#include "../structs.h"
#include "../enums.h"
#include "actor.h"
#include "d20.h"
#include "combat.h"
#include "turn.h"
#include "conditions.h"
#include "inventory.h"
#include "items.h"
#include "ai.h"
#include "spells.h"
#include "spell_resolve.h"
#include "dungeon.h"
#include "ui.h"
#include "save.h"
#include "i18n.h"
#include "dialogue.h"
/* data tables compiled rieng (declarations trong headers) */
#include "../data/monsters.h"
#include "../game/items.h"
#include "../game/spells.h"
#include <stdio.h>
#include <string.h>

/* ---------- Global state ---------- */
RNG g_r;
/* g_lang defined trong actor.c (shared cho toan bo game).
   i18n.h da extern Lang g_lang; nen khong can redefine o day. */
static Floor g_floor;
static Actor g_all[20];     /* player + monsters, index 0 = player */
static int g_n_actors;
static Inventory g_inv;
static int g_depth = 1;
static int g_player_x, g_player_y;
static int g_game_over = 0;
static int g_in_combat = 0;
static int g_show_inventory = 0;
static int g_ai_delay = 0;
static int g_level = 1;
static int g_player_class = 0;        /* 0 = Fighter, 1 = Mage (remember khi load) */
static char g_feedback[80] = {0};     /* thong bao tam thoi (vd "Da luu!") hien tren title */
static int  g_feedback_frames = 0;    /* dem frame de fade feedback */

/* XP can de len level (D&D 5e simplified: 300, 900, 2700... x3 moi cap) */
static int xp_threshold(int level){
    int t = 300;
    for(int i = 1; i < level; i++) t *= 3;
    return t;
}

/* Forward declarations (C implicit-int rule can cu neu goi truoc dinh nghia) */
static Actor *get_player(void);
static void log_add(const char *msg);
static void player_act_after(void);

/* ---------- SFX procedural (async beep, 0 byte asset) ---------- */
static DWORD WINAPI beep_thread(LPVOID p){
    int *a = (int*)p; Beep(a[0], a[1]); free(a); return 0;
}
static void sfx_beep(int freq, int dur){
    int *a = (int*)malloc(sizeof(int)*2);
    if(!a) return;
    a[0]=freq; a[1]=dur;
    HANDLE h = CreateThread(NULL, 0, beep_thread, a, 0, NULL);
    if(h) CloseHandle(h);
}
static void sfx_hit(void){ sfx_beep(200, 50); }
static void sfx_crit(void){ sfx_beep(600, 40); sfx_beep(900, 40); }
static void sfx_miss(void){ sfx_beep(120, 60); }
static void sfx_die(void){ sfx_beep(150, 200); sfx_beep(100, 300); }
static void sfx_levelup(void){ sfx_beep(523, 80); sfx_beep(659, 80); sfx_beep(784, 120); }
static void sfx_pickup(void){ sfx_beep(800, 30); sfx_beep(1000, 30); }
static void sfx_spell(void){ sfx_beep(440, 60); sfx_beep(880, 60); }
static void sfx_stairs(void){ sfx_beep(300, 100); sfx_beep(200, 100); sfx_beep(100, 150); }

/* Level up: tang max HP (+ fighter 10/lvl), +1 STR moi 4 level. */
static void check_level_up(void){
    Actor *p = get_player();
    while(p->xp >= xp_threshold(g_level) && g_level < 10){
        p->xp -= xp_threshold(g_level);
        g_level++;
        p->max_hp += 10;
        p->hp = p->max_hp;   /* full heal khi level up */
        p->level = (uint8_t)g_level;
        /* +1 STR moi 4 level (ASD) */
        if(g_level % 4 == 0){
            char buf[80];
            sprintf(buf, "*** LEVEL UP %d! +10 HP, +1 STR ***", g_level);
            log_add(buf);
        } else {
            char buf[80];
            sprintf(buf, "*** LEVEL UP %d! +10 HP (full heal) ***", g_level);
            log_add(buf);
        }
        sfx_levelup();
    }
}

/* Combat log */
static char g_log[6][80];
static int g_log_n = 0;
static void log_add(const char *msg){
    if(g_log_n < 6){
        strncpy(g_log[g_log_n], msg, 79); g_log[g_log_n][79]=0; g_log_n++;
    } else {
        for(int i=0;i<5;i++) strncpy(g_log[i], g_log[i+1], 80);
        strncpy(g_log[5], msg, 79); g_log[5][79]=0;
    }
}

/* State machine */
enum { ST_TITLE, ST_CLASS, ST_PLAY, ST_PAUSE, ST_OPTIONS, ST_ABOUT, ST_DEAD, ST_DIALOGUE };
static int g_state = ST_TITLE;
static int g_prev_state = ST_TITLE;   /* de ESC quay lai state truoc (vd Options<-Title) */
static int g_frame = 0;

/* ---------- Helpers ---------- */
static Actor *get_player(void){ return &g_all[0]; }
static Actor *monster_at(int x, int y){
    for(int i=1;i<g_n_actors;i++){
        if(!actor_is_dead(&g_all[i]) && g_all[i].x==x && g_all[i].y==y) return &g_all[i];
    }
    return NULL;
}
static void sync_player_pos(void){
    g_all[0].x = (int8_t)g_player_x;
    g_all[0].y = (int8_t)g_player_y;
}

static void start_floor(int depth){
    g_depth = depth;
    dungeon_free(&g_floor);
    dungeon_generate(&g_floor, depth, &g_r);
    g_player_x = g_floor.player_start_x;
    g_player_y = g_floor.player_start_y;
    /* Build actor list: player + monsters */
    g_all[0] = *get_player();   /* keep player stats */
    g_all[0].x = (int8_t)g_player_x; g_all[0].y = (int8_t)g_player_y;
    g_n_actors = 1;
    for(int i=0;i<g_floor.n_monsters;i++){
        g_all[g_n_actors] = g_floor.monsters[i];
        g_n_actors++;
    }
    fov_compute(g_floor.map, g_player_x, g_player_y, 8);
    char buf[80];
    sprintf(buf, "--- TANG %d ---", depth);
    log_add(buf);
    g_in_combat = 0;
}

static void new_game(int class_idx){
    rng_seed(&g_r, (uint64_t)GetTickCount());
    /* Player = Hero template */
    g_all[0] = actor_spawn(&MONSTERS[ID_PLAYER], 0, 0, TEAM_PLAYER, &g_r);
    if(class_idx == 1){  /* Mage variant */
        g_all[0].type = &MONSTERS[ID_PLAYER];
        g_all[0].hp = g_all[0].max_hp = 16;
    }
    inv_init(&g_inv);
    /* Starting gear */
    inv_add(&g_inv, &ITEMS[ID_LONGSWORD], 1);
    inv_equip(&g_inv, 0);
    inv_add(&g_inv, &ITEMS[ID_HEAL_POTION], 3);
    g_level = 1;
    get_player()->level = 1;
    get_player()->xp = 0;
    g_game_over = 0;
    g_log_n = 0;
    log_add("=== DUNGEON CRAWL START ===");
    start_floor(1);
    g_state = ST_PLAY;
}

/* ---------- Save/Load wire-up ---------- */
/* Gom state hien tai thanh SaveData va luu ra SAVE_PATH.
   Tra ve 1 neu thanh cong. */
static int do_save(void){
    Actor *p = get_player();
    SaveData s = {0};
    s.hp = p->hp; s.max_hp = p->max_hp; s.ac = p->type->ac;
    s.str = p->type->scores[AB_STR]; s.dex = p->type->scores[AB_DEX];
    s.con = p->type->scores[AB_CON]; s.intl = p->type->scores[AB_INT];
    s.wis = p->type->scores[AB_WIS]; s.cha = p->type->scores[AB_CHA];
    s.xp = p->xp; s.level = g_level;
    s.x = g_player_x; s.y = g_player_y;
    s.depth = g_depth;
    s.n_potions = inv_count(&g_inv, ID_HEAL_POTION);
    s.has_weapon = (g_inv.equipped[SLOT_WEAPON].type != NULL) ? 1 : 0;
    s.has_armor  = (g_inv.equipped[SLOT_ARMOR].type  != NULL) ? 1 : 0;
    s.class_idx  = g_player_class;
    s.rng_state  = g_r.state;
    return save_quick(&s);
}

/* Load tu SAVE_PATH, restore actor/inv/rng, regenerate floor (cung depth).
   Tra ve 1 neu thanh cong. */
static int do_load(void){
    SaveData s;
    if(!load_quick(&s)) return 0;
    /* Restore RNG truoc de dungeon_generate deterministic */
    if(s.rng_state == 0) s.rng_state = (uint64_t)GetTickCount() | 1;
    rng_seed(&g_r, s.rng_state);
    g_r.state = s.rng_state;   /* dung y state da save (rng_seed co the xor) */
    /* Player: dung class da luu de set type/HP cho chinh xac */
    g_player_class = s.class_idx;
    g_all[0] = actor_spawn(&MONSTERS[ID_PLAYER], s.x, s.y, TEAM_PLAYER, &g_r);
    if(s.class_idx == 1){ g_all[0].hp = g_all[0].max_hp = 16; }
    else { g_all[0].hp = s.hp; g_all[0].max_hp = s.max_hp; }
    get_player()->xp = (uint16_t)s.xp;
    get_player()->level = (uint8_t)s.level;
    g_level = s.level;
    /* Inventory: longsword equipped + potions + optional armor */
    inv_init(&g_inv);
    inv_add(&g_inv, &ITEMS[ID_LONGSWORD], 1);
    inv_equip(&g_inv, 0);
    inv_add(&g_inv, &ITEMS[ID_HEAL_POTION], s.n_potions);
    g_game_over = 0;
    g_log_n = 0;
    log_add(T(S_CONTINUE));
    start_floor(s.depth);
    /* start_floor dat player tai player_start; override bang vi tri da save */
    g_player_x = s.x; g_player_y = s.y;
    g_all[0].x = (int8_t)s.x; g_all[0].y = (int8_t)s.y;
    fov_compute(g_floor.map, g_player_x, g_player_y, 8);
    return 1;
}

/* Hien feedback tam thoi tren title (vd sau save). */
static void show_feedback(const char *msg){
    strncpy(g_feedback, msg, sizeof(g_feedback)-1);
    g_feedback[sizeof(g_feedback)-1] = 0;
    g_feedback_frames = 180;   /* ~3 giay @ 60fps */
}

/* ---------- Turn resolution ---------- */
static void player_act_after(void){
    /* Sau moi action cua player: monsters act (turn-based real-time-ish cho demo) */
    for(int i=1;i<g_n_actors;i++){
        if(actor_is_dead(&g_all[i])) continue;
        /* Chi monster nhin thay player (visible) moi act */
        int dx = abs(g_all[i].x - g_player_x), dy = abs(g_all[i].y - g_player_y);
        if(dx + dy < 12){
            actor_refresh_turn(&g_all[i]);
            g_all[i].type->ai(&g_all[i], g_all, g_n_actors, log_add);
        }
    }
    fov_compute(g_floor.map, g_player_x, g_player_y, 8);
    /* Resolve conditions dau turn player */
    char cbuf[80];
    cond_resolve_turn(get_player(), &g_r, cbuf, sizeof(cbuf));
    if(cbuf[0]) log_add(cbuf);
    /* Check death */
    if(actor_is_dead(get_player())){
        log_add("*** BAN DA CHET. Press R de restart. ***");
        sfx_die();
        g_state = ST_DEAD;
    }
}

static void try_move(int dx, int dy){
    int nx = g_player_x + dx, ny = g_player_y + dy;
    Actor *m = monster_at(nx, ny);
    if(m){
        /* Bump = attack */
        const MonsterAction *atk = &get_player()->type->actions[0];
        int mod = actor_ability_mod(get_player()->type->scores[AB_STR]) + inv_total_atk_bonus(&g_inv);
        int target_ac = actor_effective_ac(m, cond_ac_bonus(m));
        AtkResult r = combat_resolve_attack(atk, mod, target_ac, ROLL_NORMAL, &g_r);
        char buf[80];
        if(r.fumble){ sprintf(buf, "Ban tan cong %s: NAT 1 (hut).", m->type->name); sfx_miss(); }
        else if(r.crit){
            combat_apply_damage(m, r.damage);
            sprintf(buf, "CRIT! Ban danh %s %d damage!", m->type->name, r.damage);
            sfx_crit();
        }
        else if(r.hit){
            combat_apply_damage(m, r.damage);
            sprintf(buf, "Ban danh %s %d damage.", m->type->name, r.damage);
            sfx_hit();
        }
        else { sprintf(buf, "Ban tan cong %s: hut (AC %d).", m->type->name, m->type->ac); sfx_miss(); }
        log_add(buf);
        if(actor_is_dead(m)){
            sprintf(buf, "%s chet! (+%d XP)", m->type->name, m->type->xp);
            log_add(buf);
            get_player()->xp += m->type->xp;
            sfx_die();
            check_level_up();
        }
        player_act_after();
        return;
    }
    if(map_walkable(g_floor.map, nx, ny)){
        g_player_x = nx; g_player_y = ny;
        sync_player_pos();
        /* Pickup items */
        for(int i=0;i<g_floor.n_floor_items;i++){
            if(g_floor.floor_items[i].x==nx && g_floor.floor_items[i].y==ny){
                inv_add(&g_inv, g_floor.floor_items[i].type, g_floor.floor_items[i].qty);
                char buf[80];
                sprintf(buf, "Nhat: %s x%d", g_floor.floor_items[i].type->name, g_floor.floor_items[i].qty);
                log_add(buf);
                sfx_pickup();
                /* remove */
                g_floor.floor_items[i] = g_floor.floor_items[--g_floor.n_floor_items];
                break;
            }
        }
        /* Stairs? */
        if(map_get(g_floor.map, nx, ny) == TILE_STAIRS_DOWN){
            log_add("> Cau thang xuong! (press > de descend)");
        }
        player_act_after();
    }
}

/* ---------- Render ---------- */
static void draw_map(int ox, int oy){
    for(int y=0;y<g_floor.map->h;y++){
        for(int x=0;x<g_floor.map->w;x++){
            if(!map_seen(g_floor.map, x, y)) continue;
            int vis = map_visible(g_floor.map, x, y);
            TileType t = map_get(g_floor.map, x, y);
            const TileDef *td = &TILE_DEFS[t];
            int col = vis ? td->fg : 8;
            WCHAR ch = td->glyph;
            /* Floor items */
            for(int i=0;i<g_floor.n_floor_items;i++){
                if(g_floor.floor_items[i].x==x && g_floor.floor_items[i].y==y){
                    ch = g_floor.floor_items[i].type->glyph;
                    col = vis ? g_floor.floor_items[i].type->glyph_color : 8;
                }
            }
            /* Actors */
            if(vis){
                for(int a=0;a<g_n_actors;a++){
                    if(actor_is_dead(&g_all[a])) continue;
                    if(g_all[a].x==x && g_all[a].y==y){
                        ch = actor_glyph(&g_all[a]);
                        col = actor_color(&g_all[a]);
                    }
                }
            }
            ce_putc(ox+x, oy+y, ch, col);
        }
    }
}

static void draw_sidebar(int ox){
    int y = 1;
    ui_actor_panel(ox+2, y, get_player(), T(S_HERO), 10);
    y = 12;
    char b[80];
    sprintf(b, T(S_LEVEL_FMT), g_level); ce_text(ox+2, y++, b, 14);
    sprintf(b, "XP: %d / %d", get_player()->xp, xp_threshold(g_level)); ce_text(ox+2, y++, b, 11);
    sprintf(b, T(S_DEPTH_FMT), g_depth); ce_text(ox+2, y++, b, 13);
    y++;
    /* Dem monster con song */
    int alive = 0;
    for(int i=1;i<g_n_actors;i++) if(!actor_is_dead(&g_all[i])) alive++;
    sprintf(b, T(S_MONSTERS_FMT), alive); ce_text(ox+2, y++, b, 12);
    /* Potions */
    int pot = inv_count(&g_inv, ID_HEAL_POTION);
    sprintf(b, T(S_POTIONS_FMT), pot); ce_text(ox+2, y++, b, 10);
    y++;
    ce_text(ox+2, y++, T(S_CONTROLS), 8);
    ce_text(ox+2, y++, T(S_CTRL_MOVE), 7);
    ce_text(ox+2, y++, T(S_CTRL_SPELL1), 12);
    ce_text(ox+2, y++, T(S_CTRL_SPELL2), 11);
    ce_text(ox+2, y++, T(S_CTRL_POTION), 10);
    ce_text(ox+2, y++, T(S_CTRL_INV), 14);
    ce_text(ox+2, y++, T(S_CTRL_STAIRS), 11);
}

/* ---------- Title / class select / pause / options / about ---------- */
static void draw_title(void){
    ce_clear(1);
    ce_border((WCHAR)0x2593, 9);
    /* Title ASCII art dung ky tu thuong (ASCII art chi dung '#' - khong loi UTF-8) */
    ce_sprite(SCREEN_W/2-17, 6,
        "  ####  ####  #   #  #####   ###    ####  #   #\n"
        " #     #      ## ##  #        #    #      #   #\n"
        " #     ####   # # #  ####     #    #      #####\n"
        " #     #      #   #  #        #    #      #   #\n"
        "  #### ####   #   #  #       ###    ####  #   #", 14);
    ce_text(SCREEN_W/2-10, 14, T(S_TITLE), 11);
    ce_text(SCREEN_W/2-13, 16, T(S_SUBTITLE), 8);

    /* Buttons: Continue (chi khi co save) + New + Options + Quit.
       Layout auto can giua theo so nut. */
    int has_save = save_quick_exists();
    int n_btn = has_save ? 4 : 3;
    int btn_w = 22;
    int btn_x = SCREEN_W/2 - btn_w/2;
    int gap = 5;
    /* Y bat dau sao cho block nut can giua vung [20..SCREEN_H-6] */
    int total_h = n_btn * 3 + (n_btn-1) * (gap-3);
    int y0 = 20 + ((SCREEN_H - 6 - 20) - total_h) / 2;
    int by = y0;

    if(has_save){
        if(ui_button(btn_x, by, btn_w, T(S_CONTINUE), 10)){
            if(do_load()){
                show_feedback(T(S_CONTINUE));
                g_state = ST_PLAY;
            } else {
                show_feedback(T(S_LOAD_FAIL));
            }
        }
        by += gap;
    }
    if(ui_button(btn_x, by, btn_w, T(S_NEW_GAME), 11)){ g_state = ST_CLASS; }
    by += gap;
    if(ui_button(btn_x, by, btn_w, T(S_OPTIONS), 13)){
        g_prev_state = ST_TITLE;
        g_state = ST_OPTIONS;
    }
    by += gap;
    if(ui_button(btn_x, by, btn_w, T(S_QUIT), 12)){ ce_quit(); }

    /* Keyboard shortcuts: phím 1-N chọn menu nhanh (ngoài click chuột) */
    {
        int key_idx = -1;
        if(ce_keyPressed('1')) key_idx = 0;
        else if(ce_keyPressed('2')) key_idx = 1;
        else if(ce_keyPressed('3')) key_idx = 2;
        else if(ce_keyPressed('4')) key_idx = 3;
        if(key_idx >= 0){
            if(has_save){
                if(key_idx == 0){
                    if(do_load()){ show_feedback(T(S_CONTINUE)); g_state = ST_PLAY; }
                    else show_feedback(T(S_LOAD_FAIL));
                }
                else if(key_idx == 1) g_state = ST_CLASS;
                else if(key_idx == 2){ g_prev_state = ST_TITLE; g_state = ST_OPTIONS; }
                else if(key_idx == 3) ce_quit();
            } else {
                if(key_idx == 0) g_state = ST_CLASS;
                else if(key_idx == 1){ g_prev_state = ST_TITLE; g_state = ST_OPTIONS; }
                else if(key_idx == 2) ce_quit();
            }
        }
    }

    /* Feedback tam thoi (sau save/load) */
    if(g_feedback_frames > 0){
        ce_text(SCREEN_W/2-20, SCREEN_H-5, g_feedback, 14);
    }
    /* Hint phia duoi */
    ce_text(SCREEN_W/2-30, SCREEN_H-3, "Click chuot hoac nhan 1-2-3 de chon menu", 8);
    ce_text(SCREEN_W/2-18, SCREEN_H-2, T(S_LANG_HINT), 8);
    ce_text(SCREEN_W/2-7, SCREEN_H-1, T(S_ESC_HINT), 8);
    /* Version goc phai */
    ce_text(SCREEN_W-9, SCREEN_H-1, T(S_VERSION), 8);

    /* Phim L: doi ngon ngu */
    if(ce_keyPressed('L')) i18n_cycle();
}

static void draw_class(void){
    ce_clear(0);
    ce_text(SCREEN_W/2-13, 3, T(S_CHOOSE_CLASS), 14);
    if(ui_button(SCREEN_W/2-13, 8, 26, T(S_FIGHTER), 12)){ g_player_class = 0; new_game(0); }
    if(ui_button(SCREEN_W/2-13, 14, 26, T(S_MAGE), 9)){ g_player_class = 1; new_game(1); }
    ce_text(SCREEN_W/2-15, 22, T(S_CLASS_START), 7);
    ce_text(SCREEN_W/2-20, 24, T(S_CLASS_GEAR), 8);
    ce_text(SCREEN_W/2-20, SCREEN_H-5, "Click hoac nhan 1=Fighter, 2=Mage", 8);
    ce_text(SCREEN_W/2-9, SCREEN_H-3, T(S_BACK), 8);
    /* Keyboard shortcuts */
    if(ce_keyPressed('1')){ g_player_class = 0; new_game(0); }
    if(ce_keyPressed('2')){ g_player_class = 1; new_game(1); }
}

/* Pause overlay: ve LEN tren man hinh play (KHONG clear). */
static void draw_pause(void){
    /* Box nen dam de che man hinh play */
    int bw = 28, bh = 11;
    int bx = SCREEN_W/2 - bw/2;
    int by = SCREEN_H/2 - bh/2;
    ce_fillbg(bx, by, bx+bw, by+bh, 4);   /* nen do dam */
    /* Border */
    for(int i=0;i<bw;i++){
        ce_putc(bx+i, by, (WCHAR)0x2550, 14);
        ce_putc(bx+i, by+bh-1, (WCHAR)0x2550, 14);
    }
    for(int i=0;i<bh;i++){
        ce_putc(bx, by+i, (WCHAR)0x2551, 14);
        ce_putc(bx+bw-1, by+i, (WCHAR)0x2551, 14);
    }
    ce_putc(bx, by, (WCHAR)0x2554, 14);            /* goc tren trai */
    ce_putc(bx+bw-1, by, (WCHAR)0x2557, 14);       /* tren phai */
    ce_putc(bx, by+bh-1, (WCHAR)0x255A, 14);       /* duoi trai */
    ce_putc(bx+bw-1, by+bh-1, (WCHAR)0x255D, 14);  /* duoi phai */

    int title_len = (int)strlen(T(S_PAUSE_TITLE));
    ce_text(SCREEN_W/2 - title_len/2, by+1, T(S_PAUSE_TITLE), 14);

    int bwx = 20, bxx = SCREEN_W/2 - bwx/2;
    if(ui_button(bxx, by+3, bwx, T(S_RESUME), 10)){ g_state = ST_PLAY; }
    if(ui_button(bxx, by+7, bwx, T(S_SAVE_QUIT), 11)){
        if(do_save()) show_feedback(T(S_SAVE_OK));
        else show_feedback(T(S_SAVE_FAIL));
        g_state = ST_TITLE;
    }
    if(ui_button(bxx, by+11, bwx, T(S_QUIT_NOSAVE), 12)){
        g_state = ST_TITLE;
    }
}

static void draw_options(void){
    ce_clear(0);
    ce_border((WCHAR)0x2593, 9);
    int title_len = (g_lang == LANG_VI) ? (int)strlen(T(S_OPTIONS)) : (int)strlen(T(S_OPTIONS));
    ce_text(SCREEN_W/2 - title_len/2, 4, T(S_OPTIONS), 14);

    /* Ngon ngu hien tai */
    ce_text(SCREEN_W/2-10, 12, T(S_LANG_LABEL), 11);
    const char *cur = (g_lang == LANG_VI) ? T(S_LANG_VI) : T(S_LANG_EN);
    ce_text(SCREEN_W/2-10, 14, cur, 14);

    /* 2 nut: toggle ngon ngu + About */
    if(ui_button(SCREEN_W/2-12, 18, 24, T(S_LANG_HINT), 13)) i18n_cycle();
    if(ui_button(SCREEN_W/2-12, 24, 24, T(S_ABOUT), 11)){
        g_prev_state = ST_OPTIONS;
        g_state = ST_ABOUT;
    }
    if(ui_button(SCREEN_W/2-12, 30, 24, T(S_BACK), 10)){
        g_state = g_prev_state;
    }
    ce_text(SCREEN_W/2-7, SCREEN_H-3, T(S_ESC_HINT), 8);
    if(ce_keyPressed('L')) i18n_cycle();
}

static void draw_about(void){
    ce_clear(0);
    ce_border((WCHAR)0x2593, 9);
    int title_len = (int)strlen(T(S_ABOUT));
    ce_text(SCREEN_W/2 - title_len/2, 4, T(S_ABOUT), 14);

    int len2 = (int)strlen(T(S_ABOUT_LINE1));
    ce_text(SCREEN_W/2 - len2/2, 12, T(S_ABOUT_LINE1), 11);
    int len3 = (int)strlen(T(S_ABOUT_LINE2));
    ce_text(SCREEN_W/2 - len3/2, 15, T(S_ABOUT_LINE2), 8);
    int len4 = (int)strlen(T(S_ABOUT_LINE3));
    ce_text(SCREEN_W/2 - len4/2, 17, T(S_ABOUT_LINE3), 7);

    ce_text(SCREEN_W/2-13, 21, T(S_VERSION), 13);

    if(ui_button(SCREEN_W/2-8, 28, 16, T(S_BACK), 10)){
        g_state = g_prev_state;
    }
    ce_text(SCREEN_W/2-7, SCREEN_H-3, T(S_ESC_HINT), 8);
}

/* ---------- Update ---------- */
void update(float dt){
    g_frame++;
    /* Fade feedback counter */
    if(g_feedback_frames > 0) g_feedback_frames--;

    if(ce_keyPressed(VK_ESCAPE)){
        switch(g_state){
            case ST_PLAY:    g_state = ST_PAUSE;  break;  /* pause, KHONG mat game */
            case ST_PAUSE:   g_state = ST_PLAY;   break;
            case ST_OPTIONS: g_state = g_prev_state; break;  /* quay lai title hoac pause */
            case ST_ABOUT:   g_state = g_prev_state; break;
            case ST_DIALOGUE: g_state = g_prev_state; dlg_close(); break;  /* dismiss dialogue */
            case ST_CLASS:   g_state = ST_TITLE;  break;
            case ST_DEAD:    g_state = ST_TITLE;  break;
            case ST_TITLE:   ce_quit();           break;
        }
        return;
    }

    switch(g_state){
        case ST_TITLE:
            draw_title();
            break;
        case ST_CLASS:
            draw_class();
            break;
        case ST_PLAY: {
            ce_clear(0);
            /* Input */
            if(ce_keyPressed('W')||ce_keyPressed(VK_UP))    try_move(0,-1);
            if(ce_keyPressed('S')||ce_keyPressed(VK_DOWN))  try_move(0, 1);
            if(ce_keyPressed('A')||ce_keyPressed(VK_LEFT))  try_move(-1,0);
            if(ce_keyPressed('D')||ce_keyPressed(VK_RIGHT)) try_move(1, 0);
            /* Spells */
            if(ce_keyPressed('1')){
                Actor *t = monster_at(g_player_x, g_player_y);  /* TODO: target select */
                /* nearest monster trong FOV */
                if(!t){
                    int bd=999;
                    for(int i=1;i<g_n_actors;i++){
                        if(actor_is_dead(&g_all[i])) continue;
                        if(!map_visible(g_floor.map, g_all[i].x, g_all[i].y)) continue;
                        int d=abs(g_all[i].x-g_player_x)+abs(g_all[i].y-g_player_y);
                        if(d<bd){bd=d; t=&g_all[i];}
                    }
                }
                char buf[80];
                if(t){ spell_cast(SPELL_FIRE_BOLT, get_player(), t, 5, &g_r, buf, sizeof(buf)); sfx_spell(); }
                else strcpy(buf, "Khong co target trong tam nhin.");
                log_add(buf);
                player_act_after();
            }
            if(ce_keyPressed('2')){
                Actor *t=NULL; int bd=999;
                for(int i=1;i<g_n_actors;i++){
                    if(actor_is_dead(&g_all[i])) continue;
                    if(!map_visible(g_floor.map, g_all[i].x, g_all[i].y)) continue;
                    int d=abs(g_all[i].x-g_player_x)+abs(g_all[i].y-g_player_y);
                    if(d<bd){bd=d; t=&g_all[i];}
                }
                char buf[80];
                if(t) spell_cast(SPELL_MAGIC_MISSILE, get_player(), t, 5, &g_r, buf, sizeof(buf));
                else strcpy(buf, "Khong co target.");
                log_add(buf);
                player_act_after();
            }
            if(ce_keyPressed('H')){
                char buf[80];
                inv_use_potion(&g_inv, get_player(), &g_r, buf, sizeof(buf));
                log_add(buf);
                player_act_after();
            }
            if(ce_keyPressed('I')) g_show_inventory = !g_show_inventory;
            /* TEST: phím T = mở dialogue kế tiếp (demo cycle). Sau này thay = interact NPC */
            if(ce_keyPressed('T')){
                static int dlg_test_idx = DLG_DM_INTRO;
                if(dlg_test_idx >= DLG_COUNT) dlg_test_idx = DLG_DM_INTRO;
                if(dlg_test_idx == DLG_NONE) dlg_test_idx = DLG_DM_INTRO;
                g_prev_state = ST_PLAY;
                dlg_start(DIALOGUES[dlg_test_idx]);
                g_state = ST_DIALOGUE;
                dlg_test_idx++;
                if(dlg_test_idx >= DLG_COUNT) dlg_test_idx = DLG_DM_INTRO;
            }
            if(ce_keyPressed(VK_OEM_2)){  /* '/' or '>' */
                if(map_get(g_floor.map, g_player_x, g_player_y) == TILE_STAIRS_DOWN){
                    log_add("Xuong tang sau...");
                    sfx_stairs();
                    start_floor(g_depth + 1);
                }
            }
            /* Render */
            fov_compute(g_floor.map, g_player_x, g_player_y, 8);
            draw_map(2, 2);
            /* Sidebar */
            int sbx = 2 + g_floor.map->w + 1;
            for(int y=0;y<SCREEN_H;y++) ce_putc(sbx, y, (WCHAR)0x2551, 8);
            draw_sidebar(sbx);
            /* Log */
            const char *lines[6];
            for(int i=0;i<g_log_n;i++) lines[i]=g_log[i];
            ui_log(2, 2 + g_floor.map->h + 1, lines, g_log_n, 6);
            /* Inventory overlay */
            if(g_show_inventory){
                ui_inventory(sbx+2, 30, &g_inv, 0);
            }
            break;
        }
        case ST_PAUSE: {
            /* Ve lai man hinh play (KHONG input) roi overlay pause len tren */
            fov_compute(g_floor.map, g_player_x, g_player_y, 8);
            draw_map(2, 2);
            int sbx = 2 + g_floor.map->w + 1;
            for(int y=0;y<SCREEN_H;y++) ce_putc(sbx, y, (WCHAR)0x2551, 8);
            draw_sidebar(sbx);
            const char *lines[6];
            for(int i=0;i<g_log_n;i++) lines[i]=g_log[i];
            ui_log(2, 2 + g_floor.map->h + 1, lines, g_log_n, 6);
            /* Overlay pause */
            draw_pause();
            break;
        }
        case ST_DIALOGUE: {
            /* Ve lai man hinh play (KHONG input) roi overlay dialogue len tren */
            fov_compute(g_floor.map, g_player_x, g_player_y, 8);
            draw_map(2, 2);
            int sbx = 2 + g_floor.map->w + 1;
            for(int y=0;y<SCREEN_H;y++) ce_putc(sbx, y, (WCHAR)0x2551, 8);
            draw_sidebar(sbx);
            const char *lines[6];
            for(int i=0;i<g_log_n;i++) lines[i]=g_log[i];
            ui_log(2, 2 + g_floor.map->h + 1, lines, g_log_n, 6);
            /* Update + render dialogue overlay */
            dlg_update(dt);
            /* Box căn giữa màn hình, phía dưới */
            int box_x = (SCREEN_W - 70) / 2;   /* BOX_W=70 */
            int box_y = SCREEN_H - 12;          /* gần đáy */
            dlg_render(box_x, box_y);
            /* Nếu dialogue đóng (hết dòng) → quay lại play */
            if(!dlg_active()) g_state = g_prev_state;
            break;
        }
        case ST_OPTIONS:
            draw_options();
            break;
        case ST_ABOUT:
            draw_about();
            break;
        case ST_DEAD: {
            ce_clear(4);
            int dl = (int)strlen(T(S_YOU_DIED));
            ce_text(SCREEN_W/2 - dl/2, 20, T(S_YOU_DIED), 12);
            char dbuf[80];
            int pxp = g_all[0].xp;
            /* Hien depth/level/xp (dung format rieng de tranh nested %s chua %d) */
            sprintf(dbuf, "%s %d - %s %d - %d XP",
                    (g_lang==LANG_VI)?"Tang":"Depth", g_depth,
                    (g_lang==LANG_VI)?"Cap":"Lvl", g_level, pxp);
            int dlen = (int)strlen(dbuf);
            ce_text(SCREEN_W/2 - dlen/2, 24, dbuf, 14);
            int rh = (int)strlen(T(S_RESTART_HINT));
            ce_text(SCREEN_W/2 - rh/2, 28, T(S_RESTART_HINT), 8);
            if(ce_keyPressed('R')) g_state = ST_TITLE;
            break;
        }
    }
}

int main(void){
    ce_run(update);
    dungeon_free(&g_floor);
    return 0;
}
