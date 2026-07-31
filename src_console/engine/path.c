/* =====================================================================
   PATH - A* implementation (binary heap don gian, Manhattan heuristic).
   Chu y: A* classic, khong toi uu kich thuoc nhung de hieu.
   ===================================================================== */
#include "path.h"
#include <stdlib.h>
#include <string.h>

/* Node trong grid */
typedef struct {
    int x, y;
    int g;        /* cost tu start */
    int f;        /* g + heuristic */
    int parent;   /* index vao mang nodes, -1 = root */
    int heap_idx; /* vi tri hien tai trong heap */
    int closed;
    int opened;
} Node;

/* Binary min-heap theo f */
typedef struct {
    int *data;    /* index vao nodes */
    int size;
} MinHeap;

static void heap_push(MinHeap *h, Node *nodes, int ni){
    int i = h->size++;
    h->data[i] = ni;
    nodes[ni].heap_idx = i;
    while(i > 0){
        int parent = (i-1)/2;
        if(nodes[h->data[i]].f < nodes[h->data[parent]].f){
            int t = h->data[i]; h->data[i] = h->data[parent]; h->data[parent] = t;
            nodes[h->data[i]].heap_idx = i;
            nodes[h->data[parent]].heap_idx = parent;
            i = parent;
        } else break;
    }
}

static int heap_pop(MinHeap *h, Node *nodes){
    int result = h->data[0];
    h->size--;
    if(h->size > 0){
        h->data[0] = h->data[h->size];
        nodes[h->data[0]].heap_idx = 0;
        int i = 0;
        for(;;){
            int l = 2*i+1, r = 2*i+2, smallest = i;
            if(l < h->size && nodes[h->data[l]].f < nodes[h->data[smallest]].f) smallest = l;
            if(r < h->size && nodes[h->data[r]].f < nodes[h->data[smallest]].f) smallest = r;
            if(smallest == i) break;
            int t = h->data[i]; h->data[i] = h->data[smallest]; h->data[smallest] = t;
            nodes[h->data[i]].heap_idx = i;
            nodes[h->data[smallest]].heap_idx = smallest;
            i = smallest;
        }
    }
    nodes[result].heap_idx = -1;
    return result;
}

static void heap_decrease(MinHeap *h, Node *nodes, int ni){
    int i = nodes[ni].heap_idx;
    while(i > 0){
        int parent = (i-1)/2;
        if(nodes[h->data[i]].f < nodes[h->data[parent]].f){
            int t = h->data[i]; h->data[i] = h->data[parent]; h->data[parent] = t;
            nodes[h->data[i]].heap_idx = i;
            nodes[h->data[parent]].heap_idx = parent;
            i = parent;
        } else break;
    }
}

static int manhattan(int x0,int y0,int x1,int y1){
    int dx = abs(x0-x1), dy = abs(y0-y1);
    return dx + dy;
}

Path *path_find(const Map *m, int x0, int y0, int x1, int y1, int max_steps){
    if(!map_walkable(m, x1, y1)) return NULL;
    int W = m->w, H = m->h;
    size_t total = (size_t)W * H;
    Node *nodes = (Node*)malloc(total * sizeof(Node));
    MinHeap heap; heap.data = (int*)malloc(total * sizeof(int)); heap.size = 0;
    if(!nodes || !heap.data){ free(nodes); free(heap.data); return NULL; }
    for(size_t i=0;i<total;i++){
        nodes[i].opened = 0; nodes[i].closed = 0; nodes[i].heap_idx = -1;
    }

    int start = y0*W + x0, goal = y1*W + x1;
    nodes[start].x = x0; nodes[start].y = y0; nodes[start].g = 0;
    nodes[start].f = manhattan(x0,y0,x1,y1); nodes[start].parent = -1;
    nodes[start].opened = 1;
    heap_push(&heap, nodes, start);

    static const int DX[4] = { 0, 1, 0, -1 };
    static const int DY[4] = { -1, 0, 1, 0 };
    /* (extension point: them 8 huong cheo neu muon) */

    Path *result = NULL;
    int steps_taken = 0;
    while(heap.size > 0 && steps_taken < max_steps){
        int cur = heap_pop(&heap, nodes);
        nodes[cur].closed = 1;
        steps_taken++;
        if(cur == goal){
            /* Reconstruct path */
            int n = 0;
            for(int c = cur; c != start; c = nodes[c].parent) n++;
            Path *p = (Path*)malloc(sizeof(Path));
            p->count = n;
            p->steps = (Step*)malloc(n * sizeof(Step));
            int idx = n - 1;
            for(int c = cur; c != start; c = nodes[c].parent){
                p->steps[idx].x = nodes[c].x;
                p->steps[idx].y = nodes[c].y;
                idx--;
            }
            result = p;
            break;
        }
        for(int d = 0; d < 4; d++){
            int nx = nodes[cur].x + DX[d];
            int ny = nodes[cur].y + DY[d];
            if(!map_walkable(m, nx, ny)) continue;
            int ni = ny*W + nx;
            if(nodes[ni].closed) continue;
            int tentative_g = nodes[cur].g + 1;
            if(!nodes[ni].opened || tentative_g < nodes[ni].g){
                nodes[ni].x = nx; nodes[ni].y = ny;
                nodes[ni].g = tentative_g;
                nodes[ni].f = tentative_g + manhattan(nx,ny,x1,y1);
                nodes[ni].parent = cur;
                if(!nodes[ni].opened){
                    nodes[ni].opened = 1;
                    heap_push(&heap, nodes, ni);
                } else {
                    heap_decrease(&heap, nodes, ni);
                }
            }
        }
    }
    free(nodes); free(heap.data);
    return result;
}

void path_free(Path *p){
    if(!p) return;
    free(p->steps);
    free(p);
}
