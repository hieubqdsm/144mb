/* =====================================================================
   BSP - Dungeon generation (rooms + corridors).
   Don gian hon BSP day du: dat N phong ngau nhien khong overlap,
   noi lan Phong i <-> Phong i-1 bang corridor L-shape.
   ===================================================================== */
#ifndef CE_BSP_H
#define CE_BSP_H

#include "map.h"
#include "rng.h"

typedef struct {
    int x, y, w, h;
    int cx, cy;   /* center */
} Room;

/* Sinh dungeon vao map m. Dat n_rooms phong (gia tri duoc chia cho 3).
   Tra ve so phong thuc su da dat. rooms[] nhan danh sach phong (caller cap phat).
   Tra ve -1 neu loi. */
int bsp_generate(Map *m, RNG *rng, int n_rooms, Room *rooms_out);

#endif /* CE_BSP_H */
