/* =====================================================================
   DISTANCE - Helper tính khoảng cách D&D 5e grid (5ft = 1 cell).
   Thay thế inline abs(dx)+abs(dy) rải rác ở 8 chỗ.
   ===================================================================== */
#ifndef CE_DISTANCE_H
#define CE_DISTANCE_H

#include "../structs.h"

/* Chebyshev distance (max dx,dy). Dùng cho melee adjacency.
   chebyshev <= 1 = adjacent (trong 5ft). */
int dist_chebyshev(const Actor *a, const Actor *b);

/* Manhattan distance (|dx|+|dy|). Dùng cho movement calc. */
int dist_manhattan(const Actor *a, const Actor *b);

/* Khoảng cách feet (chebyshev * 5). Dùng cho range check. */
int dist_feet(const Actor *a, const Actor *b);

/* Có trong tầm melee (5ft) không? chebyshev <= 1. */
int dist_in_melee(const Actor *a, const Actor *b);

/* Có trong tầm bắn không? range_ft=0 = melee only. */
int dist_in_range(const Actor *a, const Actor *b, int range_ft);

/* Attacker có đe dọa target (opportunity attack) không?
   = khác team + alive + melee range. */
int dist_threatens(const Actor *attacker, const Actor *target);

#endif /* CE_DISTANCE_H */
