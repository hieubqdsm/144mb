/* =====================================================================
   FLOPPY DEFENDER - Twin-stick shooter (CONSOLE ENGINE)
   1.44MB Jam. ASCII render, procedural audio, 0 byte asset.
   Move: WASD   Shoot: Arrow keys   ESC: quit/menu
   ===================================================================== */
#define SCREEN_W 110
#define SCREEN_H 55
#define FONT_W 8
#define FONT_H 8     /* font vuong 8x8 = nhieu o hon + tranh chi tiet (nhu PEWBALL) */
#include "engine/console.h"
#include <math.h>
#include <stdlib.h>

#define MAX_BULLETS   48
#define MAX_ENEMIES   40
#define MAX_PARTICLES 200

/* ---------- SPRITES (ASCII art, 0 byte asset) ----------
   Dung Unicode block/shapes. ' ' = trong. */
static const char *SPR_PLAYER[] = {
    "  A  ",
    " BBB ",
    "BCBCB",
    " B B ",
};
#define SPR_PLAYER_W 5
#define SPR_PLAYER_H 4

static const char *SPR_ENEMY[] = {
    "###",
    "#X#",
    "###",
};
#define SPR_ENEMY_W 3
#define SPR_ENEMY_H 3

/* ---------- Entities (toa do float cho di chuyen muot) ---------- */
typedef struct { float x,y,vx,vy; int active; } Bullet;
typedef struct { float x,y; int active; int hp; } Enemy;
typedef struct { float x,y,vx,vy; int active; float life, maxlife; int col; } Particle;

static Bullet    bullets[MAX_BULLETS];
static Enemy     enemies[MAX_ENEMIES];
static Particle  particles[MAX_PARTICLES];

/* ---------- Game state ---------- */
enum { ST_MENU, ST_PLAY, ST_GAMEOVER };
static int  state = ST_MENU;
static float px, py, pvx, pvy;     /* player */
static int  php = 3;
static float shootCool = 0;
static int  score = 0, highscore = 0, wave = 1;
static int  enemiesLeft = 0;
static float spawnTimer = 0;
static float shake = 0, hurtFlash = 0;
static float t = 0;
/* FPS tracking */
static float fpsAccum = 0;
static int   fpsFrames = 0;
static int   fpsCurrent = 0;

/* ---------- Procedural SFX (Beep API - 0 byte asset) ---------- */
static void sfx(int freq, int dur, int type){
    /* type: 0=beep, 1=noise (dung Beep Windows API) */
    if(type==0){
        Beep(freq, dur);
    } else {
        /* Noise: nhieu beep ngan tan so giam */
        for(int i=0;i<3;i++) Beep(freq - i*20, dur/4);
    }
}
/* SFX ASYNC: chay Beep trong thread rieng de khong block game loop.
   Beep dong bo block thread -> FPS te. */
static DWORD WINAPI beepThread(LPVOID p){
    int *args = (int*)p;
    Beep(args[0], args[1]);
    free(args);
    return 0;
}
static void sfxBeepAsync(int freq, int dur){
    int *args = malloc(sizeof(int)*2);
    if(!args) return;
    args[0]=freq; args[1]=dur;
    HANDLE h = CreateThread(NULL, 0, beepThread, args, 0, NULL);
    if(h) CloseHandle(h);
}
static void sfxShoot(void){ sfxBeepAsync(880, 30); }
static void sfxHit(void){   sfxBeepAsync(300, 40); }
static void sfxBoom(void){  sfxBeepAsync(150, 80); sfxBeepAsync(100, 80); }
static void sfxDeath(void){ sfxBeepAsync(200, 200); sfxBeepAsync(150, 200); }

/* ---------- Spawn helpers ---------- */
static void spawnParticle(float x, float y, int col, float spd){
    for(int i=0;i<MAX_PARTICLES;i++){
        if(!particles[i].active){
            float a = (float)(rand()%360) * 3.14159f/180.0f;
            particles[i].x=x; particles[i].y=y;
            particles[i].vx=cosf(a)*spd; particles[i].vy=sinf(a)*spd*0.5f;
            particles[i].active=1;
            particles[i].life=particles[i].maxlife=0.4f;
            particles[i].col=col;
            return;
        }
    }
}
static void explode(float x, float y, int col, int n){
    for(int i=0;i<n;i++) spawnParticle(x,y,col,(float)(40+rand()%140));
}
static void fireBullet(float x, float y, float vx, float vy){
    for(int i=0;i<MAX_BULLETS;i++){
        if(!bullets[i].active){
            bullets[i].x=x; bullets[i].y=y; bullets[i].vx=vx; bullets[i].vy=vy;
            bullets[i].active=1;
            sfxShoot();
            return;
        }
    }
}
static void spawnEnemy(void){
    for(int i=0;i<MAX_ENEMIES;i++){
        if(!enemies[i].active){
            int side = rand()%4;
            if(side==0){ enemies[i].x=(float)(rand()%SCREEN_W); enemies[i].y=-2; }
            else if(side==1){ enemies[i].x=SCREEN_W+2; enemies[i].y=(float)(rand()%SCREEN_H); }
            else if(side==2){ enemies[i].x=(float)(rand()%SCREEN_W); enemies[i].y=SCREEN_H+2; }
            else { enemies[i].x=-2; enemies[i].y=(float)(rand()%SCREEN_H); }
            enemies[i].active=1;
            enemies[i].hp=1+wave/3;
            return;
        }
    }
}

