/* =====================================================================
   CONSOLE ENGINE - Implementation (1 ban duy nhat, link vao moi module)
   Win32 Console API: WriteConsoleOutput + CHAR_INFO backbuffer.
   ===================================================================== */
/* Engine can day du Win32 API (CONSOLE_FONT_INFOEX, WriteConsoleOutput...).
   KHONG define NOGDI/LEAN_AND_MEAN o day. */
#include <stdio.h>
#include "console.h"   /* console.h da #include <windows.h> day du */

/* ---------- Internal state ---------- */
static CHAR_INFO  ce_buf[SCREEN_W * SCREEN_H];
static HANDLE     ce_out, ce_in;
static SMALL_RECT ce_rect = {0,0,(SHORT)(SCREEN_W-1),(SHORT)(SCREEN_H-1)};
static int        ce_keys[256];
static int        ce_prevKeys[256];
int               ce_running = 1;
int               ce_mouseX = 0, ce_mouseY = 0;
static int        ce_mouseBtn[3];
static int        ce_prevMouseBtn[3];

/* ---------- Timing ---------- */
static double ce_freq;
static double ce_startTime;
double ce_now(void){
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart / ce_freq;
}

/* ---------- Init ---------- */
int ce_init(void){
    ce_out = GetStdHandle(STD_OUTPUT_HANDLE);
    ce_in  = GetStdHandle(STD_INPUT_HANDLE);
    if(ce_out == INVALID_HANDLE_VALUE) return 1;

    /* Phai thu nho WINDOW truoc khi set BUFFER lon (bug Windows console). */
    COORD bufSize = {(SHORT)SCREEN_W, (SHORT)SCREEN_H};
    SMALL_RECT tiny = {0,0,1,1};
    SetConsoleWindowInfo(ce_out, TRUE, &tiny);
    SetConsoleScreenBufferSize(ce_out, bufSize);
    SetConsoleWindowInfo(ce_out, TRUE, &ce_rect);
    SetConsoleActiveScreenBuffer(ce_out);

    /* Font Consolas (co dinh) */
    CONSOLE_FONT_INFOEX cfi; ZeroMemory(&cfi, sizeof(cfi));
    cfi.cbSize = sizeof(cfi);
    cfi.dwFontSize.X = FONT_W; cfi.dwFontSize.Y = FONT_H;
    cfi.FontWeight = FW_NORMAL;
    /* Copy font name "Consolas" (khai bao o dau block cho C90 compatible) */
    {
        const wchar_t *fname = L"Consolas";
        int fi = 0;
        for(; fname[fi] && fi < LF_FACESIZE-1; fi++) cfi.FaceName[fi] = fname[fi];
        cfi.FaceName[fi] = 0;
    }
    SetCurrentConsoleFontEx(ce_out, FALSE, &cfi);

    /* An con tro */
    CONSOLE_CURSOR_INFO ci; GetConsoleCursorInfo(ce_out, &ci);
    ci.bVisible = FALSE; SetConsoleCursorInfo(ce_out, &ci);

    /* Input mode */
    SetConsoleMode(ce_in, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);

    LARGE_INTEGER f; QueryPerformanceFrequency(&f); ce_freq = (double)f.QuadPart;
    ce_startTime = ce_now();
    return 0;
}

/* ---------- Render primitives ---------- */
void ce_clear(int col){
    CHAR_INFO ci; ci.Char.UnicodeChar = (WCHAR)' ';
    ci.Attributes = (WORD)(col << 4);
    int n = SCREEN_W*SCREEN_H;
    for(int i=0;i<n;i++) ce_buf[i] = ci;
}
void ce_clear_black(void){ ce_clear(CE_BLACK); }

void ce_put(int x, int y, WCHAR ch, int frg, int bkg){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return;
    ce_buf[y*SCREEN_W + x].Char.UnicodeChar = ch;
    ce_buf[y*SCREEN_W + x].Attributes = (WORD)(frg | (bkg << 4));
}
void ce_putc(int x, int y, WCHAR ch, int frg){ ce_put(x, y, ch, frg, CE_BLACK); }

void ce_fill(int x0,int y0,int x1,int y1, WCHAR ch, int frg, int bkg){
    for(int y=y0;y<y1;y++) for(int x=x0;x<x1;x++) ce_put(x,y,ch,frg,bkg);
}
void ce_fillbg(int x0,int y0,int x1,int y1, int bkg){
    ce_fill(x0,y0,x1,y1,(WCHAR)' ', CE_BLACK, bkg);
}

