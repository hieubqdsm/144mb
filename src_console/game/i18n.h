/* =====================================================================
   I18N - Bang chuoi 2 ngon ngu (Viet / Eng) cho UI.
   Dung macro T(key) de lay chuoi theo ngon ngu hien tai.
   Chuoi luu dang UTF-8 (render qua ce_text da UTF-8-aware).
   ===================================================================== */
#ifndef CE_I18N_H
#define CE_I18N_H

/* Ngon ngu */
typedef enum { LANG_VI = 0, LANG_EN = 1, LANG_COUNT } Lang;

/* Ngon ngu hien tai (global, modify qua i18n_cycle) */
extern Lang g_lang;

/* Cac key string (them key moi = them dong vao bang duoi) */
typedef enum {
    /* Menu chinh */
    S_TITLE,            /* "DUNGEON CRAWLER" / art name */
    S_SUBTITLE,         /* "ASCII roguelike 1.44MB" */
    S_CONTINUE,         /* "TIẾP TỤC" / "CONTINUE" */
    S_NEW_GAME,         /* "CHƠI MỚI" / "NEW GAME" */
    S_OPTIONS,          /* "TÙY CHỌN" / "OPTIONS" */
    S_QUIT,             /* "THOÁT" / "QUIT" */
    /* Menu - chung */
    S_BACK,             /* "QUAY LẠI" / "BACK" */
    S_ESC_HINT,         /* "ESC: thoát" / "ESC: quit" */
    S_LANG_HINT,        /* "L: đổi ngôn ngữ" / "L: switch language" */
    /* Options */
    S_LANG_LABEL,       /* "Ngôn ngữ:" / "Language:" */
    S_LANG_VI,          /* "Tiếng Việt" */
    S_LANG_EN,          /* "English" */
    S_ABOUT,            /* "GIỚI THIỆU" / "ABOUT" */
    /* About */
    S_ABOUT_LINE1,      /* ten game */
    S_ABOUT_LINE2,      /* mo ta */
    S_ABOUT_LINE3,      /* credit */
    /* Class select */
    S_CHOOSE_CLASS,     /* "CHỌN NGHỀ" / "CHOOSE YOUR CLASS" */
    S_FIGHTER,          /* "CHIẾN BINH (SỨC MẠNH)" */
    S_MAGE,             /* "PHÁP SƯ (TRÍ TUỆ)" */
    S_CLASS_START,      /* "Cả 2 đều bắt đầu với:" / "Both start with:" */
    S_CLASS_GEAR,       /* "- Kiếm dài, 3 Thuốc hồi máu" */
    /* Pause */
    S_PAUSE_TITLE,      /* "TẠM DỪNG" / "PAUSED" */
    S_RESUME,           /* "TIẾP TỤC CHƠI" / "RESUME" */
    S_SAVE_QUIT,        /* "LƯU & THOÁT" / "SAVE & QUIT" */
    S_QUIT_NOSAVE,      /* "THOÁT (KHÔNG LƯU)" / "QUIT (NO SAVE)" */
    /* Save status */
    S_SAVE_OK,          /* "Đã lưu!" */
    S_SAVE_FAIL,        /* "Lưu thất bại!" */
    S_LOAD_FAIL,        /* "Tải game thất bại!" */
    S_NO_SAVE,          /* "Không có game đã lưu" */
    /* Dead */
    S_YOU_DIED,         /* "BẠN ĐÃ CHẾT" / "YOU DIED" */
    S_RESTART_HINT,     /* "R: chơi lại | ESC: menu" */
    /* HUD sidebar */
    S_HERO,             /* "=== ANH HÙNG ===" */
    S_LEVEL_FMT,        /* "Cấp: %d" / "Level: %d" */
    S_DEPTH_FMT,        /* "Tầng: %d" / "Depth: %d" */
    S_MONSTERS_FMT,     /* "Quái: %d" / "Monsters: %d" */
    S_POTIONS_FMT,      /* "Thuốc: %d" / "Potions: %d" */
    S_CONTROLS,         /* "=== ĐIỀU KHIỂN ===" */
    S_CTRL_MOVE,        /* "WASD: di/tấn công" */
    S_CTRL_SPELL1,      /* "1: Mũi lửa" / "1: Fire Bolt" */
    S_CTRL_SPELL2,      /* "2: Tên phép" / "2: Magic Missile" */
    S_CTRL_POTION,      /* "H: uống thuốc" / "H: use potion" */
    S_CTRL_INV,         /* "I: túi đồ" / "I: inventory" */
    S_CTRL_STAIRS,      /* ">: xuống tầng" / ">: descend stairs" */
    /* Version */
    S_VERSION,          /* "v0.1.44" */
    S_KEY_COUNT
} StrKey;