static void resetGame(void){
    for(int i=0;i<MAX_BULLETS;i++)   bullets[i].active=0;
    for(int i=0;i<MAX_ENEMIES;i++)   enemies[i].active=0;
    for(int i=0;i<MAX_PARTICLES;i++) particles[i].active=0;
    px=SCREEN_W/2.0f; py=SCREEN_H/2.0f; pvx=pvy=0;
    php=3; shootCool=0; score=0; wave=1;
    enemiesLeft=4+wave; spawnTimer=0;
    shake=0; hurtFlash=0;
}

/* ---------- High score (file 4 byte) ---------- */
static void loadHi(void){
    HANDLE h = CreateFileA("hiscore.dat", GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h != INVALID_HANDLE_VALUE){
        DWORD rd; int v=0;
        if(ReadFile(h, &v, 4, &rd, NULL) && rd==4) highscore=v;
        CloseHandle(h);
    }
}
static void saveHi(void){
    if(score>highscore){
        highscore=score;
        HANDLE h = CreateFileA("hiscore.dat", GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(h != INVALID_HANDLE_VALUE){
            DWORD wr; WriteFile(h, &highscore, 4, &wr, NULL);
            CloseHandle(h);
        }
    }
}

/* ---------- Update ---------- */
/* === MOVEMENT PHYSICS (cong thuc chuan) ===
   Don vi: o/cell tren giay. Quy tac:
   - dt luu o giay
   - velocity = o/giay
   - MAX_SPEED = bang 100 o trong 3.5s = ~28 o/s
   - ACCEL = MAX_SPEED / 0.25s (dat max trong 0.25s)
   - DRAG = friction tuyen tinh (v -= v*drag*dt), durng ~0.4s khi thả
*/
#define PLAYER_MAX_SPEED 28.0f
#define PLAYER_ACCEL     112.0f   /* 28 / 0.25 */
#define PLAYER_DRAG      2.5f     /* tuyen tinh */
static void updatePlay(float dt){
    t += dt;
    /* FPS: dem frame, cap nhat moi 0.5s */
    fpsAccum += dt; fpsFrames++;
    if(fpsAccum >= 0.5f){ fpsCurrent = (int)(fpsFrames/fpsAccum); fpsAccum=0; fpsFrames=0; }
    /* Input (normalize de khong nhanh hon khi cheo) */
    float ax=0, ay=0;
    if(ce_keyDown('A')) ax-=1; if(ce_keyDown('D')) ax+=1;
    if(ce_keyDown('W')) ay-=1; if(ce_keyDown('S')) ay+=1;
    float dl = sqrtf(ax*ax+ay*ay);
    if(dl>0){ ax/=dl; ay/=dl; }

    /* 1. GIA TOC: ap dung input */
    pvx += ax * PLAYER_ACCEL * dt;
    pvy += ay * PLAYER_ACCEL * dt;
    /* 2. FRICTION tuyen tinh (luon ap dung, bat ke input) */
    pvx -= pvx * PLAYER_DRAG * dt;
    pvy -= pvy * PLAYER_DRAG * dt;
    /* 3. CAP toc do */
    float sp = sqrtf(pvx*pvx+pvy*pvy);
    if(sp > PLAYER_MAX_SPEED){ pvx=pvx/sp*PLAYER_MAX_SPEED; pvy=pvy/sp*PLAYER_MAX_SPEED; }
    /* 4. DI CHUYEN */
    px += pvx*dt; py += pvy*dt;
    /* 5. Clamp man hinh */
    if(px<2){px=2;pvx*=-0.3f;} if(px>SCREEN_W-3){px=SCREEN_W-3;pvx*=-0.3f;}
    if(py<2){py=2;pvy*=-0.3f;} if(py>SCREEN_H-3){py=SCREEN_H-3;pvy*=-0.3f;}

    /* Shooting (8 huong = arrow keys) */
    shootCool -= dt;
    int sx=0, sy=0;
    if(ce_keyDown(VK_LEFT))  sx=-1;
    if(ce_keyDown(VK_RIGHT)) sx= 1;
    if(ce_keyDown(VK_UP))    sy=-1;
    if(ce_keyDown(VK_DOWN))  sy= 1;
    if((sx||sy) && shootCool<=0){
        float sl = sqrtf((float)(sx*sx+sy*sy));
        fireBullet(px, py, sx/sl*45.0f, sy/sl*45.0f);  /* 45 o/s - nhanh hon player */
        shootCool = 0.18f;
    }

    /* Bullets */
    for(int i=0;i<MAX_BULLETS;i++){
        if(!bullets[i].active) continue;
        bullets[i].x += bullets[i].vx*dt; bullets[i].y += bullets[i].vy*dt;
        if(bullets[i].x<0||bullets[i].x>=SCREEN_W||bullets[i].y<0||bullets[i].y>=SCREEN_H)
            bullets[i].active=0;
    }

    /* Spawn enemies */
    spawnTimer -= dt;
    if(enemiesLeft>0 && spawnTimer<=0){ spawnEnemy(); enemiesLeft--; spawnTimer=0.8f; }
    int live=0;
    for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].active) live++;
    if(enemiesLeft==0 && live==0){ wave++; enemiesLeft=4+wave*2; spawnTimer=1.0f; }

    /* Enemies AI + collision */
    for(int i=0;i<MAX_ENEMIES;i++){
        if(!enemies[i].active) continue;
        float dx=px-enemies[i].x, dy=py-enemies[i].y;
        float d=sqrtf(dx*dx+dy*dy)+0.001f;
        float espd = 8.0f + wave*0.8f;   /* < player 28, tang theo wave */
        enemies[i].x += dx/d*espd*dt; enemies[i].y += dy/d*espd*dt;
        /* Hit player */
        if(d < 3.0f){
            enemies[i].active=0;
            explode(enemies[i].x, enemies[i].y, CE_RED, 16);
            sfxBoom(); php--; shake=8; hurtFlash=0.5f;
            if(php<=0){ sfxDeath(); saveHi(); state=ST_GAMEOVER; }
        }
        /* Hit bullet */
        for(int j=0;j<MAX_BULLETS;j++){
            if(!bullets[j].active) continue;
            float bd = (enemies[i].x-bullets[j].x)*(enemies[i].x-bullets[j].x)
                     + (enemies[i].y-bullets[j].y)*(enemies[i].y-bullets[j].y);
            if(bd < 6.0f){
                bullets[j].active=0; enemies[i].hp--;
                explode(bullets[j].x,bullets[j].y,CE_YEL,4);
                if(enemies[i].hp<=0){
                    enemies[i].active=0;
                    explode(enemies[i].x,enemies[i].y,CE_YEL,14);
                    sfxBoom(); score+=10; shake=3;
                } else sfxHit();
                break;
            }
        }
    }

    /* Particles */
    for(int i=0;i<MAX_PARTICLES;i++){
        if(!particles[i].active) continue;
        particles[i].x += particles[i].vx*dt;
        particles[i].y += particles[i].vy*dt;
        particles[i].vx *= powf(0.05f,dt); particles[i].vy *= powf(0.05f,dt);
        particles[i].life -= dt;
        if(particles[i].life<=0) particles[i].active=0;
    }

    if(shake>0) shake -= dt*30.0f;
    if(hurtFlash>0) hurtFlash -= dt;

    if(ce_keyDown(VK_ESCAPE)){ saveHi(); state=ST_MENU; }
}

