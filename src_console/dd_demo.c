/* =====================================================================
   D&D ASCII DEMO - Proof of concept cho 1.44MB
   - Character creation: click chuột chọn class (Warrior/Mage/Rogue)
   - Dungeon procedural sinh runtime
   - Stats: HP/MP/STR/INT/DEX
   - Click chuột để di chuyển (point & click)
   ===================================================================== */
#define SCREEN_W 100
#define SCREEN_H 50
#define FONT_W 8
#define FONT_H 8
#include "engine/console.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* CP437 / Unicode glyphs */
#define T_WALL    (WCHAR)0x2593   /* ▓ tuong */
#define T_FLOOR   (WCHAR)0x00B7   /* · san */
#define T_DOOR    (WCHAR)0x0044   /* D cua */
#define T_PLAYER  (WCHAR)0x02DA   /* ° player (hoi quang) */
#define T_GOLD    (WCHAR)0x000A   /* $ vang */
#define T_ENEMY   (WCHAR)0x000E   /* enemy */
#define T_STAIRS  (WCHAR)0x003E   /* > cau thang xuong */

#define MAP_W 60
#define MAP_H 40

static char map[MAP_H][MAP_W+1];  /* 0=empty, 1=wall, 2=floor, 3=door */
static int px=30, py=20;          /* player pos */
static int clickTargetX=-1, clickTargetY=-1;

/* Player stats */
typedef struct {
    int hp, maxhp, mp, maxmp;
    int str, intl, dex;
    int gold;
    int level, xp;
    const char *className;
    int classCol;
} Player;
static Player player;

/* State */
enum { ST_TITLE, ST_CLASS, ST_PLAY, ST_DEAD };
static int state = ST_TITLE;

/* ---------- Procedural dungeon (BSP rooms + corridors) ---------- */
static void genDungeon(void){
    /* Fill to walls */
    for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++) map[y][x] = 1;
    /* Tao 8 phong ngau nhien */
    int rooms = 0, attempts = 0;
    int lastX=-1, lastY=-1;
    while(rooms < 8 && attempts < 50){
        attempts++;
        int rw = 5 + rand()%8;
        int rh = 4 + rand()%5;
        int rx = 1 + rand()%(MAP_W - rw - 2);
        int ry = 1 + rand()%(MAP_H - rh - 2);
        /* Don phong: khong trung bat ky phong da co (don gian: kiem tra goc) */
        /* Don carver: floor */
        for(int y=ry;y<ry+rh;y++) for(int x=rx;x<rx+rw;x++) map[y][x] = 2;
        /* Noi phong truoc = corridor (L-shape) */
        if(lastX >= 0){
            int cx = lastX, cy = lastY;
            /* di ngang */
            while(cx != rx+rw/2){ if(map[cy][cx]==1) map[cy][cx]=2; cx += (cx<rx+rw/2)?1:-1; }
            /* di doc */
            while(cy != ry+rh/2){ if(map[cy][cx]==1) map[cy][cx]=2; cy += (cy<ry+rh/2)?1:-1; }
        }
        lastX = rx+rw/2; lastY = ry+rh/2;
        rooms++;
        /* Dat player o phong dau */
        if(rooms==1){ px = rx+rw/2; py = ry+rh/2; }
        /* Dat enemy/gold o phong sau */
        if(rooms > 1 && rand()%2==0){
            int ex = rx + rand()%rw, ey = ry + rand()%rh;
            map[ey][ex] = 4;  /* enemy */
        }
        if(rooms > 1 && rand()%3==0){
            int gx = rx + rand()%rw, gy = ry + rand()%rh;
            if(map[gy][gx]==2) map[gy][gx] = 5;  /* gold */
        }
    }
    /* Dat cau thang xuong o phong cuoi */
    if(lastX>=0 && lastY>=0) map[lastY][lastX] = 6;
}

