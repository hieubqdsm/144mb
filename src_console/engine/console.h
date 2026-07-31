/* =====================================================================
   CONSOLE ENGINE - Header (declarations only)
   BLT-style minimal API tren Win32 WriteConsoleOutput.
   --------------------------------------------------------------------
   Cach dung:
     #define SCREEN_W 110
     #define SCREEN_H 55
     #define FONT_W 8
     #define FONT_H 8
     #include "engine/console.h"
     ...
     int main(){ ce_run(your_update_fn); }

   LINK DEPS: user32.lib kernel32.lib winmm.lib
   ===================================================================== */
#ifndef CE_CONSOLE_H
#define CE_CONSOLE_H

/* Include day du windows.h (KHONG NOGDI, KHONG LEAN_AND_MEAN):
   engine can CONSOLE_FONT_INFOEX, WriteConsoleOutput...
   Tranh DrawText macro conflict bang cach include windows.h TRUOC tat ca. */
#include <windows.h>   /* WCHAR, CHAR_INFO, HANDLE, CONSOLE_FONT_INFOEX */

/* ---------- Config (override truoc #include) ---------- */
#ifndef SCREEN_W
#define SCREEN_W 120
#endif
#ifndef SCREEN_H
#define SCREEN_H 40
#endif
#ifndef FONT_W
#define FONT_W 8
#endif
#ifndef FONT_H
#define FONT_H 16
#endif
#ifndef TARGET_FPS
#define TARGET_FPS 60
#endif

/* ---------- Color (16 mau, 4-bit fore + 4-bit back) ---------- */
typedef enum {
    CE_BLACK=0, CE_DBLUE, CE_DGREEN, CE_DCYN, CE_DRED, CE_DMAG, CE_DYEL, CE_GREY,
    CE_DGREY, CE_BLUE, CE_GREEN, CE_CYAN, CE_RED,  CE_MAG,  CE_YEL,  CE_WHITE,
} CE_Color;

/* ---------- Sprite types (cho multi-color) ---------- */
/* 1 cell sprite: (glyph, fg, bg). Dung trong ce_sprite_multi. */
typedef struct { WCHAR ch; unsigned char fg; unsigned char bg; } CESpriteCell;

/* ---------- Lifecycle ---------- */
extern int  ce_running;          /* set 0 de thoat */
void ce_run(void (*game_update)(float dt));   /* main loop driver */
void ce_quit(void);
int  ce_init(void);
void ce_shutdown(void);

/* ---------- Timing ---------- */
double ce_now(void);             /* giay tinh tu startup */

/* ---------- Render primitives ---------- */
void ce_clear(int col);
void ce_clear_black(void);
void ce_put(int x, int y, WCHAR ch, int frg, int bkg);
void ce_putc(int x, int y, WCHAR ch, int frg);
void ce_fill(int x0, int y0, int x1, int y1, WCHAR ch, int frg, int bkg);
void ce_fillbg(int x0, int y0, int x1, int y1, int bkg);
void ce_line(int x0, int y0, int x1, int y1, WCHAR ch, int fr);
void ce_circle(int cx, int cy, int r, WCHAR ch, int fr, float aspect);
void ce_circleOutline(int cx, int cy, int r, WCHAR ch, int fr, float aspect);
void ce_fillCircle(int cx, int cy, int r, WCHAR ch, int fr, float aspect);
void ce_sphere(int cx, int cy, int r, int baseCol, int hlCol, float aspect);
int  ce_text(int x, int y, const char *s, int fr);
void ce_border(WCHAR ch, int col);
void ce_flip(void);

/* ---------- Sprite (ASCII art multi-line) ---------- */
void ce_sprite(int x0, int y0, const char *sprite, int fr);
void ce_sprite2(int x0, int y0, const char *sprite, int fr, char hl, int fr2);
/* Multi-color sprite: mang 2D cells (width x height). */
void ce_sprite_multi(int x0, int y0, int w, int h, const CESpriteCell *cells);
/* Wide-char text (ho tro Unicode/CP437 glyph) tai (x,y). */
int  ce_text_w(int x, int y, const WCHAR *s, int fr);

/* ---------- Cell query (cho FOV/editor) ---------- */
WCHAR ce_getch(int x, int y);
int  ce_getattr(int x, int y);   /* tra ve fg | (bg<<4) */

/* ---------- Input ---------- */
void ce_inputUpdate(void);
void ce_inputEnd(void);
int  ce_key(int vk);
int  ce_keyPressed(int vk);
int  ce_keyDown(int vk);
int  ce_mouseClicked(int btn);
int  ce_mouseDown(int btn);
int  ce_clickedBox(int x0, int y0, int x1, int y1);
int  ce_hoverBox(int x0, int y0, int x1, int y1);

/* ---------- Input state (extern, read-only cho game) ---------- */
extern int ce_mouseX, ce_mouseY;

#endif /* CE_CONSOLE_H */