void ce_line(int x0,int y0,int x1,int y1, WCHAR ch, int fr){
    int dx = abs(x1-x0), sx = x0<x1?1:-1;
    int dy = -abs(y1-y0), sy = y0<y1?1:-1;
    int err = dx+dy, e2;
    for(;;){
        ce_putc(x0,y0,ch,fr);
        if(x0==x1 && y0==y1) break;
        e2 = 2*err;
        if(e2 >= dy){ err += dy; x0 += sx; }
        if(e2 <= dx){ err += dx; y0 += sy; }
    }
}

void ce_circle(int cx, int cy, int r, WCHAR ch, int fr, float aspect){
    for(int y=-r; y<=r; y++){
        for(int x=-r; x<=r; x++){
            float d = (x*x) + (y*y)/(aspect*aspect);
            if(d <= r*r + r*0.5f) ce_putc(cx+x, cy+y, ch, fr);
        }
    }
}
void ce_circleOutline(int cx, int cy, int r, WCHAR ch, int fr, float aspect){
    for(int y=-r; y<=r; y++){
        for(int x=-r; x<=r; x++){
            float d = (x*x) + (y*y)/(aspect*aspect);
            if(d <= r*r + r*0.5f && d >= r*r - r*1.5f) ce_putc(cx+x, cy+y, ch, fr);
        }
    }
}
void ce_fillCircle(int cx, int cy, int r, WCHAR ch, int fr, float aspect){
    if(r < 0) return;
    for(int y=-r; y<=r; y++){
        for(int x=-r; x<=r; x++){
            float d = x*x + (y*y)/(aspect*aspect);
            if(d <= r*r + r) ce_putc(cx+x, cy+y, ch, fr);
        }
    }
}
void ce_sphere(int cx, int cy, int r, int baseCol, int hlCol, float aspect){
    static const int shadeChars[4] = {0x2591, 0x2592, 0x2593, 0x2588};  /* ░ ▒ ▓ █ */
    ce_fillCircle(cx, cy, r,     shadeChars[0], baseCol, aspect);
    ce_fillCircle(cx-1, cy-1, r-1, shadeChars[1], baseCol, aspect);
    ce_fillCircle(cx-1, cy-1, r-2, shadeChars[2], baseCol, aspect);
    ce_fillCircle(cx-1, cy-1, r-3, shadeChars[3], baseCol, aspect);
    ce_fillCircle(cx - r/3, cy - r/3, r/3, shadeChars[3], hlCol, aspect);
}

int ce_text(int x, int y, const char *s, int fr){
    int i=0;
    for(; s[i]; i++) ce_putc(x+i, y, (WCHAR)(unsigned char)s[i], fr);
    return i;
}

/* Wide-char text: ho tro Unicode/CP437 glyph (vd box-drawing ─│┌┐). */
int ce_text_w(int x, int y, const WCHAR *s, int fr){
    int i=0;
    for(; s[i]; i++) ce_putc(x+i, y, s[i], fr);
    return i;
}

void ce_border(WCHAR ch, int col){
    for(int x=0;x<SCREEN_W;x++){ ce_putc(x,0,ch,col); ce_putc(x,SCREEN_H-1,ch,col); }
    for(int y=0;y<SCREEN_H;y++){ ce_putc(0,y,ch,col); ce_putc(SCREEN_W-1,y,ch,col); }
}

/* ---------- Sprite ---------- */
void ce_sprite(int x0, int y0, const char *sprite, int fr){
    int x = 0, y = 0;
    for(const char *p = sprite; *p; p++){
        if(*p == '\n'){ x = 0; y++; continue; }
        if(*p != ' ') ce_putc(x0 + x, y0 + y, (WCHAR)(unsigned char)*p, fr);
        x++;
    }
}
void ce_sprite2(int x0, int y0, const char *sprite, int fr, char hl, int fr2){
    int x = 0, y = 0;
    for(const char *p = sprite; *p; p++){
        if(*p == '\n'){ x = 0; y++; continue; }
        if(*p != ' '){
            int c = (*p == hl) ? fr2 : fr;
            ce_putc(x0 + x, y0 + y, (WCHAR)(unsigned char)*p, c);
        }
        x++;
    }
}
/* Multi-color sprite: cells la mang w*h, moi cell co (glyph, fg, bg). */
void ce_sprite_multi(int x0, int y0, int w, int h, const CESpriteCell *cells){
    for(int y=0;y<h;y++){
        for(int x=0;x<w;x++){
            const CESpriteCell *c = &cells[y*w + x];
            if(c->ch == 0) continue;  /* 0 = transparent */
            ce_put(x0+x, y0+y, c->ch, c->fg, c->bg);
        }
    }
}

