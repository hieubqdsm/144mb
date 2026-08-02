# Kiến trúc: RPG Maker ASCII Full D&D 5e

> **Mục tiêu**: Framework RPG Maker style hoàn toàn ASCII, hỗ trợ full D&D 5e.
> **Status**: Chưa làm gì, đây là định hướng.

---

## 1. So sánh với RPG Maker gốc

| RPG Maker | Bản ASCII tương đương |
|---|---|
| Tileset (32×32 PNG) | ASCII tile (`#`, `.`, `+`) — đã có `map.h` |
| Character sprite (4 hướng) | 4 glyph variant (`@` `▲` `▼` `◀` `▶`) hoặc 1 glyph |
| Event (parallel/auto/action) | **Event system** — chưa có |
| Database (Actors/Classes) | **D&D character system** — chưa có |
| Scene manager (Map/Battle/Menu) | State machine — đã có `ST_TITLE/PLAY/PAUSE` |
| Move route | **Pathfinding move** — đã có `path.h` (A*) |
| Message box | **Dialogue system** — chưa có |
| Common events | **Quest/scripts** — chưa có |

---

## 2. Kiến trúc 4 lớp (layered)

```
┌─────────────────────────────────────────────┐
│ L4: GAME CONTENT (data + events)             │  ← tác giả viết
│   - Map data, NPC dialogue, quest scripts   │
├─────────────────────────────────────────────┤
│ L3: D&D 5e RULESET                           │  ← MỞ RỘNG engine
│   - Race/Class/Background/Feat system       │
│   - Spellbook, prepared spells              │
│   - Multiclassing, leveling table           │
├─────────────────────────────────────────────┤
│ L2: RPG FRAMEWORK (RPG Maker core)          │  ← LÀM MỚI
│   - Scene manager, Event system, Dialogue   │
│   - Character (move/facing), Shop, Inventory│
│   - Tilemap loader, Prop system             │
├─────────────────────────────────────────────┤
│ L1: ENGINE (đã có, giữ nguyên)               │
│   - ce_*, map/fov/path/bsp/xp/rng           │
│   - actor/d20/combat/turn/conditions/spells │
│   - save/load, inventory, ai                │
└─────────────────────────────────────────────┘
```

---

## 3. DANH SÁCH FUNCTIONS CẦN CÓ

### L1 — ENGINE (đã có, KHÔNG đổi)

Giữ nguyên ~80 function hiện có trong `engine/*.h` và `game/*.h` (actor/d20/combat/turn/fov/path/bsp/xp/rng/save/inv/ai/spell/conditions/ui/i18n).

---

### L2 — RPG FRAMEWORK (RPG Maker core) — CHƯA CÓ

#### 3.1 Scene Manager (`scene.h/c`)
Quản lý stack scene (Title → World → Menu → Combat → Dialogue → Shop).

```c
typedef enum {
    SCENE_TITLE, SCENE_WORLD, SCENE_MENU, SCENE_COMBAT,
    SCENE_DIALOGUE, SCENE_SHOP, SCENE_GAMEOVER, SCENE_LOAD
} SceneType;

void scene_init(void);
void scene_push(SceneType type);     // vào scene con (vd Dialogue trên World)
void scene_pop(void);                // quay lại scene cha
void scene_set(SceneType type);      // thay scene gốc (vd Title→World)
SceneType scene_current(void);
void scene_update(float dt);         // dispatch update tới scene active
void scene_render(void);             // dispatch render
```

#### 3.2 Tilemap Loader (`tilemap.h/c`)
Load map vẽ tay từ file `.map` (ASCII text) hoặc `.tmx`-like binary.

```c
typedef struct {
    int width, height;
    uint8_t *tiles;            // tile type mỗi cell
    int *prop_id;              // prop id mỗi cell (-1 = none)
    int *event_id;             // event id mỗi cell (-1 = none)
    char bg_music[32];         // music track
    char display_name[32];     // "Phandalin"
} Tilemap;

Tilemap *tilemap_load(const char *path);
void tilemap_free(Tilemap *t);
TileType tilemap_at(const Tilemap *t, int x, int y);
int tilemap_walkable(const Tilemap *t, int x, int y);     // tile + prop
int tilemap_blocks_sight(const Tilemap *t, int x, int y); // tile + prop (tall)
void tilemap_render(const Tilemap *t, int cam_x, int cam_y);
void tilemap_save(const Tilemap *t, const char *path);    // editor
```