/* ---------- Render ---------- */
/* Unicode block/shape characters cho visual dep hon */
#define CH_FILL   (WCHAR)0x2588   /* █ day dac */
#define CH_MID    (WCHAR)0x2593   /* ▓ */
#define CH_LIGHT  (WCHAR)0x2592   /* ▒ */
#define CH_FAINT  (WCHAR)0x2591   /* ░ */
#define CH_BULLET (WCHAR)0x2022   /* • */
#define CH_DIAMOND (WCHAR)0x2666  /* ◆ */
#define CH_TRI_UP  (WCHAR)0x25B2  /* ▲ */
#define CH_CIRCLE  (WCHAR)0x25CF  /* ● */
#define CH_RING    (WCHAR)0x25CB  /* ○ */
#define CH_STAR    (WCHAR)0x2605  /* ★ */

static int irandShake(int s){ return s>0 ? (rand()%(2*s+1))-s : 0; }

static void drawPlay(void){
    int shx = irandShake((int)shake), shy = irandShake((int)shake);
    ce_clear(CE_BLACK);

    /* Particles - fade qua 3 cap do */
    for(int i=0;i<MAX_PARTICLES;i++){
        if(!particles[i].active) continue;
        float a = particles[i].life/particles[i].maxlife;
        WCHAR ch = a>0.66f?CH_FILL:(a>0.33f?CH_MID:CH_LIGHT);
        ce_putc((int)particles[i].x+shx, (int)particles[i].y+shy, ch, particles[i].col);
    }
    /* Enemies - hinh cau gradient 3D (nhu PEWBALL mat trang) */
    for(int i=0;i<MAX_ENEMIES;i++){
        if(!enemies[i].active) continue;
        int ex=(int)enemies[i].x+shx;
        int ey=(int)enemies[i].y+shy;
        /* HP cao = enemy lon hon */
        int r = (enemies[i].hp > 1) ? 4 : 3;
        ce_sphere(ex, ey, r, CE_RED, CE_YEL, 0.85f);
    }
    /* Bullets - cham sang + halo nho */
    for(int i=0;i<MAX_BULLETS;i++){
        if(!bullets[i].active) continue;
        int bx=(int)bullets[i].x+shx, by=(int)bullets[i].y+shy;
        ce_putc(bx, by, CH_BULLET, CE_WHITE);
        ce_putc(bx-1, by, CH_FAINT, CE_YEL);  /* halo */
        ce_putc(bx+1, by, CH_FAINT, CE_YEL);
    }
    /* Player - tàu bitmap pattern 7x5 chi tiet (A=engine, B=body, C=cockpit, D=wing) */
    {
        static const char *ship[5] = {
            "   A   ",
            "  BBB  ",
            " BBCCB ",
            "BBBCBBB",
            " D B D ",
        };
        int sx0 = (int)px+shx - 3;
        int sy0 = (int)py+shy - 2;
        for(int j=0;j<5;j++){
            const char *row = ship[j];
            for(int sx=0; sx<7; sx++){
                char c = row[sx];
                if(c==' ') continue;
                int col; WCHAR ch;
                switch(c){
                    case 'A': col=CE_RED;   ch=CH_FILL;   break;   /* engine phun lua */
                    case 'B': col=CE_CYAN;  ch=CH_FILL;   break;   /* body */
                    case 'C': col=CE_YEL;   ch=CH_DIAMOND;break;   /* cockpit sang */
                    case 'D': col=CE_BLUE;  ch=CH_MID;    break;   /* wing nhat */
                    default:  col=CE_CYAN;  ch=CH_FILL;
                }
                /* Engine phun lua nhap nhay */
                if(c=='A' && ((int)(t*15)%2==0)) col = CE_YEL;
                ce_putc(sx0+sx, sy0+j, ch, col);
            }
        }
    }

    /* Hurt flash - vien do nhay */
    if(hurtFlash>0){
        for(int x=0;x<SCREEN_W;x++){
            ce_putc(x,0,CH_FILL,CE_RED); ce_putc(x,SCREEN_H-1,CH_FILL,CE_RED);
            ce_putc(x,1,CH_LIGHT,CE_DRED); ce_putc(x,SCREEN_H-2,CH_LIGHT,CE_DRED);
        }
        for(int y=0;y<SCREEN_H;y++){
            ce_putc(0,y,CH_FILL,CE_RED); ce_putc(SCREEN_W-1,y,CH_FILL,CE_RED);
        }
    }

    /* HUD */
    char buf[64];
    sprintf(buf,"SCORE %d",score); ce_text(2,1,buf,CE_WHITE);
    sprintf(buf,"WAVE %d",wave);   ce_text(2,2,buf,CE_GREY);
    sprintf(buf,"HI %d",highscore); ce_text(SCREEN_W-20,1,buf,CE_YEL);
    sprintf(buf,"FPS %d",fpsCurrent); ce_text(SCREEN_W-12,2,buf,fpsCurrent>=50?CE_GREEN:(fpsCurrent>=30?CE_YEL:CE_RED));
    /* HP */
    ce_text(2, SCREEN_H-2, "HP", CE_GREEN);
    for(int i=0;i<3;i++) ce_putc(5+i,SCREEN_H-2, i<php?(WCHAR)0x2588:'.', i<php?CE_GREEN:CE_DGREY);
}