/* Bang chuoi [key][ngon ngu]. UTF-8 (build voi /utf-8). */
static const char *g_strings[S_KEY_COUNT][LANG_COUNT] = {
    /* Menu chinh */
    [S_TITLE]       = { "DUNGEON CRAWLER",        "DUNGEON CRAWLER" },
    [S_SUBTITLE]    = { "ASCII roguelike 1.44MB", "ASCII roguelike 1.44MB" },
    [S_CONTINUE]    = { "TIẾP TỤC",               "CONTINUE" },
    [S_NEW_GAME]    = { "CHƠI MỚI",               "NEW GAME" },
    [S_OPTIONS]     = { "TÙY CHỌN",               "OPTIONS" },
    [S_QUIT]        = { "THOÁT",                  "QUIT" },
    /* Menu - chung */
    [S_BACK]        = { "QUAY LẠI",               "BACK" },
    [S_ESC_HINT]    = { "ESC: thoát",             "ESC: quit" },
    [S_LANG_HINT]   = { "L: đổi ngôn ngữ",        "L: switch language" },
    /* Options */
    [S_LANG_LABEL]  = { "Ngôn ngữ:",              "Language:" },
    [S_LANG_VI]     = { "Tiếng Việt",             "Tiếng Việt" },
    [S_LANG_EN]     = { "English",                "English" },
    [S_ABOUT]       = { "GIỚI THIỆU",             "ABOUT" },
    /* About */
    [S_ABOUT_LINE1] = { "ASCII DUNGEON CRAWLER",  "ASCII DUNGEON CRAWLER" },
    [S_ABOUT_LINE2] = { "Game RPG D&D-lite cho thử thách 1.44MB",
                        "D&D-lite RPG for the 1.44MB challenge" },
    [S_ABOUT_LINE3] = { "C thuần + Win32 GDI · 2026",
                        "Pure C + Win32 GDI · 2026" },
    /* Class select */
    [S_CHOOSE_CLASS]= { "CHỌN NGHỀ CỦA BẠN",      "CHOOSE YOUR CLASS" },
    [S_FIGHTER]     = { "CHIẾN BINH (SỨC MẠNH)",   "FIGHTER (STR)" },
    [S_MAGE]        = { "PHÁP SƯ (TRÍ TUỆ)",       "MAGE (INT)" },
    [S_CLASS_START] = { "Cả 2 đều bắt đầu với:",   "Both classes start with:" },
    [S_CLASS_GEAR]  = { "- Kiếm dài, 3 Thuốc hồi máu",
                        "- Longsword, 3 Healing Potions" },
    /* Pause */
    [S_PAUSE_TITLE] = { "TẠM DỪNG",                "PAUSED" },
    [S_RESUME]      = { "TIẾP TỤC CHƠI",           "RESUME" },
    [S_SAVE_QUIT]   = { "LƯU & THOÁT",             "SAVE & QUIT" },
    [S_QUIT_NOSAVE] = { "THOÁT (KHÔNG LƯU)",       "QUIT (NO SAVE)" },
    /* Save status */
    [S_SAVE_OK]     = { "Đã lưu game!",            "Game saved!" },
    [S_SAVE_FAIL]   = { "Lưu thất bại!",           "Save failed!" },
    [S_LOAD_FAIL]   = { "Tải game thất bại!",      "Load failed!" },
    [S_NO_SAVE]     = { "Không có game đã lưu",    "No saved game" },
    /* Dead */
    [S_YOU_DIED]    = { "BẠN ĐÃ CHẾT",             "YOU DIED" },
    [S_RESTART_HINT]= { "R: chơi lại | ESC: menu", "R: restart | ESC: menu" },
    /* HUD sidebar */
    [S_HERO]        = { "=== ANH HÙNG ===",        "=== HERO ===" },
    [S_LEVEL_FMT]   = { "Cấp: %d",                 "Level: %d" },
    [S_DEPTH_FMT]   = { "Tầng: %d",                "Depth: %d" },
    [S_MONSTERS_FMT]= { "Quái: %d",                "Monsters: %d" },
    [S_POTIONS_FMT] = { "Thuốc: %d",               "Potions: %d" },
    [S_CONTROLS]    = { "=== ĐIỀU KHIỂN ===",      "=== CONTROLS ===" },
    [S_CTRL_MOVE]   = { "WASD: di/tấn công",       "WASD: move/attack" },
    [S_CTRL_SPELL1] = { "1: Mũi lửa",              "1: Fire Bolt" },
    [S_CTRL_SPELL2] = { "2: Tên phép",             "2: Magic Missile" },
    [S_CTRL_POTION] = { "H: uống thuốc",           "H: use potion" },
    [S_CTRL_INV]    = { "I: túi đồ",               "I: inventory" },
    [S_CTRL_STAIRS] = { ">: xuống tầng",           ">: descend stairs" },
    /* Version */
    [S_VERSION]     = { "v0.1.44",                 "v0.1.44" },
};

/* Macro lay chuoi theo ngon ngu hien tai */
#define T(key) (g_strings[(key)][g_lang])

/* Toggle VI <-> EN */
static inline void i18n_cycle(void){
    g_lang = (g_lang == LANG_VI) ? LANG_EN : LANG_VI;
}

#endif /* CE_I18N_H */