#### 3.3 Prop System (`prop.h/c`)
Object không logic: thùng, rương, torch, giường, bàn — đè lên tile.

```c
typedef struct {
    int id;
    char name[32];
    uint16_t glyph;
    int color;
    int walkable;        // 1 = đi xuyên
    int blocks_sight;    // 1 = che FOV
    int is_container;    // 1 = rương (click để mở)
} PropDef;

extern const PropDef PROPS[];          // data table
extern const int N_PROPS;

int prop_place(Tilemap *t, int x, int y, int prop_id);
int prop_remove(Tilemap *t, int x, int y);
const PropDef *prop_at(const Tilemap *t, int x, int y);
void prop_render(const Tilemap *t, int cam_x, int cam_y);
```

#### 3.4 Character / GameObj (`character.h/c`)
Actor của game (player + NPC), có movement + facing + nameplate.

```c
typedef struct {
    Actor actor;              // reuse engine Actor (hp/ac/stats)
    int char_class_id;        // Fighter/Cleric/...
    int race_id;              // Human/Elf/Dwarf/...
    Inventory inv;
    char name[24];
    int facing;               // 0=up 1=down 2=left 3=right
    int sprite_w, sprite_h;   // 1×1 (tactical) hoặc 3×3 (zoom)
    int move_speed;           // cells/turn
    int event_id;             // event trigger khi interact
} Character;

void character_move(Character *c, int dx, int dy, const Tilemap *t);
void character_face(Character *c, int direction);
void character_render(const Character *c, int cam_x, int cam_y, int show_name);
int character_interact(Character *c, const Tilemap *t);  // action button (Space)
int character_at(int x, int y);   // index trong char list, -1 nếu none
```

#### 3.5 Event System (`event.h/c`)
Trigger khi: proximity (đến gần) / action button (nhấn Space) / auto-run / parallel.

```c
typedef enum {
    TRIGGER_NONE, TRIGGER_ACTION_BUTTON, TRIGGER_PLAYER_TOUCH,
    TRIGGER_EVENT_TOUCH, TRIGGER_AUTO_RUN, TRIGGER_PARALLEL
} TriggerType;

typedef struct {
    int id;
    TriggerType trigger;
    int x, y;                 // vị trí (cho map event)
    int condition_switch;     // switch phải on mới chạy
    int condition_item;       // phải có item
    int condition_level;      // party level tối thiểu
    /* Page: nhiều page = nhiều điều kiện hiển thị */
    int n_pages;
    /* Script: danh sách command */
    int *command_ids;
    int n_commands;
} GameEvent;

/* Command types (RPG Maker style) */
typedef enum {
    CMD_SHOW_TEXT,            // dialogue box
    CMD_SHOW_CHOICES,         // yes/no hoặc multi-choice
    CMD_CHANGE_GOLD,          // +gold
    CMD_CHANGE_ITEMS,         // +item
    CMD_CHANGE_HP,
    CMD_CHANGE_HP_PERCENT,
    CMD_CHANGE_LEVEL,
    CMD_CHANGE_EXP,
    CMD_CHANGE_STATE,         // +condition
    CMD_RECOVER_ALL,          // full heal
    CMD_CHANGE_GOLD_OTHER,
    CMD_MOVE_ROUTE,           // NPC di chuyển theo path
    CMD_WAIT,                 // pause X frames
    CMD_PLAY_SOUND,
    CMD_CHANGE_BGM,
    CMD_BATTLE_PROCESSING,    // vào combat
    CMD_SHOP_PROCESSING,      // vào shop
    CMD_NAME_INPUT,           // nhập tên
    CMD_TRANSFER_PLAYER,      // teleport sang map khác
    CMD_SET_EVENT_LOCATION,
    CMD_SWITCH_OPERATION,     // set switch
    CMD_VARIABLE_OPERATION,   // set variable
    CMD_SELF_SWITCH,          // local switch
    CMD_CONDITIONAL_BRANCH,   // if/else
    CMD_LOOP,
    CMD_BREAK_LOOP,
    CMD_EXIT_EVENT_PROCESSING,
    CMD_ERASE_EVENT,
    CMD_CALL_COMMON_EVENT,
    CMD_LABEL,                // goto label
    CMD_JUMP_TO_LABEL,
    CMD_COMMENT
} CommandType;

void event_run(GameEvent *e);
void event_queue(GameEvent *e);
int event_active(void);          // có event đang chạy?
void event_update(float dt);
CommandType event_current_cmd(void);
```

