# LMoP — Monsters & Magic Items (data thật từ Appendix A+B)

> **Nguồn**: Trích từ `DNDPDF/Lost Mine of Phandelver.html` (Appendix B Monsters + Appendix A Magic Items).
> **Mục đích**: Source-of-truth để thêm vào `data/monsters.c` + `data/items.c`.

---

## 1. MONSTERS (Appendix B — stat blocks thật)

### Bugbear (Humanoid, goblinoid, chaotic evil)
- **AC** 16 (hide armor + shield)
- **HP** 27 (5d8+5), Speed 30ft
- **STR** 15(+2) **DEX** 14(+2) **CON** 13(+1) **INT** 8(-1) **WIS** 11(+0) **CHA** 9(-1)
- **Skills** Stealth +6
- **Senses** darkvision 60ft, passive Perception 10
- **CR** 1 (200 XP)
- **Trait: Surprise Attack** — nếu surprise target, +7 (2d6) damage extra
- **Attacks**:
  - Morningstar +4, 1d8+2 piercing
  - Javelin +4, 1d6+2 piercing (range 30/120)

### Goblin (Humanoid, goblinoid, neutral evil)
- **AC** 15 (leather armor + shield)
- **HP** 7 (2d6), Speed 30ft
- **STR** 8(-1) **DEX** 14(+2) **CON** 10(+0) **INT** 10(+0) **WIS** 8(-1) **CHA** 8(-1)
- **Senses** darkvision 60ft, passive Perception 9
- **CR** 1/4 (50 XP)
- **Trait: Nimble Escape** — Disengage hoặc Dash as bonus action mỗi turn
- **Attacks**:
  - Scimitar +4, 1d6+2 slashing
  - Shortbow +4, 1d6+2 piercing (range 80/320)

### Hobgoblin (Humanoid, goblinoid, lawful evil)
- **AC** 18 (chainmail + shield)
- **HP** 11 (2d8+2), Speed 30ft
- **STR** 13(+1) **DEX** 12(+1) **CON** 12(+1) **INT** 10(+0) **WIS** 10(+0) **CHA** 9(-1)
- **CR** 1/2 (100 XP)
- **Trait: Martial Advantage** — +7 (2d6) damage nếu target trong 5ft ally
- **Attacks**:
  - Longsword +3, 1d8+1 slashing
  - Longbow +3, 1d8+1 piercing (range 150/600)

### Orc (Humanoid, chaotic evil)
- **AC** 13 (hide armor)
- **HP** 15 (2d8+6), Speed 30ft
- **STR** 16(+3) **DEX** 12(+1) **CON** 16(+3) **INT** 7(-2) **WIS** 11(+0) **CHA** 10(+0)
- **Skills** Intimidation +2
- **CR** 1/2 (100 XP)
- **Trait: Aggressive** — Dash toward hostile as bonus action
- **Attacks**:
  - Greataxe +5, 1d12+3 slashing
  - Javelin +5, 1d6+3 piercing (range 30/120)

### Ogre (Large giant, chaotic evil)
- **AC** 11 (hide armor)
- **HP** 59 (7d10+21), Speed 40ft
- **STR** 19(+4) **DEX** 8(-1) **CON** 16(+3) **INT** 5(-3) **WIS** 7(-2) **CHA** 7(-2)
- **CR** 2 (450 XP)
- **Attacks**:
  - Greatclub +6, 2d8+4 bludgeoning
  - Javelin +6, 2d6+4 piercing (range 30/120)

### Owlbear (Large monstrosity, unaligned)
- **AC** 13 (natural armor)
- **HP** 59 (7d10+21), Speed 40ft
- **STR** 20(+5) **DEX** 12(+1) **CON** 17(+3) **INT** 3(-4) **WIS** 12(+1) **CHA** 7(-2)
- **Skills** Perception +3
- **CR** 3 (700 XP)
- **Trait: Keen Sight and Smell** — advantage Perception (sight/smell)
- **Attacks (Multiattack)**: 1 beak + 1 claws
  - Beak +7, 1d10+5 piercing
  - Claws +7, 2d8+5 slashing

### Redbrand Ruffian (Humanoid, human, neutral evil)
- **AC** 14 (studded leather)
- **HP** 16 (3d8+3), Speed 30ft
- **STR** 11(+0) **DEX** 14(+2) **CON** 12(+1) **INT** 9(-1) **WIS** 9(-1) **CHA** 11(+0)
- **Skills** Intimidation +2
- **CR** 1/2 (100 XP)
- **Attacks (Multiattack)**: 2 melee
  - Shortsword +4, 1d6+2 piercing

