/* =====================================================================
   XP LOADER - Implementation
   ===================================================================== */
#include "xp_loader.h"
#include "inflate.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

/* Doc file vao buffer. Tra ve malloc'd buffer + len. NULL neu loi. */
static unsigned char *read_file(const char *path, size_t *len){
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) return NULL;
    LARGE_INTEGER sz;
    if(!GetFileSizeEx(h, &sz)){ CloseHandle(h); return NULL; }
    size_t n = (size_t)sz.QuadPart;
    unsigned char *buf = (unsigned char*)malloc(n);
    if(!buf){ CloseHandle(h); return NULL; }
    DWORD rd = 0;
    if(!ReadFile(h, buf, (DWORD)n, &rd, NULL) || rd != n){
        free(buf); CloseHandle(h); return NULL;
    }
    CloseHandle(h);
    *len = n;
    return buf;
}

/* Doc int32 little-endian tu buffer tai offset. */
static int32_t read_i32(const unsigned char *p, size_t pos, size_t max){
    if(pos + 4 > max) return 0;
    return (int32_t)(p[pos] | (p[pos+1]<<8) | (p[pos+2]<<16) | (p[pos+3]<<24));
}

XpFile *xp_load(const char *path){
    size_t file_len = 0;
    unsigned char *raw = read_file(path, &file_len);
    if(!raw) return NULL;

    /* Giai nen gzip */
    size_t data_len = 0;
    unsigned char *data = inflate_gzip(raw, file_len, &data_len);
    free(raw);
    if(!data) return NULL;

    XpFile *f = (XpFile*)calloc(1, sizeof(XpFile));
    if(!f){ free(data); return NULL; }

    size_t pos = 0;
    f->version = read_i32(data, pos, data_len); pos += 4;
    f->layer_count = read_i32(data, pos, data_len); pos += 4;
    if(f->layer_count < 1 || f->layer_count > 10){ free(data); xp_free(f); return NULL; }

    f->layers = (XpLayer*)calloc(f->layer_count, sizeof(XpLayer));
    if(!f->layers){ free(data); xp_free(f); return NULL; }

    for(int L = 0; L < f->layer_count; L++){
        XpLayer *layer = &f->layers[L];
        layer->w = read_i32(data, pos, data_len); pos += 4;
        layer->h = read_i32(data, pos, data_len); pos += 4;
        if(layer->w < 1 || layer->h < 1 || layer->w > 1024 || layer->h > 1024){
            free(data); xp_free(f); return NULL;
        }
        layer->cells = (XpCell*)calloc((size_t)layer->w * layer->h, sizeof(XpCell));
        if(!layer->cells){ free(data); xp_free(f); return NULL; }
        /* REXpaint: COLUMN-MAJOR (X outer, Y inner). Chuyen sang row-major. */
        for(int x = 0; x < layer->w; x++){
            for(int y = 0; y < layer->h; y++){
                if(pos + 10 > data_len){ free(data); xp_free(f); return NULL; }
                XpCell c;
                c.cp = read_i32(data, pos, data_len); pos += 4;
                c.fr = data[pos++]; c.fg = data[pos++]; c.fb = data[pos++];
                c.br = data[pos++]; c.bg = data[pos++]; c.bb = data[pos++];
                /* Store row-major: [y*w + x] */
                layer->cells[(size_t)y * layer->w + x] = c;
            }
        }
    }
    free(data);
    return f;
}

void xp_free(XpFile *f){
    if(!f) return;
    if(f->layers){
        for(int i = 0; i < f->layer_count; i++){
            free(f->layers[i].cells);
        }
        free(f->layers);
    }
    free(f);
}

int xp_cell_transparent(const XpCell *c){
    /* Transparent neu bg la magenta (255,0,255) hoac codepoint = 0 */
    if(c->cp == 0) return 1;
    if(c->br == 255 && c->bg == 0 && c->bb == 255) return 1;
    return 0;
}
