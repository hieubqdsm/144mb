# LMoP — Room-by-Room Details (Part 1 & 2)

> Data chi tiết từ AnyFlip preview (pages 1-22). Đây là 2 dungeon đầu,
> đủ để đánh giá tile count + encounter density cho 144mb.

---

## Part 1: Goblin Arrows

### Encounter: Goblin Ambush (Triboar Trail)
- **Setup**: Party tìm 2 dead horses block road
- **Enemies**: 4 goblins hidden (2 melee rush + 2 ranged 30ft)
- **Outcome**: Rescue线索 → follow trail tới Cragmaw Hideout

### Dungeon: Cragmaw Hideout (8 areas)
Cave system, goblin base. Stream chảy ra entrance.

| Area | Tên | Nội dung | Monsters/Traps |
|---|---|---|---|
| 1 | Cave Mouth | Entrance, stream, briar thickets | — |
| 2 | Goblin Blind | Hollowed briar, lookout post | 2 goblin guards |
| 3 | Kennel | Dank chamber, stench | 3 chained wolves |
| 4 | Steep Passage | Steep climb, fragile ledge | Ledge trap (fall) |
| 5 | Overpass | Rickety bridge, high above | 1 goblin (triggers flood) |
| 6 | Goblin Den | Smoky common room, upper ledge | Yeemik + goblins, Sildar hostage |
| 7 | Twin Pools Cave | 2 pools = flood dams | Flood trap |
| 8 | Klarg's Cave | Looted provisions, fire pit, chimney | Klarg (bugbear) + wolf |

**Treasure**: Klarg's chest — provisions + potions of healing
**Key NPC**: Yeemik (usurper), Sildar Hallwinter (rescue)

---

## Part 2: Phandalin

### Town: Phandalin (~12 buildings)
| Building | NPC | Function |
|---|---|---|
| Barthen's Provisions | Elmar Barthen | Shop (general goods) |
| Lionshield Coster | Linene Graywind | Shop (weapons/armor) |
| Stonehill Inn | Toblen Stonehill | Inn (rumors, rest) |
| Edermath Orchard | Daran Edermath | Quest giver (Old Owl Well) |
| Miner's Exchange | Halia Thornton | Quest giver (Zhentarim) |
| Alderleaf Farm | Qellene + Carp | Quest (secret tunnel info) |
| Shrine of Luck | Sister Garaele | Quest giver (Agatha, Harper) |
| Townmaster's Hall | Harbin Wester | Quest giver (Wyvern Tor orcs) |
| Tresendar Manor | (abandoned) | → Redbrand Hideout entrance |

### Dungeon: Redbrand Hideout (8 areas, dưới Tresendar Manor)

| Area | Tên | Nội dung | Monsters/Traps |
|---|---|---|---|
| 1 | Cellar | Storage, kegs, cistern | Cistern: waterproof satchel (potion of invisibility) |
| 2 | Barracks | Double bunks, barrels | 3 ruffians |
| 3 | Trapped Hall | Dusty hallway | Pit trap (loose stone tiles) |
| 4 | Tresendar Crypts | 3 stone sarcophagi | Skeletons (armed) |
| 5 | Slave Pens | Iron bar cells | 2 ruffians, Mirna Dendrar + children |
| 6 | Armory | Locked room | Weapons + red cloaks |
| 7 | Storeroom | Work area | *(data truncate)* |
| 8 | Glasstaff's Quarters | Iarno Albrek | Iarno (wizard) + Spider Staff |

**Treasure**: Potion of Invisibility, Spider Staff, weapons
**Key NPC**: Iarno "Glasstaff" Albrek (Redbrand leader), Mirna Dendrar (rescue)

---

## Tile Analysis (cho 144mb)

Đếm tile unique cần cho 2 dungeon này:

### Cragmaw Hideout tiles (cave biome)
- Cave floor (dark) ×1
- Cave wall (rock) ×1
- Stream/water ×1
- Briar thicket (foliage) ×1
- Bridge (wood) ×1
- Stalagmite (obstacle) ×1
- Fire pit ×1
- Chest (container) ×1
**= 8 tiles**

### Redbrand Hideout tiles (manor/crypt biome)
- Stone floor ×1
- Stone wall ×1
- Wooden door ×1
- Sarcophagus (crypt) ×1
- Iron bars (cell) ×1
- Pit trap (hidden) ×1
- Keg/barrel ×1
- Cistern (water) ×1
**= 8 tiles**

### Phandalin town tiles (town biome)
- Cobblestone road ×1
- Grass ×1
- Wood floor (interior) ×1
- Building wall (wood) ×1
- Building wall (stone) ×1
- Door ×1
- Well/fountain ×1
- Signpost ×1
**= 8 tiles**

### Total tiles cho Part 1+2: **~24 tiles**

→ Nếu ASCII 1×1 cell = 240 bytes raw, ~150 bytes gzip
→ Nếu 3×3 cell = 2,160 bytes raw, ~1,300 bytes gzip
→ **Cực nhẹ**, vấn đề KHÔNG nằm ở tile count

### Vấn đề thực sự: MAP LAYOUT
- Cragmaw Hideout: 8 areas ≈ 50-60 cells (procedural BSP xử lý OK)
- Redbrand Hideout: 8 areas ≈ 60-70 cells
- Wave Echo Cave (final): ~16 areas ≈ 120-150 cells

→ Dungeon layout có thể dùng procedural engine hiện có **(BSP)**,
NHƯNG layout LMoP có chủ đích (room sequence, traps, secret doors)
→ Cần **tilemap loader** (task #6 trong AGENTS.md) để load map vẽ tay
thay vì pure procedural.