### Sildar Hallwinter (Humanoid, human, neutral good) — NPC stat block
- **AC** 16 (chainmail)
- **HP** 27 (5d8+5), Speed 30ft
- **STR** 13(+1) **DEX** 10(+0) **CON** 12(+1) **INT** 10(+0) **WIS** 11(+0) **CHA** 10(+0)
- **Saving Throws** Str +3, Con +3
- **CR** 1 (200 XP)
- **Attacks (Multiattack)**: 2 melee
  - Longsword +3, 1d8+1 slashing
  - Heavy Crossbow +2, 1d10 piercing (range 100/400)
- **Reaction: Parry** — +1d6 AC khi bị melee hit (nếu đang wield melee)

### Mormesk the Wraith (Undead, neutral evil) — Wave Echo Cave
- **AC** 13
- **HP** 45 (6d8+18), Speed 0, fly 60ft
- **STR** 6(-2) **DEX** 16(+3) **CON** 16(+3) **INT** 12(+1) **WIS** 14(+2) **CHA** 15(+2)
- **Damage Resistances** acid, cold, fire, lightning, thunder; non-magical non-silvered
- **Damage Immunities** necrotic, poison
- **CR** — (không rõ, ước ~4)
- **Trait: Incorporeal Movement** — move through objects/creatures
- **Trait: Sunlight Sensitivity** — disadvantage khi trong sunlight
- **Attack: Life Drain** +5, 3d8+3 necrotic, DC 13 Con save hoặc max HP giảm = damage

### Nezznar the Black Spider (Humanoid, elf/drow, neutral evil) — MAIN BOSS
- **AC** 11 (14 with mage armor)
- **HP** 27 (6d8), Speed 30ft
- **STR** 9(-1) **DEX** 13(+1) **CON** 10(+0) **INT** 16(+3) **WIS** 14(+2) **CHA** 13(+1)
- **Saving Throws** Int +5, Wis +4
- **Skills** Arcana +5, Perception +4, Stealth +3
- **Senses** darkvision 120ft, passive Perception 14
- **CR** 2 (450 XP)
- **Special**: Spider Staff
- **Traits**:
  - Fey Ancestry (advantage vs charm, không bị sleep)
  - Sunlight Sensitivity
- **Innate Spellcasting**: dancing lights (at will), darkness + faerie fire (1/day, DC 12)
- **Spellcasting** (4th-level wizard, DC 13, +5 attack):
  - Cantrips: mage hand, ray of frost, shocking grasp
  - 1st (4 slots): mage armor, magic missile, shield
  - 2nd (3 slots): invisibility, suggestion
- **Attack: Spider Staff** +1, 1d6-1 bludgeoning + 1d6 poison

### Ochre Jelly (Large ooze)
- **AC** 8
- **HP** ~45, Speed 20ft, climb 10ft
- **CR** 2 (450 XP)
- **Trait: Split** — khi bị lightning/slashing và HP ≥ 10, chia thành 2 jelly nhỏ hơn
- **Attack: Pseudopod** +4, 2d6+2 bludgeoning + 1d6 acid

### Grick (Medium monstrosity) — Wave Echo Cave
- **AC** 14 (natural armor)
- **HP** 27 (6d8), Speed 30ft, climb 30ft
- **CR** 2 (450 XP)
- **Damage Resistances** non-magical weapons
- **Trait: Stone Camouflage** — advantage Stealth trong rocky terrain
- **Attacks (Multiattack)**: tentacles → beak nếu hit
  - Tentacles +4, 2d6+2 slashing
  - Beak +4, 1d6+2 piercing

### Spectator (Wave Echo Cave guardian) — *chưa trích đầy đủ*
- Beholder-kin, CR 3

### Young Green Dragon (Venomfang) — *Thundertree optional boss*
- **CR** 4, Deadly cho level 3 party
- Poison breath, fly 80ft
- Flee ở half HP

---

## 2. MAGIC ITEMS (Appendix A)

### Spider Staff (Iarno Glasstaff's weapon)
- **Requires attunement by a sorcerer, warlock, or wizard**
- Staff, ~5ft long, spider webs carved along length
- Hold + speak command → spider climb (tự động)
- **10 charges**, regain 1d6+4 daily at dawn:
  - 1 charge: web (save DC 13)
  - 2 charges: giant spider (xuất hiện 1 con)
- **Melee**: +1, 1d6-1 bludgeoning + 1d6 poison
- When charges cạn + roll 1 on d20 → staff crumbles thành dust

### Staff of Defense (Wave Echo Cave)
- **Requires attunement**
- Cho phép cast mage armor (1 charge, tự động daily)
- 10 charges, regain 1d6+4 daily at dawn:
  - 1 charge: shield (reaction)
  - 1 charge: expeditious retreat