static void drawMenu(void){
    ce_clear(CE_DBLUE);
    /* Starfield dong - sao bay tu tam ra (hoc tu PEWBALL TitleState) */
    {
        srand(12345);  /* seed co dinh de starfield on dinh */
        int cx = SCREEN_W/2, cy = SCREEN_H/2;
        for(int i=0;i<60;i++){
            float ang = (float)(rand()%360) * 3.14159f/180.0f;
            float dist = fmodf(t*30.0f + (rand()%80), 50.0f) + 5.0f;
            int sx = cx + (int)(cosf(ang)*dist);
            int sy = cy + (int)(sinf(ang)*dist*0.5f);
            int shade = (int)(dist/12);  /* xa hon = nhat hon */
            WCHAR ch = shade>=3?CH_FAINT:(shade>=2?CH_LIGHT:(shade>=1?CH_MID:CH_FILL));
            ce_putc(sx, sy, ch, CE_WHITE);
        }
        srand((unsigned)GetTickCount());  /* restore random cho game */
    }
    ce_border(CH_FILL, CE_BLUE);
    /* Tau lon demo sprite o giua menu (tren title) */
    ce_sprite(SCREEN_W/2-3, 4, "   A   \n"
                      "  BBB  \n"
                      " BBCCB \n"
                      "BBBCBBB\n"
                      " D B D ", CE_CYAN);
    /* Title voi sao 2 ben (can giua) */
    ce_putc(SCREEN_W/2-10, 12, CH_STAR, CE_YEL);
    ce_text(SCREEN_W/2-7, 12, "FLOPPY DEFENDER", CE_YEL);
    ce_putc(SCREEN_W/2+9, 12, CH_STAR, CE_YEL);
    ce_text(SCREEN_W/2-5, 14, "1.44MB JAM", CE_GREEN);
    /* Box huong dan */
    ce_fillbg(SCREEN_W/2-25, 20, SCREEN_W/2+25, 30, CE_BLACK);
    for(int x=SCREEN_W/2-25;x<SCREEN_W/2+25;x++){ ce_putc(x,20,CH_LIGHT,CE_DGREY); ce_putc(x,29,CH_LIGHT,CE_DGREY); }
    for(int y=20;y<30;y++){ ce_putc(SCREEN_W/2-25,y,CH_LIGHT,CE_DGREY); ce_putc(SCREEN_W/2+24,y,CH_LIGHT,CE_DGREY); }
    ce_text(SCREEN_W/2-8, 23, "Move:  W A S D", CE_WHITE);
    ce_text(SCREEN_W/2-13, 26, "Shoot: Arrow Keys (8 directions)", CE_WHITE);
    /* Nhap nhay PRESS ENTER */
    if(((int)(t*2))%2==0) ce_text(SCREEN_W/2-8, 35, "> PRESS ENTER <", CE_CYAN);
    char buf[64]; sprintf(buf,"HIGH SCORE: %d",highscore);
    ce_text(SCREEN_W/2-7, 40, buf, CE_MAG);
    ce_text(SCREEN_W/2-5, SCREEN_H-3, "ESC to quit", CE_GREY);
}

