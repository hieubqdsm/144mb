/* =====================================================================
   GDI RENDERER - Win32 GDI bitmap renderer (thay console.c).
   Giữ nguyên console.h API. Render ASCII grid qua ExtTextOutW + DIBSection.
   Multi-font foundation: 2 HFONT (tile + hud) san sang cho Phase 2.
   LINK: gdi32.lib user32.lib kernel32.lib winmm.lib
   ===================================================================== */
#include <stdio.h>
#include <string.h>
#include "console.h"

/* ---------- 16-color CGA palette (RGB) ---------- */
static const COLORREF CE_RGB[16] = {
    0x000000, 0x0000A8, 0x00A800, 0x00A8A8,   /* black blue green cyan   */
    0xA80000, 0xA800A8, 0xA85400, 0xA8A8A8,   /* red  magenta brown grey */
    0x545454, 0x5454FC, 0x54FC54, 0x54FCFC,   /* dgrey lblue lgreen lcyan*/
    0xFC5454, 0xFC54FC, 0xFCFC54, 0xFCFCFC,   /* lred  lmagenta yellow white */
};

/* ---------- Internal backbuffer cell ---------- */
typedef struct { WCHAR ch; unsigned char fg; unsigned char bg; } Cell;

static Cell   g_buf[SCREEN_W * SCREEN_H];
static int    g_keys[256], g_prevKeys[256];
static int    g_mouseBtn[3], g_prevMouseBtn[3];
int           ce_running = 1;
int           ce_mouseX = 0, ce_mouseY = 0;

/* GDI state */
static HWND   g_hwnd;
static HDC    g_screenDC, g_memDC;
static HBITMAP g_dib, g_oldBmp;
static HFONT  g_tileFont, g_oldFont;
static int    g_cellW, g_cellH;     /* pixel size of 1 cell */
static int    g_windowW, g_windowH; /* client area pixels */
static LPCWSTR g_className = L"CERpgClass";

/* Timing */
static double g_freq, g_startTime;
double ce_now(void){
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)t.QuadPart / g_freq;
}

