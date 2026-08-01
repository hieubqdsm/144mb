/* =====================================================================
   XP LOADER - Load REXpaint .xp files (gzip + column-major cells).
   Format spec tu gridsagegames.com/rexpaint/manual.txt:
     gzip( header(8B): version i32, layer_count i32;
           per layer: width i32, height i32, width*height cells;
           cell(10B): codepoint i32 + fg_rgb(3B) + bg_rgb(3B) )
   Transparency = bg magenta (255,0,255).
   ===================================================================== */
#ifndef CE_XP_LOADER_H
#define CE_XP_LOADER_H

#include <stdint.h>

typedef struct {
    int32_t cp;         /* codepoint (0 = empty) */
    uint8_t fr, fg, fb; /* foreground RGB */
    uint8_t br, bg, bb; /* background RGB */
} XpCell;

typedef struct {
    int32_t w, h;
    XpCell *cells;      /* indexed [y*w + x] (chuyen tu column-major sang row-major) */
} XpLayer;

typedef struct {
    int32_t version;
    int32_t layer_count;
    XpLayer *layers;
} XpFile;

/* Load .xp tu file path. Tra ve NULL neu loi. Caller goi xp_free(). */
XpFile *xp_load(const char *path);

/* Free .xp data. */
void xp_free(XpFile *f);

/* Helper: cell co transparent khong? (bg magenta 255,0,255 hoac cp==0) */
int xp_cell_transparent(const XpCell *c);

#endif /* CE_XP_LOADER_H */
