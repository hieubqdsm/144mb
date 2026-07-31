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
/* data tables compiled rieng (declarations trong headers) */
#include "../data/monsters.h"
#include "../game/items.h"
#include "../game/spells.h"
#include <stdio.h>
#include <string.h>

/* ---------- Global state ---------- */
RNG g_r;
static Floor g_floor;
static Actor g_all[20];     /* player + monsters, index 0 = player */
static int g_n_actors;
static Inventory g_inv;
static int g_depth = 1;
static int g_player_x, g_player_y;
static int g_game_over = 0;
static int g_in_combat = 0;
static int g_show_inventory = 0;
static int g_ai_delay = 0;  /* frames de giam AI speed */

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
enum { ST_TITLE, ST_CLASS, ST_PLAY, ST_DEAD };
static int g_state = ST_TITLE;
static int g_frame = 0;

/* ---------- Helpers ---------- */
static Actor *player(void){ return &g_all[0]; }
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
    g_all[0] = *player();   /* keep player stats */
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
    g_game_over = 0;
    g_log_n = 0;
    log_add("=== DUNGEON CRAWL START ===");
    start_floor(1);
    g_state = ST_PLAY;
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
    cond_resolve_turn(player(), &g_r, cbuf, sizeof(cbuf));
    if(cbuf[0]) log_add(cbuf);
    /* Check death */
    if(actor_is_dead(player())){
        log_add("*** BAN DA CHET. Press R de restart. ***");
        g_state = ST_DEAD;
    }
}

static void try_move(int dx, int dy){
    int nx = g_player_x + dx, ny = g_player_y + dy;
    Actor *m = monster_at(nx, ny);
    if(m){
        /* Bump = attack */
        const MonsterAction *atk = &player()->type->actions[0];
        int mod = actor_ability_mod(player()->type->scores[AB_STR]) + inv_total_atk_bonus(&g_inv);
        AtkResult r = combat_resolve_attack(atk, mod, m, ROLL_NORMAL, &g_r);
        char buf[80];
        if(r.fumble) sprintf(buf, "Ban tan cong %s: NAT 1 (hut).", m->type->name);
        else if(r.crit){
            combat_apply_damage(m, r.damage);
            sprintf(buf, "CRIT! Ban danh %s %d damage!", m->type->name, r.damage);
        }
        else if(r.hit){
            combat_apply_damage(m, r.damage);
            sprintf(buf, "Ban danh %s %d damage.", m->type->name, r.damage);
        }
        else sprintf(buf, "Ban tan cong %s: hut (AC %d).", m->type->name, m->type->ac);
        log_add(buf);
        if(actor_is_dead(m)){
            sprintf(buf, "%s chet! (+%d XP)", m->type->name, m->type->xp);
            log_add(buf);
            player()->xp += m->type->xp;
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
    ui_actor_panel(ox+2, y, player(), "=== HERO ===", 10);
    y = 12;
    char b[80];
    sprintf(b, "Tang: %d", g_depth); ce_text(ox+2, y++, b, 14);
    sprintf(b, "XP: %d", player()->xp); ce_text(ox+2, y++, b, 11);
    y++;
    ce_text(ox+2, y++, "=== CONTROLS ===", 8);
    ce_text(ox+2, y++, "WASD: move/attack", 7);
    ce_text(ox+2, y++, "1: Fire Bolt", 12);
    ce_text(ox+2, y++, "2: Magic Missile", 11);
    ce_text(ox+2, y++, "H: heal potion", 10);
    ce_text(ox+2, y++, "I: inventory", 14);
    ce_text(ox+2, y++, ">: descend stairs", 11);
}

/* ---------- Title / class select ---------- */
static void draw_title(void){
    ce_clear(1);
    ce_border((WCHAR)0x2593, 9);
    ce_sprite(SCREEN_W/2-15, 6,
        "  ██████╗  ██████╗ ██╗  ██╗    ███████╗███╗   ██╗\n"
        "  ██╔══██╗██╔═══██╗╚██╗██╔╝    ██╔════╝████╗  ██║\n"
        "  ██║  ██║██║   ██║ ╚███╔╝     █████╗  ██╔██╗ ██║\n"
        "  ██║  ██║██║   ██║ ██╔██╗     ██╔══╝  ██║╚██╗██║\n"
        "  ██████╔╝╚██████╔╝██╔╝ ██╗    ███████╗██║ ╚████║\n"
        "  ╚═════╝  ╚═════╝ ╚═╝  ╚═╝    ╚══════╝╚═╝  ╚═══╝", 12);
    ce_text(SCREEN_W/2-10, 14, "ASCII DUNGEON CRAWLER", 8);
    if(ui_button(SCREEN_W/2-8, 22, 16, "> NEW GAME", 10)){
        g_state = ST_CLASS;
    }
    ce_text(SCREEN_W/2-7, SCREEN_H-3, "ESC to quit", 8);
}

static void draw_class(void){
    ce_clear(0);
    ce_text(SCREEN_W/2-9, 3, "CHOOSE YOUR CLASS", 14);
    if(ui_button(SCREEN_W/2-10, 8, 20, "FIGHTER (STR)", 12)){ new_game(0); }
    if(ui_button(SCREEN_W/2-10, 14, 20, "MAGE (INT)", 9)){ new_game(1); }
    ce_text(SCREEN_W/2-10, 22, "Ca 2 deu bat dau voi:", 7);
    ce_text(SCREEN_W/2-12, 24, "- Longsword, 3 Healing Potions", 8);
    ce_text(SCREEN_W/2-9, SCREEN_H-3, "ESC: back", 8);
}

/* ---------- Update ---------- */
void update(float dt){
    g_frame++;
    if(ce_keyPressed(VK_ESCAPE)){
        if(g_state == ST_PLAY){ g_state = ST_TITLE; }
        else if(g_state == ST_TITLE){ ce_quit(); }
        else { g_state = ST_TITLE; }
        return;
    }

    switch(g_state){
        case ST_TITLE:
            draw_title();
            break;
        case ST_CLASS:
            draw_class();
            if(ce_keyPressed(VK_ESCAPE)) g_state = ST_TITLE;
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
                if(t) spell_cast(SPELL_FIRE_BOLT, player(), t, 5, &g_r, buf, sizeof(buf));
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
                if(t) spell_cast(SPELL_MAGIC_MISSILE, player(), t, 5, &g_r, buf, sizeof(buf));
                else strcpy(buf, "Khong co target.");
                log_add(buf);
                player_act_after();
            }
            if(ce_keyPressed('H')){
                char buf[80];
                inv_use_potion(&g_inv, player(), &g_r, buf, sizeof(buf));
                log_add(buf);
                player_act_after();
            }
            if(ce_keyPressed('I')) g_show_inventory = !g_show_inventory;
            if(ce_keyPressed(VK_OEM_2)){  /* '/' or '>' */
                if(map_get(g_floor.map, g_player_x, g_player_y) == TILE_STAIRS_DOWN){
                    log_add("Xuong tang sau...");
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
        case ST_DEAD:
            ce_clear(4);
            ce_text(SCREEN_W/2-5, 20, "YOU DIED", 12);
            sprintf_s(g_log[0], 80, "Reached depth %d, %d XP", g_depth, player()->xp);
            ce_text(SCREEN_W/2-15, 24, g_log[0], 14);
            ce_text(SCREEN_W/2-9, 28, "R: restart | ESC: menu", 8);
            if(ce_keyPressed('R')) g_state = ST_TITLE;
            break;
    }
}

int main(void){
    ce_run(update);
    dungeon_free(&g_floor);
    return 0;
}