#### 3.6 Dialogue System (`dialogue.h/c`)
Hiển thị text box, hỗ trợ portrait, choices, typewriter effect.

```c
typedef struct {
    char speaker[24];
    char lines[8][80];
    int n_lines;
    int portrait_id;        // -1 = none
    int voice_sound;        // beep per char
} Dialogue;

void dialogue_show(const Dialogue *d);
void dialogue_update(float dt);   // typewriter
int dialogue_choice(const char *options[], int n);
void dialogue_skip(void);
int dialogue_is_done(void);
int dialogue_render(int x, int y, int w, int h);
```

#### 3.7 Shop System (`shop.h/c`)
Buy/sell/identify.

```c
typedef struct {
    int item_id;
    int price;
    int stock;              // -1 = infinite
} ShopEntry;

typedef struct {
    char name[32];
    ShopEntry items[24];
    int n_items;
    int buy_price_pct;      // 100 = full price
    int sell_price_pct;     // 50 = half
    int gold;
} Shop;

void shop_open(Shop *s);
void shop_buy(Shop *s, int idx);
void shop_sell(Shop *s, int inv_idx);
void shop_close(void);
void shop_render(void);
```

#### 3.8 Quest / Variable System (`quest.h/c`)
Track quest progress, switches, variables.

```c
/* Switches (boolean) */
void switch_set(int id, int val);
int switch_get(int id);
/* Variables (int) */
void var_set(int id, int val);
int var_get(int id);
void var_add(int id, int delta);

typedef struct {
    int id;
    char name[32];
    char description[128];
    int state;              // 0=not started, 1=in progress, 2=done, 3=failed
    int objective_var;      // variable id tracking progress
    int objective_target;
    int reward_gold;
    int reward_item;
    int reward_exp;
} Quest;

void quest_start(int id);
void quest_update(int id);
void quest_complete(int id);
void quest_render_log(int x, int y, int max);
```

#### 3.9 Camera / View (`camera.h/c`)
Scroll mượt trong map lớn hơn screen.

```c
void camera_set(int x, int y);
void camera_follow(Character *c);
void camera_shake(int magnitude, int duration);
void camera_update(float dt);
void camera_get(int *cam_x, int *cam_y);
```

#### 3.10 Audio (`audio.h/c`)
Sử dụng Win32 PlaySound + MIDI.

```c
void bgm_play(const char *path);
void bgm_stop(void);
void bgm_set_volume(int vol);   // 0-100
void sfx_play(const char *path);
void sfx_play_beep(int freq, int dur);   // đã có partial trong rpg_main
```

---

### L3 — D&D 5e RULESET (mở rộng engine) — CHƯA CÓ

#### 3.11 Race / Class / Background (`dnd_char.h/c`)
Character creation đầy đủ D&D 5e.

```c
typedef struct {
    int id;
    char name[24];
    int str_bonus, dex_bonus, con_bonus, int_bonus, wis_bonus, cha_bonus;
    int speed;
    int size;
    int darkvision;
    /* Racial traits (gained at level 1) */
    int trait_ids[8];
} Race;

typedef struct {
    int id;
    char name[24];
    int hit_die;            // d6, d8, d10, d12
    int primary_ability;    // STR/DEX/INT/WIS/CHA
    int saving_throws[2];   // 2 ability
    int skill_choices;      // số skill chọn
    int armor_prof;
    int weapon_prof;
    int spellcaster;        // 0/1
    int spellcasting_ability;
    /* Level table (HP, prof bonus, features) */
    /* Features gained per level */
    int features[20][4];    // feature_id per level
} CharacterClass;

typedef struct {
    int id;
    char name[24];
    int skill_granted[2];
    int gold;
    int item_ids[4];
} Background;

extern const Race RACES[];
extern const CharacterClass CLASSES[];
extern const Background BACKGROUNDS[];

Character *character_create(int race_id, int class_id, int bg_id, RNG *rng);
void character_level_up(Character *c, RNG *rng);
int character_proficiency_bonus(const Character *c);
int character_save_bonus(const Character *c, Ability ab);
int character_skill_bonus(const Character *c, int skill_id);
int character_passive_score(const Character *c, Ability ab);  // passive Perception
```

