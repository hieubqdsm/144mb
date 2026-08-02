/* =====================================================================
   DIALOGUE - Implementation (engine: typewriter + messagebox + input)
   ===================================================================== */
#include "dialogue.h"
#include "i18n.h"
#include <string.h>
#include <stdio.h>

/* ---------- Internal state ---------- */
static const Dialogue *g_dlg = NULL;   /* dialogue hiện tại, NULL = inactive */
static int g_line_idx = 0;             /* dòng hiện tại trong dialogue */
static int g_char_idx = 0;             /* số byte đã hiện (typewriter) */
static int g_line_done = 0;            /* 1 = dòng đã hiện hết, chờ next */
static float g_type_timer = 0;         /* accumulogle cho typewriter speed */

/* Typewriter: mỗi TYPE_SPEED giây hiện thêm 1 byte */
#define TYPE_SPEED 0.02f   /* 50 char/giây */
#define BOX_W 70
#define BOX_H 8
#define BOX_TEXT_W (BOX_W - 4)   /* chiều rộng text khả dụng = 66 */

/* ---------- Helpers ---------- */
const char *dlg_speaker(const DlgLine *l){
    if(!l) return "";
    return (g_lang == LANG_VI && l->speaker_vi) ? l->speaker_vi : l->speaker_en;
}
const char *dlg_text(const DlgLine *l){
    if(!l) return "";
    return (g_lang == LANG_VI && l->text_vi) ? l->text_vi : l->text_en;
}

int dlg_active(void){ return g_dlg != NULL; }

void dlg_close(void){
    g_dlg = NULL;
    g_line_idx = 0;
    g_char_idx = 0;
    g_line_done = 0;
}

void dlg_start(const Dialogue *d){
    if(!d || d->n_lines <= 0) return;
    g_dlg = d;
    g_line_idx = 0;
    g_char_idx = 0;
    g_line_done = 0;
    g_type_timer = 0;
}

/* ---------- Update: typewriter + input ---------- */
void dlg_update(float dt){
    if(!g_dlg) return;
    const DlgLine *line = &g_dlg->lines[g_line_idx];
    const char *text = dlg_text(line);
    int text_len = (int)strlen(text);

    /* Typewriter: tăng char_idx theo thời gian */
    if(!g_line_done){
        g_type_timer += dt;
        while(g_type_timer >= TYPE_SPEED && g_char_idx < text_len){
            g_type_timer -= TYPE_SPEED;
            g_char_idx++;
        }
        if(g_char_idx >= text_len){
            g_line_done = 1;
        }
    }

    /* Input: SPACE/Enter */
    if(ce_keyPressed(VK_SPACE) || ce_keyPressed(VK_RETURN)){
        if(!g_line_done){
            /* Skip: hiện hết dòng ngay */
            g_char_idx = text_len;
            g_line_done = 1;
        } else {
            /* Next line hoặc đóng */
            g_line_idx++;
            if(g_line_idx >= g_dlg->n_lines){
                dlg_close();
            } else {
                g_char_idx = 0;
                g_line_done = 0;
                g_type_timer = 0;
            }
        }
    }
}

/* ---------- Render ---------- */
/* Vẽ 1 box viền đôi (double-line) tại (x,y) kích cỡ w×h, màu col.
   Pattern copy từ draw_pause, dùng cho dialogue. */
static void draw_box(int x, int y, int w, int h, int col){
    /* Nền tối */
    ce_fillbg(x, y, x+w, y+h, 0);
    /* Viền ngang trên/dưới */
    for(int i = 0; i < w; i++){
        ce_putc(x+i, y, (WCHAR)0x2550, col);
        ce_putc(x+i, y+h-1, (WCHAR)0x2550, col);
    }
    /* Viền dọc trái/phải */
    for(int i = 0; i < h; i++){
        ce_putc(x, y+i, (WCHAR)0x2551, col);
        ce_putc(x+w-1, y+i, (WCHAR)0x2551, col);
    }
    /* 4 góc */
    ce_putc(x,       y,       (WCHAR)0x2554, col);   /* ╔ */
    ce_putc(x+w-1,   y,       (WCHAR)0x2557, col);   /* ╗ */
    ce_putc(x,       y+h-1,   (WCHAR)0x255A, col);   /* ╚ */
    ce_putc(x+w-1,   y+h-1,   (WCHAR)0x255D, col);   /* ╝ */
}

