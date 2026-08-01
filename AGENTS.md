# AGENTS.md - Hướng dẫn cho session mới (PROJECT STATE)

## Tóm tắt project
Game ASCII RPG (D&D-lite) cho thử thách "1.44MB" (đĩa mềm).
Engine + game bằng **C thuần + Win32**, render GDI bitmap. Fit 11.6% giới hạn.

- **Repo**: github.com:hieubqdsm/144mb (branch main)
- **Working dir**: D:\CODE\144mb
- **Build tool**: MSVC (Visual Studio 2026), `/SUBSYSTEM:WINDOWS /MT`
- **Cuối cùng commit**: `0473381` (Việt hóa battle sim)

## Cách build (KHÔNG tự chạy game, chỉ build + đo size)
```bash
build_rpg.bat      # → build/rpg.exe (~171KB, game GDI)
build_sim.bat      # → build/battle_sim.exe (console, hien dice rolls)
build_test.bat     # → build/test_logic.exe (17 tests)
```
Lệnh MSVC trực tiếp (nếu build script lỗi):
```bash
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
```

## Cấu trúc thư mục
```
src_console/
├─ engine/              # ENGINE (game-agnostic, KHONG đụng)
│  ├─ console.h         # API contract (30 functions, KHONG đổi)
│  ├─ gdi_renderer.c    # Render GDI bitmap (thay WriteConsoleOutput)
│  ├─ rng.h/c           # Xorshift64 deterministic RNG
│  ├─ map.h/c           # Tile grid + walkable/transparent/seen/visible + LoS (Bresenham)
│  ├─ fov.h/c           # Recursive shadowcasting (8 octants)
│  ├─ path.h/c          # A* pathfinding
│  ├─ bsp.h/c           # Dungeon generation (rooms + corridors)
│  ├─ inflate.h/c       # DEFLATE inflater (pure-C, cho .xp gzip)
│  ├─ xp_loader.h/c     # REXpaint .xp parser
│  └─ xp_render.h/c     # Ve .xp sprite (quantize RGB→16 màu)
├─ game/                # GAME logic
│  ├─ actor.h/c         # Type/instance split + resting + death saves + action economy
│  ├─ d20.h/c           # d20 rolls + advantage/crit + damage detail
│  ├─ combat.h/c        # Attack resolve (AC + bonus) + concentration check
│  ├─ turn.h/c          # Initiative + round-robin
│  ├─ conditions.h/c    # Timed effects (DOT, buff, save-ends)
│  ├─ items.h           # ItemType definitions
│  ├─ inventory.h/c     # Backpack + equip + use potion
│  ├─ ai.h/c            # 3 AI: ai_melee_chaser, ai_ranged, ai_boss
│  ├─ spells.h          # SpellDef definitions
│  ├─ spell_resolve.h/c # Cast spell + apply effect
│  ├─ dungeon.h/c       # Multi-floor procedural (BSP + spawn)
│  ├─ ui.h/c            # Sidebar + HP bar + log + button
│  ├─ save.h/c          # Save/Load binary (versioned)
│  ├─ rpg_main.c        # MAIN GAME (menu→class→dungeon→combat)
│  ├─ battle_sim.c      # DEMO: console battle hien dice (Vietnamese)
│  └─ test_logic.c      # TEST: 17 tests logic (ALL PASS)
├─ data/                # DATA compiled const tables (EDIT ở đây)
│  ├─ monsters.h/c      # 8 monsters (Goblin/Anh hùng/skeleton/wolf/orc/ogre/rồng/zombie)
│  ├─ items.c           # 4 items (kiếm dài/áo giáp/thuốc/dao găm)
│  └─ spells.c          # 7 spells (fire bolt/magic missile/fireball/mage armor/cure/poison/hold)
├─ structs.h            # Shared structs + DSL macros (DICE/STATS/CR/SIZE/TYPE)
└─ enums.h              # Ability/DamageType/Condition/Team/SaveType/RollMode
```

