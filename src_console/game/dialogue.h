/* =====================================================================
   DIALOGUE - Message box engine + data cho NPC/DM dialogue.
   Song ngu VI/EN qua g_lang (match i18n.h pattern).
   Features: messagebox overlay, typewriter effect, linear advance.
   ===================================================================== */
#ifndef CE_DIALOGUE_H
#define CE_DIALOGUE_H

#include "../engine/console.h"

/* ---------- Data structures ---------- */

/* 1 dòng dialogue: speaker + text, mỗi cái có cặp VI/EN */
typedef struct {
    const char *speaker_vi;   /* "DM" / "Sildar" / "Halia" */
    const char *speaker_en;
    const char *text_vi;      /* nội dung, UTF-8 có dấu */
    const char *text_en;
} DlgLine;

/* 1 cuộc hội thoại: nhiều DlgLine + title */
typedef struct {
    int id;                   /* DlgId enum */
    const char *title_vi;     /* "Phần mở đầu" */
    const char *title_en;
    const DlgLine *lines;
    int n_lines;
} Dialogue;

/* ---------- DlgId enum (data/ dialogues.c dùng) ---------- */
typedef enum {
    DLG_NONE = 0,
    DLG_DM_INTRO,             /* DM kể intro LMoP */
    DLG_SILDAR_RESCUE,        /* Sildar khi cứu ở Cragmaw */
    DLG_GUNDREN_RESCUE,       /* Gundren khi cứu ở Cragmaw Castle */
    DLG_BARTHEN,              /* Elmar Barthen - shopkeeper */
    DLG_LINENE,               /* Linene Graywind - weapon shop */
    DLG_STONEHILL,            /* Toblen Stonehill - innkeeper */
    DLG_EDERMATH,             /* Daran Edermath - Order of Gauntlet */
    DLG_HALIA,                /* Halia Thornton - Zhentarim */
    DLG_QELLENE,              /* Qellene Alderleaf - farmer */
    DLG_GARAELE,              /* Sister Garaele - Harpers */
    DLG_HARBIN,               /* Harbin Wester - townmaster */
    DLG_IARNO_LETTER,         /* Bức thư Black Spider cho Iarno */
    DLG_REIDOTH,              /* Reidoth - druid */
    DLG_AGATHA,               /* Agatha - banshee */
    DLG_NEZZNAR_BOSS,         /* Nezznar - boss cuối */
    DLG_KLARG,                /* Klarg - bugbear Cragmaw */
    DLG_KING_GROL,            /* King Grol - Cragmaw Castle */
    DLG_VENOMFANG,            /* Venomfang - dragon Thundertree */
    DLG_MIRNA,                /* Mirna Dendrar - rescue */
    DLG_DROOP,                /* Droop - goblin Information */
    DLG_NART,                 /* Narth - old farmer rumor */
    DLG_COUNT
} DlgId;

/* Bang dialogue (defined trong data/dialogues.c) - array of pointers */
extern const Dialogue * const DIALOGUES[DLG_COUNT];

/* ---------- Engine API ---------- */

/* Bắt đầu 1 dialogue. Caller tự set g_state = ST_DIALOGUE sau khi gọi. */
void dlg_start(const Dialogue *d);

/* Update mỗi frame: typewriter + input (SPACE=next, ESC=dismiss). */
void dlg_update(float dt);

/* Render messagebox overlay tại (ox, oy). Vẽ box + title + speaker + text. */
void dlg_render(int ox, int oy);

/* Đang active? (có dialogue đang chạy không) */
int dlg_active(void);

/* Đóng dialogue hiện tại. */
void dlg_close(void);

/* Helpers song ngữ: lấy text theo g_lang */
const char *dlg_speaker(const DlgLine *l);
const char *dlg_text(const DlgLine *l);

#endif /* CE_DIALOGUE_H */
