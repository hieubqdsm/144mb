/* =====================================================================
   INFLATE - Pure-C DEFLATE inflater (RFC 1951).
   Implementation don gian, tinfl-style. Dung cho .xp gzip.
   ===================================================================== */
#include "inflate.h"
#include <stdlib.h>
#include <string.h>

/* ---------- Output buffer dong (grow) ---------- */
typedef struct {
    unsigned char *data;
    size_t cap, len;
} OutBuf;

static int outbuf_init(OutBuf *o){
    o->cap = 4096; o->len = 0;
    o->data = (unsigned char*)malloc(o->cap);
    return o->data != NULL;
}
static int outbuf_putc(OutBuf *o, unsigned char c){
    if(o->len >= o->cap){
        size_t nc = o->cap * 2;
        unsigned char *nd = (unsigned char*)realloc(o->data, nc);
        if(!nd) return 0;
        o->data = nd; o->cap = nc;
    }
    o->data[o->len++] = c;
    return 1;
}
static int outbuf_write(OutBuf *o, const unsigned char *src, size_t n){
    for(size_t i=0;i<n;i++) if(!outbuf_putc(o, src[i])) return 0;
    return 1;
}

/* ---------- Bit reader (LSB first, RFC 1951) ---------- */
typedef struct {
    const unsigned char *data;
    size_t len, pos;       /* byte pos */
    unsigned bitbuf;       /* current bits */
    int bitcount;          /* bits in bitbuf */
} BitReader;

static void br_init(BitReader *b, const unsigned char *data, size_t len){
    b->data = data; b->len = len; b->pos = 0;
    b->bitbuf = 0; b->bitcount = 0;
}
static int br_need(BitReader *b, int n){
    while(b->bitcount < n){
        if(b->pos >= b->len) return 0;
        b->bitbuf |= (unsigned)(b->data[b->pos++]) << b->bitcount;
        b->bitcount += 8;
    }
    return 1;
}
static unsigned br_get(BitReader *b, int n){
    if(!br_need(b, n)) return 0;
    unsigned v = b->bitbuf & ((1u << n) - 1);
    b->bitbuf >>= n; b->bitcount -= n;
    return v;
}

/* ---------- Huffman decode ---------- */
#define MAX_BITS 15
typedef struct { unsigned short counts[MAX_BITS+1]; unsigned short symbols[288]; } HTable;

static int build_huff(HTable *t, const unsigned char *lens, int n){
    memset(t->counts, 0, sizeof(t->counts));
    for(int i=0;i<n;i++) t->counts[lens[i]]++;
    t->counts[0] = 0;
    int offs[MAX_BITS+2]; offs[1] = 0;
    for(int i=1;i<MAX_BITS;i++) offs[i+1] = offs[i] + t->counts[i];
    for(int i=0;i<n;i++){
        int l = lens[i];
        if(l) t->symbols[offs[l]++] = (unsigned short)i;
    }
    return 1;
}
static int decode_huff(BitReader *b, const HTable *t){
    int code=0, first=0, index=0;
    for(int len=1; len<=MAX_BITS; len++){
        if(!br_need(b, 1)) return -1;
        code |= br_get(b, 1);
        int count = t->counts[len];
        if(code - count < first) return t->symbols[index + (code - first)];
        index += count; first += count; first <<= 1;
        code <<= 1;
    }
    return -1;
}

/* Length/base tables (RFC 1951) */
static const unsigned char LEN_BASE[29] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
static const unsigned char LEN_EXTRA[29] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
static const unsigned char DIST_BASE[30] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
static const unsigned char DIST_EXTRA[30] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

/* Fixed huffman lens */
static void fixed_lens(unsigned char *litlen, unsigned char *dist){
    for(int i=0;i<144;i++) litlen[i]=8;
    for(int i=144;i<256;i++) litlen[i]=9;
    for(int i=256;i<280;i++) litlen[i]=7;
    for(int i=280;i<288;i++) litlen[i]=8;
    for(int i=0;i<30;i++) dist[i]=5;
}

static int inflate_block(BitReader *b, OutBuf *out, const HTable *lt, const HTable *dt){
    for(;;){
        int sym = decode_huff(b, lt);
        if(sym < 0) return 0;
        if(sym == 256) return 1;   /* end of block */
        if(sym < 256){
            if(!outbuf_putc(out, (unsigned char)sym)) return 0;
        } else {
            sym -= 257;
            if(sym >= 29) return 0;
            int length = LEN_BASE[sym] + (int)br_get(b, LEN_EXTRA[sym]);
            int dsym = decode_huff(b, dt);
            if(dsym < 0 || dsym >= 30) return 0;
            int dist = DIST_BASE[dsym] + (int)br_get(b, DIST_EXTRA[dsym]);
            if(dist > (int)out->len) return 0;
            size_t src = out->len - dist;
            for(int i=0;i<length;i++){
                if(!outbuf_putc(out, out->data[src + i])) return 0;
            }
        }
    }
}

