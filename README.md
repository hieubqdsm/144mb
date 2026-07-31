# 144mb — ASCII Tabletop RPG Engine (1.44MB Jam)

Game engine + demo cho thử thách "nhét game vào 1 đĩa mềm 1.44MB".
Hướng: **ASCII roguelike RPG** (D&D-lite) bằng C + Win32 Console API.

## Tech stack
- **C thuần** + MSVC 14.51 (Visual Studio 2026), build static `/MT`.
- **Win32 Console API** (`WriteConsoleOutput`) — render ASCII + 16 màu + CP437 glyphs.
- **0 dependency ngoài** (không Raylib/SDL/GPU) → chạy mọi Windows, không bug driver.

## Cấu trúc
```
src_console/
├─ engine/
│  ├─ console.h        # API declarations (BLT-style minimal)
│  └─ console.c        # Win32 Console implementation (1 bản duy nhất)
├─ main.c              # Demo shooter (Floppy Defender)
└─ dd_demo.c           # Demo D&D (character creation + dungeon)

pewball_demo/          # Reference: PEWBALL (FloppyJam 2018 winner) build để học
build_console.bat      # Build shooter
build_dd.bat           # Build D&D demo
```

## Build
```bash
build_console.bat    # → build/game.exe  (~168KB)
build_dd.bat         # → build/dd.exe     (~150KB)
```
Cả 2 đều fit dư dật dưới 1.44MB (dùng ~11% giới hạn).

## Engine API (console.h)
- **Render**: `ce_put/ce_fill/ce_line/ce_circle/ce_sphere/ce_sprite/ce_sprite_multi/ce_text/ce_text_w/ce_border`
- **Input**: `ce_keyDown/ce_keyPressed/ce_mouseClicked/ce_clickedBox/ce_hoverBox`
- **Loop**: `ce_run(update_fn)` — callback-based, hybrid sleep + timeBeginPeriod(1)

## Tiến độ
- [x] Phase 1: Raylib pipeline (bỏ — bug hiển thị GPU)
- [x] Phase 2: Console engine (hoạt động 100%, render verified)
- [x] Phase A: Refactor engine tách module (console.h + console.c)
- [ ] Phase B: engine layer (rng, map, fov, path, bsp) + game systems (actor, d20, combat)
- [ ] Phase C: content (monsters/items/spells data tables, inventory, dialogue)
- [ ] Phase D: REXpaint sprite loader + polish

## Kiến trúc tham khảo
- **NetHack**: data-driven compiled tables + type/instance split
- **Libtcod**: module structure + engine/game separation
- **BearLibTerminal**: minimal console API
