/* =====================================================================
   DISTANCE - Implementation
   ===================================================================== */
#include "distance.h"
#include "actor.h"

int dist_chebyshev(const Actor *a, const Actor *b){
    int dx = a->x - b->x; if(dx < 0) dx = -dx;
    int dy = a->y - b->y; if(dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

int dist_manhattan(const Actor *a, const Actor *b){
    int dx = a->x - b->x; if(dx < 0) dx = -dx;
    int dy = a->y - b->y; if(dy < 0) dy = -dy;
    return dx + dy;
}

int dist_feet(const Actor *a, const Actor *b){
    return dist_chebyshev(a, b) * 5;
}

int dist_in_melee(const Actor *a, const Actor *b){
    return dist_chebyshev(a, b) <= 1;
}

int dist_in_range(const Actor *a, const Actor *b, int range_ft){
    if(range_ft <= 0) return dist_in_melee(a, b);
    return dist_feet(a, b) <= range_ft;
}

int dist_threatens(const Actor *attacker, const Actor *target){
    if(!attacker || !target) return 0;
    if(actor_is_dead(attacker)) return 0;
    if(actor_is_dead(target)) return 0;
    if(attacker->team == target->team) return 0;
    return dist_in_melee(attacker, target);
}