/* ---------- Render map ---------- */
static void drawMap(int ox, int oy){
    for(int y=0;y<MAP_H;y++){
        for(int x=0;x<MAP_W;x++){
            WCHAR ch; int col;
            switch(map[y][x]){
                case 1: ch=T_WALL;  col=CE_DGREY; break;
                case 2: ch=T_FLOOR; col=CE_DGREY; break;
                case 3: ch=T_DOOR;  col=CE_YEL;   break;
                case 4: ch='r';     col=CE_RED;   break;   /* rat enemy */
                case 5: ch='$';     col=CE_YEL;   break;   /* gold */
                case 6: ch='>';     col=CE_CYAN;  break;   /* stairs */
                default: ch=' ';    col=CE_BLACK;
            }
            ce_putc(ox+x, oy+y, ch, col);
        }
    }
    /* Player */
    ce_putc(ox+px, oy+py, '@', CE_GREEN);
    /* Click target marker */
    if(clickTargetX>=0){
        ce_putc(ox+clickTargetX, oy+clickTargetY, 'X', CE_CYAN);
    }
}

/* ---------- Sidebar (stats) ---------- */
static void drawSidebar(int ox, int oy){
    ce_fillbg(ox, 0, SCREEN_W, SCREEN_H, CE_BLACK);
    ce_border((WCHAR)0x2592, CE_DBLUE);
    /* Vertical separator */
    for(int y=0;y<SCREEN_H;y++) ce_putc(ox, y, (WCHAR)0x2502, CE_DGREY);  /* │ */

    int y = 1;
    ce_text(ox+2, y++, "= HERO =", CE_YEL); y++;
    char b[64];
    sprintf(b, "%s  Lv%d", player.className, player.level); ce_text(ox+2, y++, b, player.classCol);
    y++;
    /* HP bar */
    ce_text(ox+2, y, "HP", CE_RED);
    int hplen = 20;
    int hpfill = player.hp * hplen / player.maxhp;
    for(int i=0;i<hplen;i++) ce_putc(ox+6+i, y, i<hpfill?(WCHAR)0x2588:'.', i<hpfill?CE_RED:CE_DRED);
    sprintf(b, "%d/%d", player.hp, player.maxhp); ce_text(ox+6+hplen+1, y, b, CE_WHITE);
    y+=2;
    /* MP bar */
    ce_text(ox+2, y, "MP", CE_BLUE);
    int mpfill = player.mp * hplen / player.maxmp;
    for(int i=0;i<hplen;i++) ce_putc(ox+6+i, y, i<mpfill?(WCHAR)0x2588:'.', i<mpfill?CE_BLUE:CE_DBLUE);
    sprintf(b, "%d/%d", player.mp, player.maxmp); ce_text(ox+6+hplen+1, y, b, CE_WHITE);
    y+=2;
    /* Stats */
    sprintf(b, "STR %d  INT %d", player.str, player.intl); ce_text(ox+2, y++, b, CE_WHITE);
    sprintf(b, "DEX %d  GOLD %d", player.dex, player.gold); ce_text(ox+2, y++, b, CE_YEL);
    y++;
    /* Hints */
    ce_text(ox+2, y++, "Click map to move", CE_GREY);
    ce_text(ox+2, y++, "> stairs = next lvl", CE_GREY);
    ce_text(ox+2, y++, "$ gold", CE_GREY);
    ce_text(ox+2, y++, "r enemy", CE_GREY);
}

