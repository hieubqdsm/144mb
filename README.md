# 144mb — ASCII Dungeon Crawler RPG (1.44MB Jam)

ASCII roguelike RPG (D&D-lite) bằng C + Win32 Console API. Fit ~11% giới hạn 1.44MB.

## Build & run
```bash
build_rpg.bat    # → build/rpg.exe (~167KB)
```
Double-click `build\rpg.exe` để chơi.

## Controls
- **WASD/arrows** — di chuyển / tấn công (bump vào enemy)
- **1** — Fire Bolt (cantrip, 1d10 fire)
- **2** — Magic Missile (1st-level, 3d4+3 auto-hit)
- **H** — dùng Healing Potion
- **I** — toggle inventory panel
- **>** (trên cầu thang) — xuống tầng
- **R** — restart (khi chết)
- **ESC** — menu / quit

## Cấu trúc
```
src_console/
├─ engine/                 # ENGINE (game-agnostic)
│  ├─ console.{h,c}        # Win32 Console API renderer (BLT-style)
│  ├─ rng.{h,c}            # Xorshift64 deterministic RNG
│  ├─ map.{h,c}            # Tile grid + walkable/transparent/seen/visible
│  ├─ fov.{h,c}            # Recursive shadowcasting (8 octants)
│  ├─ path.{h,c}           # A* pathfinding (binary heap)
│  └─ bsp.{h,c}            # Dungeon generation (rooms + corridors)
├─ game/                   # GAME (D&D-lite)
│  ├─ actor.{h,c}          # Type/Instance split (NetHack permonst/monst)
│  ├─ d20.{h,c}            # d20 rolls + advantage/disadvantage + crit
│  ├─ combat.{h,c}         # Attack resolution (to-hit vs AC, damage)
│  ├─ turn.{h,c}           # Initiative + round-robin turn order
│  ├─ conditions.{h,c}     # Timed effects (DOT, buffs, save-ends)
│  ├─ items.{h,c}          # Item type definitions
│  ├─ inventory.{h,c}      # Backpack + equip slots + use items
│  ├─ ai.{h,c}             # Monster AI (melee chaser)
│  ├─ spells.h + spell_resolve.c   # Spell definitions + cast resolve
│  ├─ dungeon.{h,c}        # Multi-floor procedural dungeon
│  ├─ ui.{h,c}             # Sidebar, HP bar, combat log, inventory panel
│  └─ rpg_main.c           # Main game loop (menu→class→dungeon→combat)
├─ data/                   # NetHack-style compiled const tables
│  ├─ monsters.{h,c}       # Goblin, Hero (stats, actions, dice)
│  ├─ items.c              # Longsword, Chain Shirt, Potion, Dagger
│  └─ spells.c             # Fire Bolt, Magic Missile, Fireball, Mage Armor, Cure Wounds
├─ structs.h               # Shared structs (DiceFormula, MonsterType, Actor, Battle)
└─ enums.h                 # Ability, DamageType, Condition, Team, SaveType, RollMode

pewball_demo/              # Reference: PEWBALL (FloppyJam 2018)
build_rpg.bat              # Build script
```

## Kiến trúc
- **Engine/Game separation** (libtcod): engine không chứa game logic.
- **Type/Instance split** (NetHack): `MonsterType` (const shared) → `Actor` (instance). 50 goblin = 1 def + 50 tiny instances.
- **Data-driven compiled tables** (NetHack): monster/item/spell = `const` arrays, 0 parser, 0 runtime cost.
- **Command pattern combat** (Natural_20): attack resolve = dice → AC check → damage/crit.
- **Minimal console API** (BearLibTerminal): ~20 render functions trên `WriteConsoleOutput`.

## Tính năng D&D-lite đã có
- ✅ d20 resolution: attack rolls, advantage/disadvantage, crit (nat 20), fumble (nat 1)
- ✅ 6 ability scores (STR/DEX/CON/INT/WIS/CHA) + modifiers
- ✅ AC, HP, damage types (slashing/piercing/fire/force...)
- ✅ Turn-based combat with initiative
- ✅ 5 spells (4 kinds: atk-ranged, magic-missile, save-half, buff-AC, heal)
- ✅ Inventory + equip (weapon/armor) + potions
- ✅ Timed effects (DOT poison, AC buffs, save-ends)
- ✅ FOV fog of war (recursive shadowcasting)
- ✅ Multi-floor procedural dungeon (BSP rooms + corridors + stairs)
- ✅ Monster AI (melee chaser)
- ✅ Combat log + HP bars + sidebar

## Tiến độ
- [x] Phase A: Engine refactor tách module
- [x] Phase B: engine layer (rng/map/fov/path/bsp)
- [x] Phase C: game systems (actor/d20/combat/turn/items/inventory/spells/dungeon/ui)
- [ ] Phase D: REXpaint sprite loader + polish + sprite art
