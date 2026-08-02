# Đánh giá Budget 144mb — dựa trên LMoP thực tế

> Số liệu đếm từ module LMoP (thay vì ước lượng trước).
> LMoP = benchmark "medium campaign" được chơi nhiều nhất.

---

## 1. Asset manifest THỰC TẾ (đếm từ LMoP)

| Hạng mục | Số lượng | Ghi chú |
|---|---|---|
| **Tiles** | ~30 | 3 biomes (cave/town/crypt) × ~8 tile/biome + variants |
| **Monster sprites** | ~25 | goblin, wolf, bugbear, skeleton, zombie, orc, dragon, drow, spectator, wraith... |
| **NPC sprites** | ~20 | Gundren, Sildar, Barthen, Linene, Halia, Garaele, Reidoth, Agatha, King Grol, Venomfang... |
| **Magic items** | ~12 | potions, Spider Staff, Staff of Defense, Lightbringer, Dragonthan, +1 weapons |
| **Boss sprites** | 4 | Klarg, King Grol, Nezznar, Venomfang (large) |
| **Dungeon maps** | 5 | Cragmaw Hideout, Redbrand, Cragmaw Castle, Wave Echo, Thundertree |
| **Town/wilderness maps** | 4 | Phandalin, Triboar Trail, wilderness sandbox |
| **Cutscene art** | 4 | Chapter intros |
| **Dialogue text** | ~20 NPC × ~10-15 lines | = 200-300 lines dialogue |
| **Story/quest text** | ~10 quests | Background, hooks, journals |

**Tổng: ~95 art assets + text content**

---

## 2. Tính lại budget cho LMoP (không phải "full medium")

### Approach A: ASCII .xp (current path)

```
exe base (engine + game logic):           175 KB
tiles (30 × 3×3 .xp):                       3 KB
monsters (25 × 4×4 .xp):                    3 KB
npc (20 × 4×4 .xp):                         3 KB
items (12 × 2×2 .xp):                       1 KB
boss (4 × 6×6 .xp):                         2 KB
cutscenes (4 × 100×50 .xp):               118 KB   ← ĐẮT NHẤT
---------------------------------------- ----------
ART SUBTOTAL:                            ~130 KB
---------------------------------------- ----------
Còn cho content (maps + dialogue + story):
  1440 - 175 - 130 =                    1,135 KB
```

### Approach B: 1-bit RLE hybrid (ASCII tiles + 1-bit large art)

```
exe base:                                 175 KB
tiles (30 × 3×3 ASCII):                    3 KB
monsters/npc/items (ASCII):                7 KB
boss (4 × 1-bit 32×32):                    1 KB
cutscenes (4 × 1-bit 800×400):            28 KB   ← nhẹ hơn ASCII 4×
+ 1-bit decoder code:                      1 KB
---------------------------------------- ----------
ART SUBTOTAL:                             ~40 KB
---------------------------------------- ----------
Còn cho content:
  1440 - 175 - 40 =                      1,225 KB
```

### Còn cho content (dialogue + maps + story)

| Content type | Size ước lượng | Vừa không? |
|---|---|---|
| Dialogue (~300 lines × ~80 char) | ~25 KB | ✅ cực dư |
| Map layouts (9 maps × ~2KB ASCII tilemap) | ~18 KB | ✅ cực dư |
| Story/quest text | ~10 KB | ✅ cực dư |
| Music (nếu muốn, 1-2 MIDI) | ~200-400 KB | ✅ dư (cả 2 approach) |

---

## 3. Phán quyết: LMoP FIT 1.44MB ở CẢ 2 hướng

| | ASCII thuần | 1-bit hybrid |
|---|---|---|
| **LMoP art** | 130 KB | 40 KB |
| **Còn cho content** | 1,135 KB | 1,225 KB |
| **Fit?** | ✅ Dư dả | ✅ Rất dư dả |

→ **Không có áp lực size nào**. Ngay cả ASCII thuần (nặng nhất) cũng chỉ dùng 130KB art, còn 1.1MB cho content. Chênh lệch 2 hướng chỉ ~90KB — không đáng kể cho LMoP.