unsigned char *inflate_raw(const unsigned char *data, size_t data_len, size_t *out_len){
    BitReader br; br_init(&br, data, data_len);
    OutBuf out; if(!outbuf_init(&out)) return NULL;
    HTable lt, dt;
    int final = 0;
    while(!final){
        if(!br_need(&br, 1)) { free(out.data); return NULL; }
        final = (int)br_get(&br, 1);
        int btype = (int)br_get(&br, 2);
        if(btype == 0){
            /* Stored: skip to byte boundary, read len/nlen, copy */
            br.bitbuf = 0; br.bitcount = 0;
            if(br.pos + 4 > br.len) { free(out.data); return NULL; }
            unsigned len = br.data[br.pos] | (br.data[br.pos+1]<<8);
            br.pos += 4;
            if(br.pos + len > br.len) { free(out.data); return NULL; }
            if(!outbuf_write(&out, br.data + br.pos, len)) { free(out.data); return NULL; }
            br.pos += len;
        } else if(btype == 1){
            unsigned char llens[288], dlens[30];
            fixed_lens(llens, dlens);
            build_huff(&lt, llens, 288);
            build_huff(&dt, dlens, 30);
            if(!inflate_block(&br, &out, &lt, &dt)) { free(out.data); return NULL; }
        } else if(btype == 2){
            if(!br_need(&br, 14)) { free(out.data); return NULL; }
            int hlit = (int)br_get(&br, 5) + 257;
            int hdist = (int)br_get(&br, 5) + 1;
            int hclen = (int)br_get(&br, 4) + 4;
            static const unsigned char ORDER[19] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
            unsigned char clens[19]; memset(clens, 0, sizeof(clens));
            for(int i=0;i<hclen;i++) clens[ORDER[i]] = (unsigned char)br_get(&br, 3);
            HTable ct; build_huff(&ct, clens, 19);
            unsigned char lens[288+30]; memset(lens, 0, sizeof(lens));
            int total = hlit + hdist, i = 0;
            while(i < total){
                int s = decode_huff(&br, &ct);
                if(s < 0) { free(out.data); return NULL; }
                if(s < 16) lens[i++] = (unsigned char)s;
                else if(s == 16){ int rep = 3 + (int)br_get(&br,2); unsigned char v = i>0?lens[i-1]:0; while(rep-- && i<total) lens[i++]=v; }
                else if(s == 17){ int rep = 3 + (int)br_get(&br,3); while(rep-- && i<total) lens[i++]=0; }
                else { int rep = 11 + (int)br_get(&br,7); while(rep-- && i<total) lens[i++]=0; }
            }
            build_huff(&lt, lens, hlit);
            build_huff(&dt, lens + hlit, hdist);
            if(!inflate_block(&br, &out, &lt, &dt)) { free(out.data); return NULL; }
        } else { free(out.data); return NULL; }
    }
    *out_len = out.len;
    return out.data;
}

unsigned char *inflate_gzip(const unsigned char *data, size_t data_len, size_t *out_len){
    /* Skip gzip header (RFC 1952): 10 bytes min + optional FNAME/FEXTRA/FCOMMENT/FHCRC */
    if(data_len < 18) return NULL;
    if(data[0] != 0x1f || data[1] != 0x8b) return NULL;   /* gzip magic */
    int flags = data[3];
    size_t pos = 10;
    if(flags & 0x04){ /* FEXTRA */
        if(pos + 2 > data_len) return NULL;
        int xlen = data[pos] | (data[pos+1]<<8); pos += 2 + xlen;
    }
    if(flags & 0x08){ /* FNAME */
        while(pos < data_len && data[pos]) pos++; pos++;
    }
    if(flags & 0x10){ /* FCOMMENT */
        while(pos < data_len && data[pos]) pos++; pos++;
    }
    if(flags & 0x02) pos += 2;  /* FHCRC */
    if(pos >= data_len) return NULL;
    /* Body = data[pos .. data_len-8] (last 8 = CRC + size) */
    size_t body_len = data_len - pos - 8;
    return inflate_raw(data + pos, body_len, out_len);
}
