/* =====================================================================
   PATH - A* pathfinding tren map (walkable cells).
   Tra ve path tu (x0,y0) den (x1,y1), hoac NULL neu khong tim duoc.
   Caller phai free() ket qua (field path->steps).
   ===================================================================== */
#ifndef CE_PATH_H
#define CE_PATH_H

#include "map.h"

typedef struct { int x, y; } Step;

typedef struct {
    Step *steps;   /* mang cac buoc (KHONG gom diem xuat phat) */
    int count;     /* so buoc */
} Path;

/* Tim duong. Tra ve NULL neu khong co duong. Free bang path_free(). */
Path *path_find(const Map *m, int x0, int y0, int x1, int y1, int max_steps);
void path_free(Path *p);

#endif /* CE_PATH_H */
