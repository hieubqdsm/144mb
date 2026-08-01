/* =====================================================================
   XP RENDER - Ve .xp sprite len GDI memDC (RGB full color, 16-color limit).
   VÌ .xp co RGB 24-bit nhung engine 16-mau: quantize RGB -> CE_Color gan nhat.
   Ve truc tiep len backbuf cell (dung ce_put), KHONG pha voi game 16-color.
   ===================================================================== */
#ifndef CE_XP_RENDER_H
#define CE_XP_RENDER_H

#include "xp_loader.h"

/* Ve 1 layer cua .xp sprite tai cell (ox, oy).
   Transparent cell (bg magenta hoac cp=0) duoc skip.
   RGB duoc quantize ve 16 CE_Color (lossy nhung khong vo). */
void xp_draw_layer(int ox, int oy, const XpLayer *layer);

/* Ve toan bo .xp file (composite tat ca layers bottom-up) tai (ox, oy). */
void xp_draw_file(int ox, int oy, const XpFile *f);

/* Quantize RGB -> CE_Color index (0-15). */
int xp_rgb_to_ce(int r, int g, int b);

#endif /* CE_XP_RENDER_H */