/* Word-wrap: cắt text thành các dòng không vượt BOX_TEXT_W.
   Trả về số dòng, điền vào out_offsets[] (byte offset bắt đầu mỗi dòng)
   và out_lens[] (chiều dài byte mỗi dòng). */
static int wrap_text(const char *text, int offsets[], int lens[], int max_lines){
    int n_lines = 0;
    int i = 0;
    int len = (int)strlen(text);
    while(i < len && n_lines < max_lines){
        int start = i;
        int end = i + BOX_TEXT_W;
        if(end >= len){
            /* Còn lại < 1 dòng */
            offsets[n_lines] = start;
            lens[n_lines] = len - start;
            n_lines++;
            break;
        }
        /* Tìm space gần nhất trước end để cắt không giữa từ */
        int cut = end;
        while(cut > start && text[cut] != ' ') cut--;
        if(cut == start) cut = end;   /* từ quá dài, cắt ngang */
        offsets[n_lines] = start;
        lens[n_lines] = cut - start;
        n_lines++;
        /* Bỏ space */
        i = cut;
        while(i < len && text[i] == ' ') i++;
    }
    return n_lines;
}

void dlg_render(int ox, int oy){
    if(!g_dlg) return;
    /* ox, oy = tọa độ top-left của box trên screen */
    draw_box(ox, oy, BOX_W, BOX_H, 11);   /* cyan border */

    /* Title (nếu có) */
    const char *title = (g_lang == LANG_VI && g_dlg->title_vi) ? g_dlg->title_vi : g_dlg->title_en;
    if(title && title[0]){
        int tlen = (int)strlen(title);
        ce_text(ox + (BOX_W - tlen)/2, oy, title, 14);
    }

    /* Speaker của dòng hiện tại */
    const DlgLine *line = &g_dlg->lines[g_line_idx];
    const char *speaker = dlg_speaker(line);
    const char *text = dlg_text(line);

    /* Speaker line (row oy+1) */
    if(speaker && speaker[0]){
        ce_text(ox + 2, oy + 1, speaker, 13);   /* magenta */
    }

    /* Body: wrap text, hiện typewriter (char_idx bytes đầu) */
    int offsets[4], lens[4];
    int n_lines = wrap_text(text, offsets, lens, 4);

    /* Typewriter: chỉ hiện g_char_idx bytes đầu của toàn text */
    int chars_shown = g_char_idx;
    for(int li = 0; li < n_lines; li++){
        int row = oy + 2 + li;
        if(chars_shown <= 0) break;
        int show = (chars_shown < lens[li]) ? chars_shown : lens[li];
        if(show > 0){
            /* Copy substring (ce_text cần null-terminated) */
            char buf[80];
            int cl = show < (int)sizeof(buf)-1 ? show : (int)sizeof(buf)-1;
            memcpy(buf, text + offsets[li], cl);
            buf[cl] = 0;
            ce_text(ox + 2, row, buf, 15);   /* trắng */
        }
        chars_shown -= lens[li];
    }

    /* Hint: [SPACE: tiếp/đóng] ở góc dưới phải */
    const char *hint;
    if(g_line_done){
        hint = (g_line_idx < g_dlg->n_lines - 1) ? "[SPACE: tiếp]" : "[SPACE: đóng]";
    } else {
        hint = "[SPACE: bỏ qua]";
    }
    int hlen = (int)strlen(hint);
    ce_text(ox + BOX_W - hlen - 2, oy + BOX_H - 1, hint, 8);   /* grey */

    /* Indicator dòng: (line_idx+1)/n_lines ở góc dưới trái */
    char progress[16];
    sprintf(progress, "%d/%d", g_line_idx + 1, g_dlg->n_lines);
    ce_text(ox + 2, oy + BOX_H - 1, progress, 8);
}