## Tiến độ cơ chế (ĐÃ XONG - verify 17 tests PASS)
✅ d20 rolls (advantage/crit/fumble), AC check, damage dice
✅ Actor HP/death/heal, action economy (action/bonus/reaction/move)
✅ Combat: attack resolve vs AC (base + armor + condition bonus)
✅ Turn-based + initiative + round-robin
✅ Inventory + equip + potion use
✅ Conditions: poison DOT, stun, save-ends, buff AC
✅ Spells: 7 spells (5 kinds: atk-ranged/magic-missile/save-half/buff-AC/heal/poison/stun)
✅ Save/Load binary
✅ Line of Sight (Bresenham) cho ranged
✅ Resting (short/long), Death saves (3 save/3 fail)
✅ FOV shadowcasting, procedural dungeon (BSP), multi-floor
✅ 8 monster types, 3 AI behaviors
✅ Concentration check
✅ REXpaint .xp loader (sẵn sàng, chưa plug sprite art)
✅ Tiếng Việt có dấu (SetConsoleOutputCP UTF-8) trong battle_sim

## CÒN THIẾU / CHƯA LÀM (ưu tiên theo impact)
1. **Plug save/load vào rpg_main.c** (engine có, game chưa gọi - cần UI menu "Continue")
2. **Plug resting vào rpg_main.c** (actor_short_rest/long_rest có, game chưa bind key)
3. **Plug death saves vào rpg_main.c** (engine có, game chết ngay HP=0)
4. **Plug concentration vào take_damage** (combat_concentration_check có, chưa gọi)
5. **Targeting system** (spell auto-chon gần nhất, chưa cho player chọn bằng chuột)
6. **Tilemap loader** (map vẽ tay - hiện chỉ procedural) → cho campaign có chủ ý
7. **Sprite .xp art** (loader có, chưa có file .xp + chưa plug xp_draw_file vào draw_map)
8. **Dialogue / NPC system** (chưa có)
9. **Campaign / story flow** (chưa có, chỉ endless dungeon)
10. **Multi-region layout** (GDI foundation có 2 HFONT, chưa chia zone map/HUD khác size)
11. **Thêm content** (monster/item/spell mới = chỉ thêm entry trong data/*.c)

## Flow thiết kế content (KHÔNG cần code, chỉ text editor)
- **Thêm quái**: `data/monsters.c` copy 1 entry, đổi `.name/.ac/.hp_dice/.scores/.glyph`, thêm ID trong `monsters.h` + tăng N_MONSTERS
- **Thêm vũ khí/giáp**: `data/items.c` copy entry, thêm ID trong `items.h`
- **Thêm phép**: `data/spells.c` copy entry, thêm ID trong `spells.h`
- **DSL macros** (trong structs.h): `DICE(2,6,2)`, `STATS(STR(8),DEX(14),...)`, `CR(0.25)`, `SIZE_SMALL`, `TYPE_DRAGON`, `CE_RED`
- **Test balance**: `build_sim.bat` → battle_sim.exe (Hero vs Dragon, hien dice)

## Quy ước quan trọng
- **KHÔNG tự mở Explorer hay chạy game** (user tự chạy khi muốn)
- **Chỉ build + đo size**, KHÔNG launch exe trừ khi user yêu cầu
- **Vietnamese có dấu** cho UI/log (UTF-8). Code comments cũng Vietnamese
- **Commit message** Vietnamese, push lên main sau mỗi milestone
- **Test harness** (`test_logic.c`) phải ALL PASS trước commit lớn
- **console.h là API contract** - KHÔNG đổi signature (game phụ thuộc)
- **Engine (engine/*) KHÔNG chứa game logic** - tách bạch như libtcod
- **Size budget**: rpg.exe hiện 171KB / 1440KB (11.6%), còn dư ~1.27MB

## Lệnh git
```bash
git add -A && git reset HEAD lib/ .zcode/   # stage (skip lib/ raylib, .zcode/)
git commit -m "..."                          # commit Vietnamese
git push origin main                         # push
git log --oneline -5                         # xem history
```

## Khi session mới bắt đầu — hỏi user:
"Cần làm gì tiếp?" → user sẽ nói. Các hướng phổ biến:
- "thêm quái/vũ khí/phép" → sửa data/*.c (xem Flow thiết kế content)
- "plug save/load/resting/death saves" → sửa rpg_main.c bind vào game
- "làm tilemap loader" → viết map loader mới (engine)
- "vẽ sprite" → user dùng REXpaint, mình plug xp_draw_file
- "làm dialogue/campaign" → viết system mới
- "fix bug X" → đọc code, sửa, rebuild test