- **Melee bonus**: +1 AC khi cầm
- +1 to melee attacks

### Lightbringer (Magic mace, Wave Echo Cave crypt)
- **Requires attunement**
- Mace +1 (đánh +1, damage +1)
- **Property**: Emit light (bright 20ft, dim 20ft thêm) — command word
- **Bonus**: Advantage vs undead

### Dragonthan (Magic scale mail, Wave Echo Cave)
- **Requires attunement**
- Scale mail +1 (+1 AC)
- **Property**: Resistance acid (half damage)
- Made from green dragon scales

### Boots of Striding and Springing (Wave Echo Cave, trên người Thaden)
- **Requires attunement**
- Speed không bị giảm bởi encumbrance/heavy armor
- **Jump**: Dex check với advantage, jump distance ×3

### Scrolls (tìm được trong adventure)
- **Scroll of charm person** (Iarno's chest)
- **Scroll of fireball** (Iarno's chest)
- **Scroll of misty step** (Venomfang's hoard)
- **Scroll of lightning bolt** (Venomfang's hoard)

### Common items (không magic)
- Potions of Healing (multiple — Klarg's chest, cistern, Sister Garaele reward)
- Potion of Invisibility (Redbrand cistern satchel)

---

## 3. BẢNG SO SÁNH: ENGINE CÓ vs CẦN THÊM

| Monster | Có trong `monsters.c`? | Action |
|---|---|---|
| Goblin | ✅ ID_GOBLIN | — |
| Wolf | ✅ ID_WOLF | — |
| Bugbear | ❌ | **Thêm** (cho Klarg) |
| Orc | ✅ ID_ORC | — |
| Ogre | ✅ ID_OGRE | — |
| Skeleton | ✅ ID_SKELETON | — |
| Zombie | ✅ ID_ZOMBIE | — |
| Redbrand Ruffian | ❌ | **Thêm** (Phandalin/Hideout) |
| Hobgoblin | ❌ | **Thêm** (wilderness) |
| Owlbear | ❌ | **Thêm** (wilderness) |
| Grick | ❌ | **Thêm** (Wave Echo) |
| Spectator | ❌ | **Thêm** (Wave Echo) |
| Ochre Jelly | ❌ | **Thêm** (Wave Echo, Split trait) |
| Wraith (Mormesk) | ❌ | **Thêm** (Wave Echo) |
| Drow Wizard (Nezznar) | ❌ | **Thêm** (boss cuối) |
| Young Green Dragon | ✅ ID_DRAGON | Rename/adjust stats cho Venomfang |
| Sildar (NPC stat) | ❌ | **Thêm** (NPC combat-capable) |

→ **Cần thêm ~11 monster entries** (mỗi entry ~15 dòng trong `monsters.c`)

| Magic Item | Có trong `items.c`? | Action |
|---|---|---|
| Longsword | ✅ ID_LONGSWORD | — |
| Chain Shirt | ✅ ID_CHAINSHIRT | — |
| Heal Potion | ✅ ID_HEAL_POTION | — |
| Dagger | ✅ ID_DAGGER | — |
| Spider Staff | ❌ | **Thêm** (magic + charges) |
| Staff of Defense | ❌ | **Thêm** (magic + charges) |
| Lightbringer | ❌ | **Thêm** (magic mace) |
| Dragonthan | ❌ | **Thêm** (magic armor) |
| Boots of Striding | ❌ | **Thêm** (magic wondrous) |
| Scrolls (4 loại) | ❌ | **Thêm** (consumable magic) |
| Potion of Invisibility | ❌ | **Thêm** (consumable magic) |

→ **Cần thêm ~10 magic item entries**

---

## 4. WORKFLOW THÊM DATA

Mỗi entry mới = copy template từ `monsters.c`/`items.c` hiện có + đổi stats. DSL macros (`DICE`, `STATS`, `CR`, `SIZE_*`) đã hỗ trợ.

Ví dụ Bugbear:
```c
[ID_BUGBEAR] = {
    .name = "Bugbear",
    .size = SIZE_MEDIUM, .type = TYPE_HUMANOID,
    .ac = 16, .hp_dice = DICE(5, 8, 5), .speed = 6,
    .scores = STATS(STR(15), DEX(14), CON(13), INT(8), WIS(11), CHA(9)),
    .cr = CR(1), .xp = 200,
    .glyph = 'B', .glyph_color = CE_RED,
    .actions = bugbear_actions, .n_actions = 2,
    .ai = ai_melee_chaser,
},
```

Size ước tính: 11 monster × 15 dòng + 10 item × 10 dòng = **~265 dòng code, ~3 KB exe**.