/* ---------- WndProc ---------- */
static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp){
    switch(msg){
        case WM_ERASEBKGND: return 1;            /* suppress flicker */
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(h, &ps);
            if(g_memDC) BitBlt(dc, 0,0, g_windowW, g_windowH, g_memDC, 0,0, SRCCOPY);
            EndPaint(h, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            ce_running = 0;
            return 0;
        case WM_MOUSEMOVE: {
            /* Convert pixel -> cell coords */
            int px = LOWORD(lp), py = HIWORD(lp);
            ce_mouseX = (g_cellW > 0) ? px / g_cellW : 0;
            ce_mouseY = (g_cellH > 0) ? py / g_cellH : 0;
            return 0;
        }
        case WM_LBUTTONDOWN: g_mouseBtn[0] = 2; return 0;
        case WM_RBUTTONDOWN: g_mouseBtn[1] = 2; return 0;
        case WM_MBUTTONDOWN: g_mouseBtn[2] = 2; return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

/* ---------- Init ---------- */
int ce_init(void){
    /* Auto-scale cell size theo man hinh */
    int scrW = GetSystemMetrics(SM_CXSCREEN);
    int scrH = GetSystemMetrics(SM_CYSCREEN);
    int maxW = scrW / SCREEN_W;
    int maxH = (scrH - 80) / SCREEN_H;
    int fit = (maxW < maxH) ? maxW : maxH;
    if(fit < 8) fit = 8;
    if(fit > 36) fit = 36;
    g_cellW = g_cellH = fit;
    g_windowW = SCREEN_W * g_cellW;
    g_windowH = SCREEN_H * g_cellH;

    /* Register window class */
    WNDCLASSW wc; ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = g_className;
    RegisterClassW(&wc);

    /* Compute window rect (client = g_windowW x g_windowH) */
    RECT rc = {0, 0, g_windowW, g_windowH};
    DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);  /* fixed size */
    AdjustWindowRect(&rc, style, FALSE);
    int ww = rc.right - rc.left, wh = rc.bottom - rc.top;
    int wx = (scrW - ww) / 2, wy = (scrH - wh) / 2;
    if(wx < 0) wx = 0; if(wy < 0) wy = 0;

    g_hwnd = CreateWindowExW(0, g_className, L"ASCII Dungeon Crawler",
                             style, wx, wy, ww, wh,
                             NULL, NULL, wc.hInstance, NULL);
    if(!g_hwnd) return 1;

    /* DC + double buffer */
    g_screenDC = GetDC(g_hwnd);
    g_memDC = CreateCompatibleDC(g_screenDC);
    BITMAPINFO bi; ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = g_windowW;
    bi.bmiHeader.biHeight = -g_windowH;   /* top-down */
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    g_dib = CreateDIBSection(g_memDC, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
    g_oldBmp = (HBITMAP)SelectObject(g_memDC, g_dib);

    /* Font monospace */
    g_tileFont = CreateFontW(g_cellH, g_cellW, 0, 0, FW_NORMAL, 0,0,0,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
                             FF_DONTCARE | FIXED_PITCH, L"Consolas");
    g_oldFont = (HFONT)SelectObject(g_memDC, g_tileFont);
    SetTextColor(g_memDC, CE_RGB[CE_WHITE]);
    SetBkColor(g_memDC, CE_RGB[CE_BLACK]);
    SetBkMode(g_memDC, OPAQUE);

    /* Timing */
    LARGE_INTEGER f; QueryPerformanceFrequency(&f); g_freq = (double)f.QuadPart;
    g_startTime = ce_now();

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    /* Clear backbuffer to spaces on black */
    for(int i=0;i<SCREEN_W*SCREEN_H;i++){ g_buf[i].ch = L' '; g_buf[i].fg = 7; g_buf[i].bg = 0; }
    return 0;
}

void ce_shutdown(void){
    if(g_memDC){
        SelectObject(g_memDC, g_oldFont);
        SelectObject(g_memDC, g_oldBmp);
        DeleteObject(g_tileFont);
        DeleteObject(g_dib);
        DeleteDC(g_memDC);
    }
    if(g_screenDC) ReleaseDC(g_hwnd, g_screenDC);
    if(g_hwnd) DestroyWindow(g_hwnd);
    UnregisterClassW(g_className, GetModuleHandleW(NULL));
}
void ce_quit(void){ ce_running = 0; }

/* ---------- Render primitives (delegate giua cac cap, OOB clip) ---------- */
void ce_clear(int col){
    Cell c = { L' ', 7, (unsigned char)col };
    for(int i=0;i<SCREEN_W*SCREEN_H;i++) g_buf[i] = c;
}
void ce_clear_black(void){ ce_clear(CE_BLACK); }

void ce_put(int x, int y, WCHAR ch, int frg, int bkg){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return;
    Cell *c = &g_buf[y*SCREEN_W + x];
    c->ch = ch; c->fg = (unsigned char)frg; c->bg = (unsigned char)bkg;
}
void ce_putc(int x, int y, WCHAR ch, int frg){ ce_put(x, y, ch, frg, CE_BLACK); }

void ce_fill(int x0,int y0,int x1,int y1, WCHAR ch, int frg, int bkg){
    for(int y=y0;y<y1;y++) for(int x=x0;x<x1;x++) ce_put(x,y,ch,frg,bkg);
}
void ce_fillbg(int x0,int y0,int x1,int y1, int bkg){
    ce_fill(x0,y0,x1,y1,(WCHAR)L' ', CE_BLACK, bkg);
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
    for(int y=-r; y<=r; y++)
        for(int x=-r; x<=r; x++){
            float d = (x*x) + (y*y)/(aspect*aspect);
            if(d <= r*r + r*0.5f) ce_putc(cx+x, cy+y, ch, fr);
        }
}
void ce_circleOutline(int cx, int cy, int r, WCHAR ch, int fr, float aspect){
    for(int y=-r; y<=r; y++)
        for(int x=-r; x<=r; x++){
            float d = (x*x) + (y*y)/(aspect*aspect);
            if(d <= r*r + r*0.5f && d >= r*r - r*1.5f) ce_putc(cx+x, cy+y, ch, fr);
        }
}
void ce_fillCircle(int cx, int cy, int r, WCHAR ch, int fr, float aspect){
    if(r < 0) return;
    for(int y=-r; y<=r; y++)
        for(int x=-r; x<=r; x++){
            float d = x*x + (y*y)/(aspect*aspect);
            if(d <= r*r + r) ce_putc(cx+x, cy+y, ch, fr);
        }
}
void ce_sphere(int cx, int cy, int r, int baseCol, int hlCol, float aspect){
    static const int shadeChars[4] = {0x2591, 0x2592, 0x2593, 0x2588};
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
void ce_sprite_multi(int x0, int y0, int w, int h, const CESpriteCell *cells){
    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++){
            const CESpriteCell *c = &cells[y*w + x];
            if(c->ch == 0) continue;
            ce_put(x0+x, y0+y, c->ch, c->fg, c->bg);
        }
}

/* ---------- Cell query ---------- */
WCHAR ce_getch(int x, int y){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return ' ';
    return g_buf[y*SCREEN_W + x].ch;
}
int ce_getattr(int x, int y){
    if((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return 0;
    Cell *c = &g_buf[y*SCREEN_W + x];
    return (int)(c->fg | (c->bg << 4));
}

/* ---------- Flip: render backbuf -> memDC -> screen ---------- */
void ce_flip(void){
    /* Drain messages (WM_PAINT etc.) */
    MSG msg;
    while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)){
        if(msg.message == WM_QUIT){ ce_running = 0; break; }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    /* Sweep backbuf, draw each cell */
    RECT cellRect;
    for(int y=0;y<SCREEN_H;y++){
        for(int x=0;x<SCREEN_W;x++){
            Cell *c = &g_buf[y*SCREEN_W + x];
            cellRect.left = x*g_cellW; cellRect.top = y*g_cellH;
            cellRect.right = cellRect.left + g_cellW; cellRect.bottom = cellRect.top + g_cellH;
            SetTextColor(g_memDC, CE_RGB[c->fg & 15]);
            SetBkColor(g_memDC, CE_RGB[c->bg & 15]);
            ExtTextOutW(g_memDC, cellRect.left, cellRect.top, ETO_OPAQUE|ETO_CLIPPED,
                        &cellRect, &c->ch, 1, NULL);
        }
    }
    /* Present to screen */
    BitBlt(g_screenDC, 0,0, g_windowW, g_windowH, g_memDC, 0,0, SRCCOPY);
}

/* ---------- Input ---------- */
void ce_inputUpdate(void){
    for(int i=0;i<256;i++){
        int held = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
        if(held && !g_prevKeys[i])      g_keys[i] = 2;
        else if(held)                    g_keys[i] = 1;
        else                             g_keys[i] = 0;
    }
    /* Mouse buttons: edge 2 chi khi WM_xBUTTONDOWN set, sau do demote 1 */
    for(int i=0;i<3;i++){
        int vk = (i==0)?VK_LBUTTON:(i==1)?VK_RBUTTON:VK_MBUTTON;
        int held = (GetAsyncKeyState(vk) & 0x8000) ? 1 : 0;
        if(g_mouseBtn[i]==2 && held) g_mouseBtn[i] = 1;  /* edge -> held */
        else if(g_mouseBtn[i]==2 && !held) g_mouseBtn[i] = 0;
        if(held && g_mouseBtn[i]==0) g_mouseBtn[i] = 1;
    }
}
void ce_inputEnd(void){
    for(int i=0;i<256;i++) g_prevKeys[i] = (g_keys[i]!=0);
    for(int i=0;i<3;i++)  g_prevMouseBtn[i] = (g_mouseBtn[i]!=0);
}
int ce_key(int vk){ return g_keys[vk]; }
int ce_keyPressed(int vk){ return g_keys[vk]==2; }
int ce_keyDown(int vk){ return g_keys[vk]!=0; }
int ce_mouseClicked(int btn){ return g_mouseBtn[btn]==2; }
int ce_mouseDown(int btn){ return g_mouseBtn[btn]!=0; }
int ce_clickedBox(int x0, int y0, int x1, int y1){
    if(g_mouseBtn[0]!=2) return 0;
    return (ce_mouseX>=x0 && ce_mouseX<=x1 && ce_mouseY>=y0 && ce_mouseY<=y1);
}
int ce_hoverBox(int x0, int y0, int x1, int y1){
    return (ce_mouseX>=x0 && ce_mouseX<=x1 && ce_mouseY>=y0 && ce_mouseY<=y1);
}

/* ---------- Main loop ---------- */
void ce_run(void (*game_update)(float dt)){
    if(ce_init()){
        MessageBoxA(NULL, "ce_init() that bai.", "Loi", MB_OK);
        return;
    }
    double targetFrame = 1.0 / TARGET_FPS;
    double lastTime = ce_now() - g_startTime;
    timeBeginPeriod(1);
    while(ce_running){
        double now = ce_now() - g_startTime;
        float dt = (float)(now - lastTime);
        lastTime = now;
        if(dt > 0.1f) dt = 0.1f;

        ce_inputUpdate();
        game_update(dt);
        ce_flip();
        ce_inputEnd();

        /* Hybrid sleep + spin-wait */
        double elapsed = (ce_now()-g_startTime) - now;
        double wait = targetFrame - elapsed;
        if(wait > 0.002){
            Sleep((DWORD)((wait-0.002)*1000.0));
            while((ce_now()-g_startTime) - now < targetFrame){ /* spin */ }
        }
    }
    timeEndPeriod(1);
    ce_shutdown();
}