/* ---------- Click button on title ---------- */
static void drawTitle(void){
    ce_clear(CE_BLACK);
    /* ASCII logo "D&D" lon */
    ce_sprite(SCREEN_W/2-15, 4,
        "  ██████╗  ██████╗ ██╗  ██╗    ███████╗███╗   ██╗ ██████╗ ██╗  ██╗\n"
        "  ██╔══██╗██╔═══██╗╚██╗██╔╝    ██╔════╝████╗  ██║██╔═══██╗╚██╗██╔╝\n"
        "  ██║  ██║██║   ██║ ╚███╔╝     ███████╗██╔██╗ ██║██║   ██║ ╚███╔╝ \n"
        "  ██║  ██║██║   ██║ ██╔██╗     ╚════██║██║╚██╗██║██║   ██║ ██╔██╗ \n"
        "  ██████╔╝╚██████╔╝██╔╝ ██╗    ███████║██║ ╚████║╚██████╔╝██╔╝ ██╗\n"
        "  ╚═════╝  ╚═════╝ ╚═╝  ╚═╝    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝", CE_RED);
    ce_text(SCREEN_W/2-9, 13, "ASCII DUNGEON CRAWLER", CE_GREY);
    ce_text(SCREEN_W/2-12, 15, "A 1.44MB proof-of-concept", CE_DGREY);
    /* Play button */
    int bx=SCREEN_W/2-8, by=25, bw=16, bh=3;
    int hover = ce_hoverBox(bx, by, bx+bw, by+bh);
    ce_fillbg(bx, by, bx+bw, by+bh, hover?CE_DGREEN:CE_DGREY);
    ce_putc(bx,by,(WCHAR)0x2554,CE_WHITE); ce_putc(bx+bw-1,by,(WCHAR)0x2557,CE_WHITE);
    ce_putc(bx,by+bh-1,(WCHAR)0x255A,CE_WHITE); ce_putc(bx+bw-1,by+bh-1,(WCHAR)0x255D,CE_WHITE);
    ce_text(bx+4, by+1, "> NEW GAME", hover?CE_WHITE:CE_GREY);
    if(ce_clickedBox(bx,by,bx+bw,by+bh)){ state=ST_CLASS; }
    ce_text(SCREEN_W/2-7, SCREEN_H-3, "ESC to quit", CE_DGREY);
}

/* ---------- Class selection ---------- */
static void drawClassSelect(void){
    ce_clear(CE_BLACK);
    ce_text(SCREEN_W/2-9, 3, "CHOOSE YOUR CLASS", CE_YEL);
    /* 3 box class */
    const char *names[3] = { "WARRIOR", "MAGE", "ROGUE" };
    const char *desc[3] = { "High STR, HP", "High INT, MP", "High DEX, crit" };
    int cols[3] = { CE_RED, CE_BLUE, CE_GREEN };
    int bw=20, bh=10;
    int spacing = 4;
    int totalW = 3*bw + 2*spacing;
    int x0 = SCREEN_W/2 - totalW/2;
    for(int i=0;i<3;i++){
        int bx = x0 + i*(bw+spacing);
        int by = 8;
        int hover = ce_hoverBox(bx, by, bx+bw, by+bh);
        ce_fillbg(bx, by, bx+bw, by+bh, hover?CE_DGREY:CE_BLACK);
        for(int x=bx;x<bx+bw;x++){ ce_putc(x,by,(WCHAR)0x2550,cols[i]); ce_putc(x,by+bh-1,(WCHAR)0x2550,cols[i]); }
        for(int y=by;y<by+bh;y++){ ce_putc(bx,y,(WCHAR)0x2551,cols[i]); ce_putc(bx+bw-1,y,(WCHAR)0x2551,cols[i]); }
        ce_putc(bx,by,(WCHAR)0x2554,cols[i]); ce_putc(bx+bw-1,by,(WCHAR)0x2557,cols[i]);
        ce_putc(bx,by+bh-1,(WCHAR)0x255A,cols[i]); ce_putc(bx+bw-1,by+bh-1,(WCHAR)0x255D,cols[i]);
        ce_text(bx + bw/2 - strlen(names[i])/2, by+2, names[i], cols[i]);
        ce_text(bx + bw/2 - strlen(desc[i])/2, by+5, desc[i], CE_WHITE);
        /* Stats preview */
        if(i==0) ce_text(bx+2, by+7, "STR:18 HP:30", CE_RED);
        if(i==1) ce_text(bx+2, by+7, "INT:18 MP:20", CE_BLUE);
        if(i==2) ce_text(bx+2, by+7, "DEX:18 crit", CE_GREEN);
        if(ce_clickedBox(bx,by,bx+bw,by+bh)){
            switch(i){
                case 0: player.hp=30;player.maxhp=30;player.mp=5;player.maxmp=5;player.str=18;player.intl=8;player.dex=10;player.className="Warrior";player.classCol=CE_RED; break;
                case 1: player.hp=18;player.maxhp=18;player.mp=20;player.maxmp=20;player.str=6;player.intl=18;player.dex=10;player.className="Mage";player.classCol=CE_BLUE; break;
                case 2: player.hp=22;player.maxhp=22;player.mp=8;player.maxmp=8;player.str=10;player.intl=10;player.dex=18;player.className="Rogue";player.classCol=CE_GREEN; break;
            }
            player.gold=0; player.level=1; player.xp=0;
            genDungeon();
            state=ST_PLAY;
        }
    }
    ce_text(SCREEN_W/2-13, SCREEN_H-3, "Click a class to begin", CE_GREY);
}

