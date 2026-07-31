/* =====================================================================
   FOV - Recursive shadowcasting field-of-view.
   Danh dau tat ca cell nhin thay duoc tu (cx,cy) voi ban kinh radius
   tren map (doc map.transparent[]). Goi map_mark_visible() cho moi cell.
   Thuat toan chuan roguelike (Bjorn Bergstrom).
   ===================================================================== */
#ifndef CE_FOV_H
#define CE_FOV_H

#include "map.h"

void fov_compute(Map *m, int cx, int cy, int radius);

#endif /* CE_FOV_H */