### Cutscene là phần "đắt" duy nhất
- 4 cutscene × 100×50 cell = 118KB ASCII (91% art budget)
- 4 cutscene × 800×400px = 28KB 1-bit (70% ít hơn)
- → Nếu skip cutscene (dùng text intro thay ASCII art), art chỉ còn **~12KB**

---

## 4. blockers kỹ thuật cho LMoP (KHÔNG phải size)

Dựa trên đọc module, đây là thứ **thực sự thiếu** để làm LMoP:

| # | Blocker | Status hiện tại | Ảnh hưởng |
|---|---|---|---|
| 1 | **Tilemap loader** | ❌ Chưa có (chỉ procedural BSP) | Map LMoP có chủ ý (room sequence, traps) — procedural không tái tạo được |
| 2 | **Dialogue system** | ❌ Chưa có | 20 NPC × dialogue = core của town hub Phandalin |
| 3 | **Quest system** | ❌ Chưa có | 10 quests (Gundren, Sildar, Redbrands, Agatha, orcs...) |
| 4 | **Targeting mouse** | ❌ Auto-nearest only | Combat LMoP cần chọn mục tiêu (ranged spells) |
| 5 | **Multi-region layout** | ⚠️ Foundation có (2 HFONT) | Town + wilderness + dungeon cần layout khác |
| 6 | **Death saves / resting** | ⚠️ Engine có, chưa plug | LMoP cần long rest giữa dungeon |
| 7 | **Vendor/shop** | ❌ Chưa có | Barthen's Provisions, Lionshield Coster |
| 8 | **NPC AI (neutral)** | ⚠️ Chỉ monster AI | NPC town không đánh, chỉ dialogue |

**→ Size không phải vấn đề. Workflow features mới mới là bottleneck.**

---

## 5. Khuyến nghị roadmap (dựa trên LMoP)

Nếu chọn LMoP làm target campaign, thứ tự feature cần làm:

### Phase 1: Foundation (để LMoP playable)
1. **Tilemap loader** — load map vẽ tay (cho 5 dungeon + town)
2. **Dialogue system** — NPC nói chuyện (cho 20 NPC Phandalin)
3. **Plug save/load + resting + death saves** — đã có engine, chỉ wire-up

### Phase 2: Town hub
4. **Shop/vendor system** — mua bán ở Barthen's, Lionshield
5. **Quest tracker** — log quest, objective, reward
6. **NPC neutral AI** — NPC không attack, chỉ dialogue

### Phase 3: Wilderness
7. **Multi-region layout** — chuyển town ↔ wilderness ↔ dungeon
8. **Overworld map** — travel giữa locations (Triboar Trail)
9. **Random encounter** — wandering monsters

### Phase 4: Polish
10. **Sprite .xp** — plug vào draw_map (tool đã có sẵn)
11. **Targeting mouse** — chọn mục tiêu combat
12. **Concentration check** — đã có engine, wire-up

**Art assets (sprite) làm ở Phase 4** — không phải bottleneck, vì engine + tool đã sẵn.

---

## 6. Kết luận cuối cùng

**LMoP hoàn toàn khả thi cho 1.44MB.** Câu hỏi không phải "có đủ chỗ không" (rất dư), mà là **"có đủ tính năng game không"**.

| Câu hỏi | Trả lời |
|---|---|
| Art size có vấn đề? | ❌ Không. 40-130KB, dư >1MB |
| ASCII hay 1-bit? | ASCII đủ dùng. 1-bit chỉ lợi nếu muốn +music/cutscene nhiều |
| Có cần raylib? | ❌ Không. GDI + ASCII đủ cho LMoP |
| SVG có cần? | ❌ Không. Overkill cho roguelike |
| Bottleneck thực sự? | ✅ **Features**: tilemap loader, dialogue, quest, shop |

**→ Nên dừng đánh giá art format. Focus vào tilemap loader + dialogue system trước.**