#### 3.12 Skills (`skills.h/c`)
18 skill D&D 5e (Acrobatics, Arcana, Athletics, ...).

```c
typedef enum {
    SKILL_ACROBATICS=0, SKILL_ANIMAL_HANDLING, SKILL_ARCANA, SKILL_ATHLETICS,
    SKILL_DECEPTION, SKILL_HISTORY, SKILL_INSIGHT, SKILL_INTIMIDATION,
    SKILL_INVESTIGATION, SKILL_MEDICINE, SKILL_NATURE, SKILL_PERCEPTION,
    SKILL_PERFORMANCE, SKILL_PERSUASION, SKILL_RELIGION, SKILL_SLEIGHT_OF_HAND,
    SKILL_STEALTH, SKILL_SURVIVAL, SKILL_COUNT
} SkillId;

extern const Ability SKILL_ABILITY[SKILL_COUNT];
extern const char *SKILL_NAMES[SKILL_COUNT];

int skill_check(Character *c, SkillId skill, int dc, RollMode mode, RNG *rng);
int skill_contested(Character *a, Character *b, SkillId sa, SkillId sb, RNG *rng);
```

#### 3.13 Feats (`feats.h/c`)
Optional D&D feats (Great Weapon Master, Sharpshooter, ...).

```c
typedef struct {
    int id;
    char name[40];
    char description[128];
    /* Mechanical effects */
    int str_bonus, dex_bonus, con_bonus, int_bonus, wis_bonus, cha_bonus;
    int ac_bonus;
    int speed_bonus;
    /* Hooks (function pointers for special behavior) */
    int (*on_attack_hit)(Character *self, Character *target);
    int (*on_damaged)(Character *self, Character *source, int dmg);
} Feat;

void feat_add(Character *c, int feat_id);
int feat_has(const Character *c, int feat_id);
```

#### 3.14 Spellbook (`spellbook.h/c`)
Mở rộng `spells.h` hiện có: spell slots, prepared spells.

```c
typedef struct {
    int prepared[40];       // spell ids prepared
    int n_prepared;
    int slot_used[9];       // slots per level 1-9
    int slot_max[9];        // max slots per level
} Spellbook;

int spell_prepare(Spellbook *sb, int spell_id);
int spell_unprepare(Spellbook *sb, int spell_id);
int spell_cast_slot(Spellbook *sb, int level, RNG *rng);   // dùng slot
int spell_slots_left(const Spellbook *sb, int level);
void spell_slots_restore(Spellbook *sb);                   // long rest
```

#### 3.15 Combat Extended (`combat_ext.h/c`)
Phần D&D 5e chưa có: grapple, shove, dodge/disengage/dash action.

```c
typedef enum {
    ACTION_ATTACK, ACTION_DASH, ACTION_DISENGAGE, ACTION_DODGE,
    ACTION_HELP, ACTION_HIDE, ACTION_READY, ACTION_SEARCH,
    ACTION_USE_OBJECT, ACTION_CAST_SPELL, ACTION_BONUS_ACTION
} ActionType;

typedef enum {
    ATTACK_MELEE, ATTACK_RANGED, ATTACK_GRAPPLE, ATTACK_SHOVE
} AttackType;

int combat_grapple(Character *attacker, Character *target, RNG *rng);
int combat_shove(Character *attacker, Character *target, RNG *rng);   // push/knock prone
int combat_attack_opportunity(Character *attacker, Character *target, RNG *rng);
int combat_ranged_disadvantage(const Character *attacker, const Character *target);  // enemy trong 5ft
int combat_cover_bonus(const Character *target, const Tilemap *t);    // half/three-quarter
```

#### 3.16 Equipment Extended (`equip_ext.h/c`)
Magic items, attunement, properties.

```c
typedef struct {
    int rarity;             // common/uncommon/rare/legendary/artifact
    int requires_attunement;
    int attuned_by;         // char index, -1 = none
    int caster_level;       // for scroll/wand
    int charges;
    int charge_recharge;    // 1d6 / dawn / etc
} MagicItemExt;

int item_attune(Character *c, int inv_idx);
int item_use_charge(Inventory *inv, int inv_idx, int amount);
```