/* ---------- Play update ---------- */
static void updatePlay(float dt){
    /* Click map = move target */
    if(ce_mouseClicked(0)){
        int mx = ce_mouseX - 0;  /* map offset 0,0 */
        int my = ce_mouseY - 0;
        if(mx>=0 && mx<MAP_W && my>=0 && my<MAP_H && map[my][mx]!=1){
            clickTargetX = mx; clickTargetY = my;
        }
    }
    /* Move toward target (1 step/frame, walkable) */
    if(clickTargetX>=0){
        if(px < clickTargetX && map[py][px+1]!=1) px++;
        else if(px > clickTargetX && map[py][px-1]!=1) px--;
        else if(py < clickTargetY && map[py+1][px]!=1) py++;
        else if(py > clickTargetY && map[py-1][px]!=1) py--;
        if(px==clickTargetX && py==clickTargetY) clickTargetX=-1;
    }
    /* Keyboard fallback */
    if(ce_keyPressed('W')||ce_keyPressed(VK_UP))    { if(py>0&&map[py-1][px]!=1) py--; clickTargetX=-1; }
    if(ce_keyPressed('S')||ce_keyPressed(VK_DOWN))  { if(py<MAP_H-1&&map[py+1][px]!=1) py++; clickTargetX=-1; }
    if(ce_keyPressed('A')||ce_keyPressed(VK_LEFT))  { if(px>0&&map[py][px-1]!=1) px--; clickTargetX=-1; }
    if(ce_keyPressed('D')||ce_keyPressed(VK_RIGHT)) { if(px<MAP_W-1&&map[py][px+1]!=1) px++; clickTargetX=-1; }

    /* Interact tile */
    if(map[py][px]==5){ player.gold += 5+rand()%10; map[py][px]=2; }    /* gold */
    if(map[py][px]==4){ /* enemy - don don combat */
        int dmg = player.str/2 + rand()%4;
        player.hp -= 3;
        map[py][px]=2;
        player.xp += 5;
    }
    if(map[py][px]==6){ genDungeon(); }  /* next level */

    if(player.hp<=0) state=ST_DEAD;
}

/* ---------- Main ---------- */
void update(float dt){
    switch(state){
        case ST_TITLE:
            drawTitle();
            if(ce_keyPressed(VK_ESCAPE)) ce_quit();
            break;
        case ST_CLASS:
            drawClassSelect();
            if(ce_keyPressed(VK_ESCAPE)) state=ST_TITLE;
            break;
        case ST_PLAY:
            updatePlay(dt);
            drawMap(0,0);
            drawSidebar(MAP_W, 0);
            break;
        case ST_DEAD:
            ce_clear(CE_DRED);
            ce_text(SCREEN_W/2-5, 20, "YOU DIED", CE_RED);
            ce_text(SCREEN_W/2-9, 24, "ENTER to restart", CE_GREY);
            if(ce_keyPressed(VK_RETURN)){ state=ST_TITLE; }
            break;
    }
}

int main(void){
    memset(&player, 0, sizeof(player));
    srand((unsigned)GetTickCount());
    ce_run(update);
    return 0;
}
