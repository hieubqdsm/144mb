# AI Party — LMoP Flow Simulator

> Simulator Python chạy D&D 5e combat flow với **LLM agents** (DM + 4 player).
> Dùng để **test toàn bộ cơ chế** (perception/surprise/combat/loot/choice) trước khi
> tốn thời gian làm art/map cho game C thật.

## Kiến trúc

```
┌─────────────┐     INTENT JSON      ┌─────────────┐
│ DM Agent    │ ───────────────────► │   Engine    │  (rule judge)
│ (LLM)       │ ◄───── result ────── │ (Python)    │
└─────────────┘                      └─────────────┘
       ▲ narrate                             ▲
       │                                     │ execute
┌──────┴──────┐     INTENT JSON              │
│ Player x4   │ ─────────────────────────────┘
│ (LLM agents)│
└─────────────┘
```

- **Engine** = luật gia bất biến (dice/AC/damage/surprise/loot) — LLM không bao giờ tự tính
- **Agents** = diễn viên (LLM đưa INTENT qua function calling, không free text)
- **Flow graph** = node campaign (INTRO → TRAVEL → AMBUSH → COMBAT → LOOT → TOWN)

## Cách chạy

### Mock mode (không cần API key — bot heuristic)
```bash
cd ai_party
python flow_sim.py                 # seed mặc định 42
python flow_sim.py --seed 7        # seed khác
python flow_sim.py --start combat_spotted  # bắt đầu từ node cụ thể
```

### LLM mode (cần API key)
```bash
# Set env var (OpenAI / compatible)
export OPENAI_API_KEY="sk-..."
# HOẶC
export LLM_API_KEY="sk-..."
export LLM_MODEL="gpt-4o-mini"        # default
export LLM_BASE_URL="https://api.openai.com/v1"  # đổi cho proxy/local

python flow_sim.py --llm
```

Nếu không có API key → tự **fallback mock** (chạy được luôn).

## Files

| File | Vai trò |
|---|---|
| `engine.py` | Luật gia: Actor/dice/combat/surprise/loot/choice. Tái dùng + bổ sung từ bản cũ |
| `agents.py` | 4 hero pre-made (Thorin/Elara/Lyra/Bjorn) + monster spawn |
| `campaign.py` | Flow graph LMoP intro + town (6 node) |
| `protocol.py` | Action JSON schema + validator (chặn LLM nonsense) |
| `flow_sim.py` | Orchestrator chính: DM→player→engine loop. Có Mock + LLM DecisionMaker |
| `llm_client.py` | OpenAI-compatible wrapper (urllib stdlib, không cần requests) |

## Cơ chế D&D đã test được

| Cơ chế | File | Status |
|---|---|---|
| Initiative (d20+DEX, sort) | engine.py | ✅ |
| Combat (attack/crit/fumble/AC) | engine.py | ✅ |
| Surprise (Stealth vs passive Perception) | engine.py `roll_surprise` | ✅ |
| Loot (CR-based treasure table) | engine.py `gen_loot` | ✅ |
| Loot distribution (take/leave/give_to) | engine.py `offer_loot_to_party` | ✅ |
| Choice node (branch flow) | campaign.py | ✅ |
| Town hub (NPC list + choices) | campaign.py | ✅ |
| Coop communication (say field) | protocol.py | ✅ |
| Personality agents (4 hero khác tính cách) | flow_sim.py | ✅ |

## Flow graph LMoP (intro + town)

```
neverwinter_inn (INTRO)
       │
       ▼
triboar_travel (TRAVEL — perception check)
       │
       ├─ spotted (Bjorn WIS 16 → passive 13 > stealth) ──► combat_spotted
       │
       └─ surprised (party bị bất ngờ) ─────────────────► combat_surprised
                                     │
                                     ▼
                            post_ambush_loot (LOOT)
                                     │
                                     ▼
                            phandalin_arrival (TOWN — 5 choices)
                                     │
                                     ├─ talk_barthen → (loop, talk NPC)
                                     ├─ rest_inn → (loop)
                                     ├─ talk_halia → (loop)
                                     ├─ talk_garaele → (loop)
                                     └─ leave_town → end_demo
```

## Personality system

Mỗi hero có personality riêng → quyết định khác nhau:

| Hero | Class | Personality |
|---|---|---|
| **Thorin** | Fighter | Dũng cảm, tiên phong, nói ngắn gọn |
| **Elara** | Wizard | Thông minh, ưu tiên spell, hay phân tích |
| **Lyra** | Rogue | Thận trọng, quan sát + góp ý chiến thuật |
| **Bjorn** | Cleric | Đức độ, ưu tiên heal, nói tích cực |

Trong mock mode: personality ảnh hưởng chat text. Trong LLM mode: personality vào system prompt.

## Kết nối với game C thật

Engine Python = **reimpl** của engine C (`src_console/`). Sau khi flow verify OK,
chuyển sang C bằng cách:
1. Port `campaign.py` sang C struct (`TilemapDef` + `SceneNode`)
2. Port `engine.py` đã có sẵn trong C (`combat.c`/`d20.c`/`turn.c`)
3. Plug dialogue thật (đã có `data/dialogues.c`) vào town node

→ Flow simulator = **bản vẽ kỹ thuật** trước khi xây game thật.