/* ---------- Cell query ---------- */
WCHAR ce_getch(int x, int y){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return ' ';
    return ce_buf[y*SCREEN_W + x].Char.UnicodeChar;
}
int ce_getattr(int x, int y){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return 0;
    return (int)ce_buf[y*SCREEN_W + x].Attributes;
}

/* ---------- Flip ---------- */
void ce_flip(void){
    COORD bufSize = {(SHORT)SCREEN_W, (SHORT)SCREEN_H};
    COORD origin = {0,0};
    WriteConsoleOutput(ce_out, ce_buf, bufSize, origin, &ce_rect);
}

/* ---------- Input ---------- */
void ce_inputUpdate(void){
    for(int i=0;i<256;i++){
        int held = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
        if(held && !ce_prevKeys[i])      ce_keys[i] = 2;
        else if(held)                    ce_keys[i] = 1;
        else                             ce_keys[i] = 0;
    }
    int mb[3] = {
        (GetAsyncKeyState(VK_LBUTTON)&0x8000)?1:0,
        (GetAsyncKeyState(VK_RBUTTON)&0x8000)?1:0,
        (GetAsyncKeyState(VK_MBUTTON)&0x8000)?1:0,
    };
    for(int i=0;i<3;i++){
        if(mb[i] && !ce_prevMouseBtn[i]) ce_mouseBtn[i] = 2;
        else if(mb[i])                   ce_mouseBtn[i] = 1;
        else                             ce_mouseBtn[i] = 0;
    }
    INPUT_RECORD ir[32]; DWORD n=0;
    if(GetNumberOfConsoleInputEvents(ce_in, &n) && n>0){
        ReadConsoleInput(ce_in, ir, n, &n);
        for(DWORD i=0;i<n;i++){
            if(ir[i].EventType == MOUSE_EVENT){
                ce_mouseX = ir[i].Event.MouseEvent.dwMousePosition.X;
                ce_mouseY = ir[i].Event.MouseEvent.dwMousePosition.Y;
            }
        }
    }
}
void ce_inputEnd(void){
    for(int i=0;i<256;i++) ce_prevKeys[i] = (ce_keys[i]!=0);
    for(int i=0;i<3;i++)  ce_prevMouseBtn[i] = (ce_mouseBtn[i]!=0);
}
int ce_key(int vk){ return ce_keys[vk]; }
int ce_keyPressed(int vk){ return ce_keys[vk]==2; }
int ce_keyDown(int vk){ return ce_keys[vk]!=0; }
int ce_mouseClicked(int btn){ return ce_mouseBtn[btn]==2; }
int ce_mouseDown(int btn){ return ce_mouseBtn[btn]!=0; }
int ce_clickedBox(int x0, int y0, int x1, int y1){
    if(ce_mouseBtn[0]!=2) return 0;
    return (ce_mouseX>=x0 && ce_mouseX<=x1 && ce_mouseY>=y0 && ce_mouseY<=y1);
}
int ce_hoverBox(int x0, int y0, int x1, int y1){
    return (ce_mouseX>=x0 && ce_mouseX<=x1 && ce_mouseY>=y0 && ce_mouseY<=y1);
}

/* ---------- Shutdown ---------- */
void ce_shutdown(void){
    SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
}
void ce_quit(void){ ce_running = 0; }

/* ---------- Main loop ---------- */
void ce_run(void (*game_update)(float dt)){
    if(ce_init()){
        MessageBoxA(NULL, "ce_init() that bai - chay ngoai console?", "Loi", MB_OK);
        return;
    }
    double targetFrame = 1.0 / TARGET_FPS;
    double lastTime = ce_now() - ce_startTime;
    /* Tang do phan giai timer Windows len 1ms (mac dinh ~15ms -> gay giat) */
    timeBeginPeriod(1);
    while(ce_running){
        double now = ce_now() - ce_startTime;
        float dt = (float)(now - lastTime);
        lastTime = now;
        if(dt > 0.1f) dt = 0.1f;

        ce_inputUpdate();
        game_update(dt);
        ce_flip();
        ce_inputEnd();

        /* Hybrid sleep: Sleep phan lon + spin-wait 2ms cuoi (chinh xac) */
        double elapsed = (ce_now()-ce_startTime) - now;
        double wait = targetFrame - elapsed;
        if(wait > 0.002){
            double sleepPart = wait - 0.002;
            Sleep((DWORD)(sleepPart*1000.0));
            while((ce_now()-ce_startTime) - now < targetFrame) { /* spin */ }
        }
    }
    timeEndPeriod(1);
    ce_shutdown();
}