#### 3.17 Conditions Full D&D (`conditions_ext.h/c`)
Mở rộng bitmask hiện có: thêm prone, restrained, exhaustion levels.

```c
typedef enum {
    /* Đã có */
    /* + thêm: */
    COND exhaustion_level;  /* 0-6 levels */
} ExtCondition;

void exhaustion_add_level(Character *c);
int exhaustion_level(const Character *c);
int exhaustion_speed_penalty(const Character *c);
int exhaustion_disadvantage_check(const Character *c);
void exhaustion_long_rest_reduce(Character *c);  // giảm 1 level khi long rest
```

---

### L4 — GAME CONTENT — TÁC GIẢ VIẾT

Không phải function, là data:
- `data/maps/*.map` — 10 file map LMoP
- `data/dialogues/*.dlg` — NPC dialogue scripts
- `data/quests/*.qst` — quest definitions
- `data/events/*.evt` — event scripts (scripted sequences)
- `data/races.c`, `data/classes.c`, `data/feats.c` — D&D 5e tables
- `data/shops.c` — vendor inventories
- `data/sprites/*.xp` — sprite art

---

## 4. TỔNG KẾT

### Số lượng function

| Layer | Module | Function mới | Status |
|---|---|---|---|
| **L1 Engine** | (giữ nguyên) | 0 | ✅ Có ~80 func |
| **L2 Framework** | scene | 7 | ❌ |
| | tilemap | 7 | ❌ |
| | prop | 4 | ❌ |
| | character | 5 | ❌ |
| | event | 5 | ❌ |
| | dialogue | 5 | ❌ |
| | shop | 5 | ❌ |
| | quest | 4 | ❌ |
| | camera | 4 | ❌ |
| | audio | 4 | ❌ |
| **L3 D&D 5e** | dnd_char | 5 | ❌ |
| | skills | 2 | ❌ |
| | feats | 2 | ❌ |
| | spellbook | 4 | ❌ |
| | combat_ext | 5 | ❌ |
| | equip_ext | 2 | ❌ |
| | conditions_ext | 4 | ❌ |
| **L4 Content** | (data files) | — | ❌ |

**Tổng: ~74 function mới**, chia 17 module.

### Ước lượng size code

| Module | Dòng code ước tính | KB exe thêm |
|---|---|---|
| L2 Framework (10 module) | ~2500 | ~25 KB |
| L3 D&D 5e (7 module) | ~1800 | ~18 KB |
| **Tổng thêm** | ~4300 dòng | **~43 KB** |

→ Engine hiện 165 KB + 43 KB = **~208 KB**, còn dư **1232 KB** cho content/maps/art.

### Thứ tự ưu tiên build

| Phase | Module | Lý do |
|---|---|---|
| **P1 (core)** | scene + tilemap + character | Cho phép có "thế giới" |
| **P1** | event + dialogue | Cho phép có "cốt truyện" |
| **P2 (D&D core)** | dnd_char + skills + spellbook | Character creation đầy đủ |
| **P2** | combat_ext | Grapple/shove/opportunity |
| **P3 (polish)** | shop + quest + camera + prop | Town hub, side quest |
| **P3** | feats + equip_ext + conditions_ext | Endgame depth |
| **P4 (content)** | maps + dialogues + data | LMoP campaign |

---

## 5. CÂU HỎI THIẾT KẾ CÒN MỞ

Trước khi code, cần chốt:

1. **Sprite size**: 1×1 (tactical) hay 3×3 (zoom, có nameplate)? → quyết định render layer
2. **Map size**: 60×32 (hiện) hay lớn hơn 100×60 (cho town)? → camera scroll
3. **Tile size**: 1 cell = 1 tile (hiện) hay 1 tile = 3×3 cell (detail)? → quyết định `tilemap_at()`
4. **Event script format**: text script (parser) hay C array (compiled)? → trade-off size vs flexibility
5. **Combat view**: full-screen battle scene (RPG Maker) hay inline (LMoP hiện tại)? → scene architecture
6. **Save format**: binary (hiện) hay JSON/text (editor-friendly)? → save/load rewrite
