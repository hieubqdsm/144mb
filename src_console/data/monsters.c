/* =====================================================================
   DATA - Monster definitions (const compiled table, NetHack style).
   =====================================================================
   CACH DOC (sau refactor - tu giai thich):
     .hp_dice   = DICE(2,6,0)      <- "2d6+0"  (count, sides, mod)
     .scores    = STATS(STR(8), DEX(14), CON(10), INT(10), WIS(8), CHA(8))
     .cr        = CR(0.25)         <- Challenge Rating (luu x4 internal)
     .size      = SIZE_SMALL       <- thay so 2
     .type      = TYPE_HUMANOID    <- thay so 0
     .glyph_color = CE_RED         <- thay so 12

   THEM/SUA MONSTER = edit 1 entry nay. Khong can parser, 0 byte file.
   Stats lay tu D&D 5e SRD.
   ===================================================================== */
#include "monsters.h"
#include "../structs.h"   /* DICE, STATS, CR, SIZE_*, TYPE_*, enum macros */
#include "../engine/console.h"  /* CE_RED, CE_GREEN, ... */
#include "../game/ai.h"

/* =====================================================================
   ATTACKS (mang const, shared giua instances cung loai)
   Dung designated init de doc ro.
   ===================================================================== */

/* --- Goblin: Scimitar (melee) + Shortbow (ranged) --- */
static const MonsterAction goblin_actions[] = {
    { .name="Scimitar", .atk_bonus=4, .reach=5,  .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_SLASHING },
    { .name="Shortbow", .atk_bonus=4, .reach=80, .range_max=320,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Player: Longsword (melee) + Fire Bolt (cantrip ranged) --- */
static const MonsterAction player_actions[] = {
    { .name="Kiếm dài", .atk_bonus=5, .reach=5,   .range_max=0,
      .damage=DICE(1,8,3), .dmg_type=DMG_SLASHING },
    { .name="Tiễn lửa", .atk_bonus=5, .reach=120, .range_max=0,
      .damage=DICE(1,10,0), .dmg_type=DMG_FIRE },
};

/* --- Skeleton: Shortsword (melee) + Shortbow (ranged archer) --- */
static const MonsterAction skeleton_actions[] = {
    { .name="Shortsword", .atk_bonus=4, .reach=5,  .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
    { .name="Shortbow",   .atk_bonus=4, .reach=80, .range_max=320,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Wolf: Bite (fast melee, pack hunter) --- */
static const MonsterAction wolf_actions[] = {
    { .name="Bite", .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Orc: Greataxe (1d12+3 nguy hiem!) + Javelin (ranged) --- */
static const MonsterAction orc_actions[] = {
    { .name="Greataxe", .atk_bonus=5, .reach=5,  .range_max=0,
      .damage=DICE(1,12,3), .dmg_type=DMG_SLASHING },
    { .name="Javelin",  .atk_bonus=5, .reach=30, .range_max=120,
      .damage=DICE(1,6,3), .dmg_type=DMG_PIERCING },
};

/* --- Ogre: Greatclub (2d8+4 tanky) + Javelin --- */
static const MonsterAction ogre_actions[] = {
    { .name="Greatclub", .atk_bonus=6, .reach=5,  .range_max=0,
      .damage=DICE(2,8,4), .dmg_type=DMG_BLUDGEONING },
    { .name="Javelin",   .atk_bonus=6, .reach=30, .range_max=120,
      .damage=DICE(1,10,4), .dmg_type=DMG_PIERCING },
};

/* --- Dragon (boss): Cắn + Phun lửa (multiattack 2/turn) --- */
static const MonsterAction dragon_actions[] = {
    { .name="Cắn",        .atk_bonus=8, .reach=5, .range_max=0,
      .damage=DICE(2,10,6), .dmg_type=DMG_PIERCING },
    { .name="Phun lửa", .atk_bonus=8, .reach=0, .range_max=0,
      .damage=DICE(8,6,0), .dmg_type=DMG_FIRE },
};

/* --- Zombie: Slam (slow, tanky undead) --- */
static const MonsterAction zombie_actions[] = {
    { .name="Slam", .atk_bonus=3, .reach=5, .range_max=0,
      .damage=DICE(1,6,1), .dmg_type=DMG_BLUDGEONING },
};

/* =====================================================================
   LMoP MONSTERS (11 moi) - stats tu docs/lmop/monsters-items.md
   Bo qua trait dac biet (Split/Nimble Escape/Martial Advantage).
   ===================================================================== */

/* --- Bugbear: Morningstar + Javelin (Cragmaw Hideout/Castle) --- */
static const MonsterAction bugbear_actions[] = {
    { .name="Morningstar", .atk_bonus=4, .reach=5,  .range_max=0,
      .damage=DICE(1,8,2), .dmg_type=DMG_PIERCING },
    { .name="Javelin",     .atk_bonus=4, .reach=30, .range_max=120,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Redbrand Ruffian: 2x Shortsword (Phandalin/Hideout) --- */
static const MonsterAction redbrand_actions[] = {
    { .name="Shortsword", .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Hobgoblin: Longsword + Longbow (wilderness) --- */
static const MonsterAction hobgoblin_actions[] = {
    { .name="Longsword", .atk_bonus=3, .reach=5,   .range_max=0,
      .damage=DICE(1,8,1), .dmg_type=DMG_SLASHING },
    { .name="Longbow",   .atk_bonus=3, .reach=150, .range_max=600,
      .damage=DICE(1,8,1), .dmg_type=DMG_PIERCING },
};

/* --- Owlbear: Multiattack beak + claws (wilderness) --- */
static const MonsterAction owlbear_actions[] = {
    { .name="Beak",  .atk_bonus=7, .reach=5, .range_max=0,
      .damage=DICE(1,10,5), .dmg_type=DMG_PIERCING },
    { .name="Claws", .atk_bonus=7, .reach=5, .range_max=0,
      .damage=DICE(2,8,5),  .dmg_type=DMG_SLASHING },
};

/* --- Grick: Tentacles + Beak (Wave Echo Cave) --- */
static const MonsterAction grick_actions[] = {
    { .name="Tentacles", .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(2,6,2), .dmg_type=DMG_SLASHING },
    { .name="Beak",      .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(1,6,2), .dmg_type=DMG_PIERCING },
};

/* --- Spectator: Eye rays (ranged, Wave Echo Cave guardian) --- */
static const MonsterAction spectator_actions[] = {
    { .name="Eye Ray", .atk_bonus=5, .reach=0, .range_max=120,
      .damage=DICE(3,8,0), .dmg_type=DMG_FORCE },
};

/* --- Ochre Jelly: Pseudopod + acid (Wave Echo Cave) --- */
static const MonsterAction ochre_jelly_actions[] = {
    { .name="Pseudopod", .atk_bonus=4, .reach=5, .range_max=0,
      .damage=DICE(2,6,2), .dmg_type=DMG_BLUDGEONING },
};

/* --- Mormesk Wraith: Life Drain necrotic (Wave Echo Cave) --- */
static const MonsterAction wraith_actions[] = {
    { .name="Life Drain", .atk_bonus=5, .reach=5, .range_max=0,
      .damage=DICE(3,8,3), .dmg_type=DMG_NECROTIC },
};

/* --- Nezznar (Black Spider): Spider Staff + spells (FINAL BOSS) --- */
static const MonsterAction nezznar_actions[] = {
    { .name="Spider Staff", .atk_bonus=1, .reach=5, .range_max=0,
      .damage=DICE(1,6,0), .dmg_type=DMG_POISON },
    { .name="Magic Missile", .atk_bonus=5, .reach=120, .range_max=0,
      .damage=DICE(3,4,3), .dmg_type=DMG_FORCE },
};

/* --- Sildar Hallwinter: Longsword (NPC combat-capable) --- */
static const MonsterAction sildar_actions[] = {
    { .name="Longsword", .atk_bonus=3, .reach=5,   .range_max=0,
      .damage=DICE(1,8,1), .dmg_type=DMG_SLASHING },
    { .name="Heavy Crossbow", .atk_bonus=2, .reach=100, .range_max=400,
      .damage=DICE(1,10,0), .dmg_type=DMG_PIERCING },
};

/* --- Venomfang: Bite + Poison Breath (Thundertree optional boss) --- */
static const MonsterAction venomfang_actions[] = {
    { .name="Bite",         .atk_bonus=7, .reach=10, .range_max=0,
      .damage=DICE(2,10,4),  .dmg_type=DMG_PIERCING },
    { .name="Poison Breath", .atk_bonus=0, .reach=0,  .range_max=60,
      .damage=DICE(12,6,0),  .dmg_type=DMG_POISON },
};

/* =====================================================================
   MONSTER TABLE - moi entry = 1 monster type.
   Design flow: copy 1 entry, doi ten/stats. Them ID o monsters.h.
   ===================================================================== */
const MonsterType MONSTERS[N_MONSTERS] = {

    /* [ID_GOBLIN] - CR 1/4, melee chaser -------------------- */
    [ID_GOBLIN] = {
        .name       = "Goblin",
        .name_vi    = "Yêu tinh",
        .size       = SIZE_SMALL,
        .type       = TYPE_HUMANOID,
        .ac         = 15,
        .hp_dice    = DICE(2, 6, 0),                  /* 2d6 (avg 7) */
        .speed      = 6,                               /* 30 ft/turn */
        .scores     = STATS(STR(8),  DEX(14), CON(10),
                            INT(10), WIS(8),  CHA(8)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'g',
        .glyph_color= CE_RED,
        .actions    = goblin_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_PLAYER] - Hero template --------------------------- */
    [ID_PLAYER] = {
        .name       = "Anh hùng",
        .name_vi    = "Anh hùng",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 16,
        .hp_dice    = DICE(1, 10, 3),                  /* 1d10+3 (fighter) */
        .speed      = 6,
        .scores     = STATS(STR(16), DEX(12), CON(14),
                            INT(10), WIS(10), CHA(12)),
        .cr         = CR(0),
        .xp         = 0,
        .glyph      = '@',
        .glyph_color= CE_GREEN,
        .actions    = player_actions,
        .n_actions  = 2,
        .ai         = NULL,
    },

    /* [ID_SKELETON] - CR 1/4, ranged archer ----------------- */
    [ID_SKELETON] = {
        .name       = "Skeleton",
        .name_vi    = "Bộ xương",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_UNDEAD,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 2),                   /* 2d8+2 (avg 11) */
        .speed      = 6,
        .scores     = STATS(STR(10), DEX(14), CON(15),
                            INT(6),  WIS(8),  CHA(5)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 's',
        .glyph_color= CE_GREY,
        .actions    = skeleton_actions,
        .n_actions  = 2,
        .ai         = ai_ranged,
    },

    /* [ID_WOLF] - CR 1/4, fast melee ------------------------ */
    [ID_WOLF] = {
        .name       = "Wolf",
        .name_vi    = "Sói",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_BEAST,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 2),                   /* 2d8+2 (avg 11) */
        .speed      = 8,                               /* 40 ft - nhanh! */
        .scores     = STATS(STR(12), DEX(15), CON(12),
                            INT(3),  WIS(12), CHA(6)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'w',
        .glyph_color= CE_DGREY,
        .actions    = wolf_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },

    /* [ID_ORC] - CR 1/2, brute melee ------------------------ */
    [ID_ORC] = {
        .name       = "Orc",
        .name_vi    = "Oóc",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 13,
        .hp_dice    = DICE(2, 8, 6),                   /* 2d8+6 (avg 15) */
        .speed      = 6,
        .scores     = STATS(STR(16), DEX(12), CON(16),
                            INT(7),  WIS(11), CHA(10)),
        .cr         = CR(0.5),
        .xp         = 100,
        .glyph      = 'o',
        .glyph_color= CE_RED,
        .actions    = orc_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_OGRE] - CR 2, tanky heavy hitter ------------------ */
    [ID_OGRE] = {
        .name       = "Ogre",
        .name_vi    = "Yêu og",
        .size       = SIZE_LARGE,
        .type       = TYPE_GIANT,
        .ac         = 11,
        .hp_dice    = DICE(7, 10, 7),                  /* 7d10+7 (avg 45) */
        .speed      = 6,
        .scores     = STATS(STR(19), DEX(8),  CON(16),
                            INT(5),  WIS(7),  CHA(7)),
        .cr         = CR(2),
        .xp         = 450,
        .glyph      = 'O',
        .glyph_color= CE_MAG,
        .actions    = ogre_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_DRAGON] - CR 5, boss (multiattack) ---------------- */
    [ID_DRAGON] = {
        .name       = "Rồng",
        .name_vi    = "Rồng đỏ",
        .size       = SIZE_HUGE,
        .type       = TYPE_DRAGON,
        .ac         = 17,
        .hp_dice    = DICE(12, 10, 36),                /* 12d10+36 (avg 102) */
        .speed      = 6,
        .scores     = STATS(STR(23), DEX(12), CON(19),
                            INT(14), WIS(13), CHA(15)),
        .cr         = CR(5),
        .xp         = 1800,
        .glyph      = 'D',
        .glyph_color= CE_RED,
        .actions    = dragon_actions,
        .n_actions  = 2,
        .ai         = ai_boss,
    },

    /* [ID_ZOMBIE] - CR 1/4, slow undead tank ---------------- */
    [ID_ZOMBIE] = {
        .name       = "Zombie",
        .name_vi    = "Xác sống",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_UNDEAD,
        .ac         = 8,
        .hp_dice    = DICE(3, 8, 9),                   /* 3d8+9 (avg 22) */
        .speed      = 4,                               /* 20 ft - cham */
        .scores     = STATS(STR(13), DEX(6),  CON(16),
                            INT(3),  WIS(6),  CHA(5)),
        .cr         = CR(0.25),
        .xp         = 50,
        .glyph      = 'z',
        .glyph_color= CE_DGREEN,                       /* xanh la (undead) */
        .actions    = zombie_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },

    /* ====================================================================
       LMoP MONSTERS (11 moi) - stats tu docs/lmop/monsters-items.md
       ==================================================================== */

    /* [ID_BUGBEAR] - CR 1, Cragmaw Hideout (Klarg) / Castle (King Grol) */
    [ID_BUGBEAR] = {
        .name       = "Bugbear",
        .name_vi    = "Tinh đỏ",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 16,
        .hp_dice    = DICE(5, 8, 5),                   /* 5d8+5 (avg 27) */
        .speed      = 6,
        .scores     = STATS(STR(15), DEX(14), CON(13),
                            INT(8),  WIS(11), CHA(9)),
        .cr         = CR(1),
        .xp         = 200,
        .glyph      = 'B',
        .glyph_color= CE_DYEL,
        .actions    = bugbear_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_REDBRAND] - CR 1/2, Phandalin town / Redbrand Hideout */
    [ID_REDBRAND] = {
        .name       = "Redbrand Ruffian",
        .name_vi    = "Côn đồ Redbrand",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 14,
        .hp_dice    = DICE(3, 8, 3),                   /* 3d8+3 (avg 16) */
        .speed      = 6,
        .scores     = STATS(STR(11), DEX(14), CON(12),
                            INT(9),  WIS(9),  CHA(11)),
        .cr         = CR(0.5),
        .xp         = 100,
        .glyph      = 'R',
        .glyph_color= CE_RED,
        .actions    = redbrand_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },

    /* [ID_HOBGOBLIN] - CR 1/2, wilderness encounter (bounty hunters) */
    [ID_HOBGOBLIN] = {
        .name       = "Hobgoblin",
        .name_vi    = "Yêu tinh hob",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 18,
        .hp_dice    = DICE(2, 8, 2),                   /* 2d8+2 (avg 11) */
        .speed      = 6,
        .scores     = STATS(STR(13), DEX(12), CON(12),
                            INT(10), WIS(10), CHA(9)),
        .cr         = CR(0.5),
        .xp         = 100,
        .glyph      = 'H',
        .glyph_color= CE_DGREEN,
        .actions    = hobgoblin_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_OWBEAR] - CR 3, wilderness apex predator */
    [ID_OWBEAR] = {
        .name       = "Owlbear",
        .name_vi    = "Gấu cú",
        .size       = SIZE_LARGE,
        .type       = TYPE_BEAST,
        .ac         = 13,
        .hp_dice    = DICE(7, 10, 21),                 /* 7d10+21 (avg 59) */
        .speed      = 8,
        .scores     = STATS(STR(20), DEX(12), CON(17),
                            INT(3),  WIS(12), CHA(7)),
        .cr         = CR(3),
        .xp         = 700,
        .glyph      = 'Y',
        .glyph_color= CE_DYEL,
        .actions    = owlbear_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_GRICK] - CR 2, Wave Echo Cave tentacle worm */
    [ID_GRICK] = {
        .name       = "Grick",
        .name_vi    = "Giun đá",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_BEAST,
        .ac         = 14,
        .hp_dice    = DICE(6, 8, 0),                   /* 6d8 (avg 27) */
        .speed      = 6,
        .scores     = STATS(STR(14), DEX(14), CON(11),
                            INT(3),  WIS(14), CHA(5)),
        .cr         = CR(2),
        .xp         = 450,
        .glyph      = 'G',
        .glyph_color= CE_DGREY,
        .actions    = grick_actions,
        .n_actions  = 2,
        .ai         = ai_melee_chaser,
    },

    /* [ID_SPECTATOR] - CR 3, Wave Echo Cave guardian (beholder-kin) */
    [ID_SPECTATOR] = {
        .name       = "Spectator",
        .name_vi    = "Kẻ ngắm",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_BEAST,
        .ac         = 13,
        .hp_dice    = DICE(7, 8, 0),                   /* ~30 HP */
        .speed      = 0,                               /* fly only */
        .scores     = STATS(STR(14), DEX(14), CON(15),
                            INT(13), WIS(14), CHA(15)),
        .cr         = CR(3),
        .xp         = 700,
        .glyph      = 'e',
        .glyph_color= CE_YEL,
        .actions    = spectator_actions,
        .n_actions  = 1,
        .ai         = ai_ranged,
    },

    /* [ID_OCHRE_JELLY] - CR 2, Wave Echo Cave (Split trait bo qua) */
    [ID_OCHRE_JELLY] = {
        .name       = "Ochre Jelly",
        .name_vi    = "Thạch vàng",
        .size       = SIZE_LARGE,
        .type       = TYPE_BEAST,
        .ac         = 8,
        .hp_dice    = DICE(6, 8, 15),                  /* ~45 HP */
        .speed      = 4,
        .scores     = STATS(STR(15), DEX(6),  CON(20),
                            INT(1),  WIS(6),  CHA(1)),
        .cr         = CR(2),
        .xp         = 450,
        .glyph      = 'j',
        .glyph_color= CE_YEL,
        .actions    = ochre_jelly_actions,
        .n_actions  = 1,
        .ai         = ai_melee_chaser,
    },

    /* [ID_WRAITH] - CR 4, Mormesk (Wave Echo Cave undead) */
    [ID_WRAITH] = {
        .name       = "Mormesk Wraith",
        .name_vi    = "Oan hồn Mormesk",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_UNDEAD,
        .ac         = 13,
        .hp_dice    = DICE(6, 8, 18),                  /* 6d8+18 (avg 45) */
        .speed      = 6,                               /* fly 60 ft */
        .scores     = STATS(STR(6),  DEX(16), CON(16),
                            INT(12), WIS(14), CHA(15)),
        .cr         = CR(4),
        .xp         = 1100,
        .glyph      = 'W',
        .glyph_color= CE_CYAN,
        .actions    = wraith_actions,
        .n_actions  = 1,
        .ai         = ai_ranged,
    },

    /* [ID_NEZZNAR] - CR 2, FINAL BOSS (Wave Echo Cave) */
    [ID_NEZZNAR] = {
        .name       = "Nezznar",
        .name_vi    = "Nhện Đen Nezznar",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 11,                              /* 14 with mage armor */
        .hp_dice    = DICE(6, 8, 0),                   /* 6d8 (avg 27) */
        .speed      = 6,
        .scores     = STATS(STR(9),  DEX(13), CON(10),
                            INT(16), WIS(14), CHA(13)),
        .cr         = CR(2),
        .xp         = 450,
        .glyph      = 'N',
        .glyph_color= CE_DMAG,
        .actions    = nezznar_actions,
        .n_actions  = 2,
        .ai         = ai_boss,
    },

    /* [ID_SILDAR] - CR 1, NPC rescue (combat-capable) */
    [ID_SILDAR] = {
        .name       = "Sildar Hallwinter",
        .name_vi    = "Sildar Hallwinter",
        .size       = SIZE_MEDIUM,
        .type       = TYPE_HUMANOID,
        .ac         = 16,
        .hp_dice    = DICE(5, 8, 5),                   /* 5d8+5 (avg 27) */
        .speed      = 6,
        .scores     = STATS(STR(13), DEX(10), CON(12),
                            INT(10), WIS(11), CHA(10)),
        .cr         = CR(1),
        .xp         = 200,
        .glyph      = 'S',
        .glyph_color= CE_GREEN,
        .actions    = sildar_actions,
        .n_actions  = 2,
        .ai         = NULL,                            /* NPC, khong attack */
    },

    /* [ID_VENOMFANG] - CR 4, Thundertree optional boss (Young Green Dragon) */
    [ID_VENOMFANG] = {
        .name       = "Venomfang",
        .name_vi    = "Nanh Độc",
        .size       = SIZE_LARGE,
        .type       = TYPE_DRAGON,
        .ac         = 17,
        .hp_dice    = DICE(16, 10, 48),                /* 16d10+48 (avg 136) */
        .speed      = 8,                               /* fly 80 ft */
        .scores     = STATS(STR(19), DEX(12), CON(17),
                            INT(11), WIS(12), CHA(15)),
        .cr         = CR(4),
        .xp         = 1100,
        .glyph      = 'V',
        .glyph_color= CE_GREEN,
        .actions    = venomfang_actions,
        .n_actions  = 2,
        .ai         = ai_boss,
    },
};