static void drawGameOver(void){
    ce_clear(CE_DRED);
    ce_border(CH_FILL, CE_RED);
    ce_putc(SCREEN_W/2-7, 18, CH_STAR, CE_YEL);
    ce_text(SCREEN_W/2-5, 18, "GAME OVER", CE_WHITE);
    ce_putc(SCREEN_W/2+5, 18, CH_STAR, CE_YEL);
    char buf[64];
    sprintf(buf,"SCORE: %d",score);          ce_text(SCREEN_W/2-5, 26, buf, CE_YEL);
    sprintf(buf,"WAVE REACHED: %d",wave);    ce_text(SCREEN_W/2-8, 28, buf, CE_GREY);
    ce_text(SCREEN_W/2-12, 36, "ENTER retry | ESC menu", CE_GREEN);
}

/* ---------- Main ---------- */
void update(float dt){
    switch(state){
        case ST_MENU:
            drawMenu();
            if(ce_keyPressed(VK_RETURN)){ resetGame(); state=ST_PLAY; t=0; }
            if(ce_keyPressed(VK_ESCAPE)) ce_quit();
            break;
        case ST_PLAY:
            updatePlay(dt);
            drawPlay();
            break;
        case ST_GAMEOVER:
            drawGameOver();
            if(ce_keyPressed(VK_RETURN)){ resetGame(); state=ST_PLAY; t=0; }
            if(ce_keyPressed(VK_ESCAPE)) state=ST_MENU;
            break;
    }
}

int main(void){
    loadHi();
    srand((unsigned)GetTickCount());
    ce_run(update);
    return 0;
}
