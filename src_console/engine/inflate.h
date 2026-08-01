/* =====================================================================
   INFLATE - Pure-C DEFLATE inflater (tinfl-style, ~250 dòng).
   Dùng de giai nen gzip (.xp REXpaint files). KHONG external lib (zlib).
   API: inflate_gzip(data, data_len) -> malloc'd buffer + out_len.
   ===================================================================== */
#ifndef CE_INFLATE_H
#define CE_INFLATE_H

#include <stddef.h>

/* Giai nen gzip: bo 10-byte header + optional fields, inflate DEFLATE body.
   Tra ve malloc'd buffer (caller free), out_len = kich thuoc giai nen.
   Tra ve NULL neu loi. */
unsigned char *inflate_gzip(const unsigned char *data, size_t data_len, size_t *out_len);

/* Giai nen raw DEFLATE (khong gzip header). */
unsigned char *inflate_raw(const unsigned char *data, size_t data_len, size_t *out_len);

#endif /* CE_INFLATE_H */
