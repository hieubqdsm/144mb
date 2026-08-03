"""
FLOW_SIM.PY - LMoP Flow Simulator (orchestrator chính).

Chạy toàn bộ flow campaign LMoP intro + town với:
  - DM agent narrate + adjudicate
  - 4 player agent quyết định action
  - Engine adjudicate luật D&D (perception/surprise/combat/loot/choice)

Phase 1 (file này): dùng bot heuristic (MockDecisionMaker), KHÔNG cần API key.
Phase 2: thay MockDecisionMaker bằng LLM agent thật (llm_client.py).

Cách chạy:
  python flow_sim.py              # mock mode (heuristic bot)
  python flow_sim.py --llm        # LLM mode (cần API key)
  python flow_sim.py --seed 42    # deterministic
"""

import sys
import random
import argparse

from engine import (Actor, GameState, resolve_attack, d20, add_init,
                    roll_surprise, gen_loot, offer_loot_to_party)
from campaign import NODES, get_node, next_after_action, SceneNode
from protocol import validate_action, make_action, parse_llm_json
from agents import make_party, make_monsters


# =============================================================================
# CONSOLE OUTPUT HELPERS
# =============================================================================

def sep(title: str):
    print("\n" + "═" * 70)
    print(f"  {title}")
    print("═" * 70)

def dm_say(text: str):
    """DM narration box."""
    print("\n┌─ DM ────────────────────────────────────────────────────────────┐")
    for line in text.split("\n"):
        print(f"│ {line}")
    print("└──────────────────────────────────────────────────────────────────┘")

def player_say(name: str, text: str):
    print(f"  💬 {name}: {text}")

def engine_log(text: str):
    print(f"    ⚙️  {text}")

def result_log(text: str):
    print(f"    ✅ {text}")


# =============================================================================
# MOCK DECISION MAKER (Phase 1 — bot heuristic, không cần LLM)
# =============================================================================

class MockDecisionMaker:
    """
    Stand-in cho LLM. Mỗi agent có personality riêng để quyết định khác nhau.
    Trong Phase 2 sẽ thay bằng LLM thật.
    """
    PERSONALITIES = {
        "Thorin": "hung hăng, luôn tấn công trước",
        "Elara": "pháp sư, ưu tiên spell nếu nhiều enemy",
        "Lyra": "rogue, thích nói chuyện + chiến thuật",
        "Bjorn": "cleric, thận trọng, ưu tiên heal nếu HP thấp",
    }

    def __init__(self, name: str):
        self.name = name
        self.personality = self.PERSONALITIES.get(name, "cân bằng")

    def decide_combat(self, state: GameState, actor: Actor) -> dict:
        """Heuristic combat: attack lowest-HP enemy. Personality thay đổi chat."""
        targets = [a for a in state.actors if a.alive and a.team != actor.team]
        if not targets:
            return make_action("wait", say=f"{self.name} không thấy mục tiêu.")
        target = min(targets, key=lambda a: a.hp)
        say = ""
        if self.name == "Thorin":
            say = f"Tôi giữ tên {target.name}!"
        elif self.name == "Elara":
            say = f"Phép thuật cho {target.name}."
        elif self.name == "Lyra":
            say = f"Đội ơi tập trung vào {target.name}."
        elif self.name == "Bjorn":
            say = f"Theo tôi, đánh {target.name}."
        return make_action("attack", target=target.name, say=say)

    def decide_loot(self, item: dict, party: list) -> dict:
        """Heuristic loot: luôn take, give_to người phù hợp."""
        give_to = party[0].name  # default Fighter
        if "Potion" in item.get("name", ""):
            give_to = next((a.name for a in party if "Bjorn" in a.name), party[0].name)
        elif "Shortbow" in item.get("name", ""):
            give_to = next((a.name for a in party if "Lyra" in a.name), party[0].name)
        return {"take": True, "give_to": give_to}

    def decide_town_choice(self, choices: list) -> dict:
        """Heuristic: chọn talk Barthen lần đầu, leave_town lần sau (tránh infinite loop)."""
        if not hasattr(self, "_town_visited"):
            self._town_visited = False
        if not self._town_visited:
            self._town_visited = True
            # Lần đầu: chọn talk NPC đầu tiên (Barthen)
            return {"chosen": choices[0]["id"]}
        # Lần sau: rời town
        leave = next((c["id"] for c in choices if c["id"] == "leave_town"), choices[-1]["id"])
        return {"chosen": leave}


class _TownGuard:
    """Shared stateful guard chống infinite loop town (dùng cho cả mock + LLM)."""
    _visits = {}   # node_id -> count

    @classmethod
    def decide(cls, node_id, choices):
        n = cls._visits.get(node_id, 0) + 1
        cls._visits[node_id] = n
        # Lần đầu: talk NPC đầu. Lần sau: leave.
        if n <= 1:
            return {"chosen": choices[0]["id"]}
        leave = next((c["id"] for c in choices if c["id"] == "leave_town"), choices[-1]["id"])
        return {"chosen": leave}


# =============================================================================
# LLM DECISION MAKER (Phase 2 — LLM thật, cần API key)
# =============================================================================

class LLMDecisionMaker:
    """
    Mỗi agent = 1 LLM call. Đọc state + history, function-call trả JSON action.
    Fallback heuristic nếu LLM fail (no key / network error).
    """
    PERSONALITIES = {
        "Thorin": "Fighter dũng cảm, luôn tiên phong tấn công. Nói chuyện trực tiếp, ngắn gọn.",
        "Elara": "Wizard thông minh, ưu tiên phép thuật nếu nhiều enemy. Hay phân tích tình hình.",
        "Lyra": "Rogue thận trọng, hay quan sát + góp ý chiến thuật. Thích đánh lén kẻ yếu.",
        "Bjorn": "Cleric đức độ, quan tâm đồng đội, ưu tiên heal nếu ai HP thấp. Nói năng tích cực.",
    }

    def __init__(self, name: str, llm_client):
        self.name = name
        self.llm = llm_client
        self.personality = self.PERSONALITIES.get(name, "cân bằng")
        self.history = []  # conversation memory

    def _build_system(self) -> str:
        return (f"Bạn là {self.name}, một hero trong party D&D 5e. "
                f"Tính cách: {self.personality}\n"
                f"Trả lời qua function choose_action. "
                f"Bạn nói tiếng Việt. Cooperate với đồng đội (có thể dùng trường 'say').")

    def decide_combat(self, state, actor) -> dict:
        if not self.llm or not self.llm.is_available():
            return MockDecisionMaker(self.name).decide_combat(state, actor)
        try:
            snap = state.snapshot()
            foes = [a for a in snap["actors"] if a["team"] == "monsters" and a["alive"]]
            user_msg = (f"Lượt của bạn. HP bạn: {actor.hp}/{actor.max_hp}. "
                        f"Kẻ địch còn sống: {[(f['name'], f['hp']) for f in foes]}. "
                        f"Chọn action (attack target / cast spell / say).")
            from llm_client import TOOL_COMBAT_ACTION
            resp = self.llm.chat(self._build_system(),
                                 [{"role": "user", "content": user_msg}],
                                 tools=[TOOL_COMBAT_ACTION],
                                 tool_choice={"type": "function", "function": {"name": "choose_action"}})
            if resp.get("error"):
                print(f"    ⚠️ LLM error: {resp['error'][:80]}. Fallback heuristic.")
                return MockDecisionMaker(self.name).decide_combat(state, actor)
            for tc in resp.get("tool_calls", []):
                args = tc.get("function", {}).get("arguments", "{}")
                from protocol import parse_llm_json
                parsed = parse_llm_json(args) or {}
                if parsed.get("verb"):
                    return parsed
        except Exception as e:
            print(f"    ⚠️ LLM exception: {str(e)[:80]}. Fallback heuristic.")
        return MockDecisionMaker(self.name).decide_combat(state, actor)

    def decide_loot(self, item, party) -> dict:
        if not self.llm or not self.llm.is_available():
            return MockDecisionMaker(self.name).decide_loot(item, party)
        try:
            from llm_client import TOOL_LOOT_DECISION
            party_names = [a.name for a in party]
            user_msg = (f"Loot được: {item['name']}. Party: {party_names}. "
                        f"Bạn (đại diện party) chọn take/leave + give_to ai.")
            resp = self.llm.chat(self._build_system(),
                                 [{"role": "user", "content": user_msg}],
                                 tools=[TOOL_LOOT_DECISION])
            if resp.get("error"):
                return MockDecisionMaker(self.name).decide_loot(item, party)
            for tc in resp.get("tool_calls", []):
                args = tc.get("function", {}).get("arguments", "{}")
                from protocol import parse_llm_json
                parsed = parse_llm_json(args) or {}
                if "take" in parsed:
                    return parsed
        except Exception:
            pass
        return MockDecisionMaker(self.name).decide_loot(item, party)

    def decide_town_choice(self, choices: list) -> dict:
        """LLM chọn town action. Fallback _TownGuard nếu LLM fail (chống infinite loop)."""
        if not self.llm or not self.llm.is_available():
            return _TownGuard.decide("town", choices)
        try:
            from llm_client import TOOL_TOWN_CHOICE
            options_text = "\n".join(f"- {c['id']}: {c['label']}" for c in choices)
            user_msg = f"Đang ở thị trấn. Chọn 1:\n{options_text}"
            resp = self.llm.chat(self._build_system(),
                                 [{"role": "user", "content": user_msg}],
                                 tools=[TOOL_TOWN_CHOICE])
            if not resp.get("error"):
                for tc in resp.get("tool_calls", []):
                    args = tc.get("function", {}).get("arguments", "{}")
                    from protocol import parse_llm_json
                    parsed = parse_llm_json(args) or {}
                    if parsed.get("chosen"):
                        # Validate chosen id hợp lệ
                        valid_ids = [c["id"] for c in choices]
                        if parsed["chosen"] in valid_ids:
                            return parsed
            # LLM fail hoặc chọn sai → fallback guard
            print("    ⚠️ LLM town choice fail. Fallback guard.")
        except Exception:
            pass
        return _TownGuard.decide("town", choices)


# =============================================================================
# COMBAT RUNNER (round loop)
# =============================================================================

def run_combat(node: SceneNode, party: list, rng: random.Random,
               deciders: dict, party_surprised: list = None) -> dict:
    """
    Combat loop đầy đủ: initiative + surprise + round-robin.
    party_surprised: list tên bị surprise (mất turn round 1).
    Return {"winner": "party"|"monsters", "rounds": N}
    """
    party_surprised = party_surprised or []

    # Spawn monsters
    monsters = make_monsters()[:node.monster_count]
    # Scale nếu count khác default
    while len(monsters) < node.monster_count:
        monsters.append(Actor(f"Goblin {chr(65+len(monsters))}", "monsters",
                              hp=7, max_hp=7, ac=15, atk_bonus=4, damage_dice="1d6+2",
                              glyph="g", dex_score=14, wis_score=8, stealth_bonus=6))

    state = GameState(actors=[], seed=rng.randint(0, 999999))
    for a in party + monsters:
        add_init(a, rng)
    state.actors = party + monsters
    state.actors.sort(key=lambda a: a.init, reverse=True)

    engine_log(f"Initiative: {[(a.name, a.init) for a in state.actors]}")
    if party_surprised:
        engine_log(f"Party bị SURPRISE: {party_surprised} (mất turn round 1)")

    max_rounds = 20
    while state.round <= max_rounds and not state.combat_over:
        for actor in list(state.actors):
            if not actor.alive or state.combat_over:
                continue
            # Skip nếu surprised ở round 1
            if state.round == 1 and actor.name in party_surprised:
                print(f"  😴 {actor.name} bị surprise, bỏ qua turn này.")
                continue

            # Player hoặc monster decide
            if actor.team == "party":
                d = deciders[actor.name]
                action = d.decide_combat(state, actor)
            else:
                # Monster AI heuristic
                targets = [a for a in state.actors if a.alive and a.team != actor.team]
                if not targets:
                    continue
                target = min(targets, key=lambda a: a.hp)
                action = make_action("attack", target=target.name)

            # Show chat
            if action.get("say"):
                player_say(actor.name, action["say"])

            # Validate + execute
            v = validate_action(action, {"attack", "wait", "say"},
                                state.snapshot())
            if not v["ok"]:
                print(f"  ⚠️ {actor.name}: {v['error']}")
                continue
            a = v["action"]
            if a["verb"] == "attack":
                target = next((x for x in state.actors if x.name == a["target"]), None)
                if target:
                    r = resolve_attack(actor, target, rng)
                    state.log.append(r)
                    tag = "💥 CRIT" if r["crit"] else ("⚔️ HIT" if r["hit"] else "💨 miss")
                    print(f"  {tag} {actor.name} → {target.name}: "
                          f"{r['damage']}dmg (HP {r['target_hp_after']}/{target.max_hp})")

            winner = state.check_combat_over()
            if winner:
                state.combat_over = True
                result_log(f"{winner.upper()} thắng sau {state.round} round!")
                return {"winner": winner, "rounds": state.round}

        state.round += 1

    return {"winner": "draw", "rounds": max_rounds}


# =============================================================================
# SCENE HANDLERS (mỗi scene_type 1 handler)
# =============================================================================

def handle_intro(node: SceneNode, party, rng, deciders):
    dm_say(node.narration)
    return {"next": node.next}

def handle_travel(node: SceneNode, party, rng, deciders):
    dm_say(node.narration)
    # Ẩn: DM roll perception vs stealth
    monsters = make_monsters()[:node.monster_count or 4]
    result = roll_surprise(party, monsters, rng, node.stealth_bonus)
    engine_log(f"Passive Perception team: {max(a.passive_perception() for a in party)}")
    engine_log(f"Monster Stealth roll: {result['monster_stealth_roll']} (d20+{node.stealth_bonus})")

    if result["spotted"]:
        player_say("Bjorn", "Khoan! Tôi thấy bóng trong lùm cây... Goblins!")
        dm_say("Đội giành инициатив. Goblin chưa kịp phục kích trọn vẹn.")
        return {"spotted": True, "next": node.on_spotted,
                "party_surprised": []}
    else:
        dm_say("Goblins lao ra từ bụi rậm! Đội bị bắt ngờ hoàn toàn.")
        return {"spotted": False, "next": node.on_surprised,
                "party_surprised": result["party_surprised"]}

def handle_combat(node: SceneNode, party, rng, deciders, party_surprised=None):
    dm_say(node.narration)
    result = run_combat(node, party, rng, deciders, party_surprised)
    return {"combat_result": result, "next": node.next,
            "winner": result["winner"]}

def handle_loot(node: SceneNode, party, rng, deciders):
    dm_say(node.narration)
    loot = gen_loot(node.loot_source, 4, rng)
    engine_log(f"Loot roll: {loot['gold']} gold + {len(loot['items'])} items")
    # Dùng engine offer_loot_to_party: AI chọn take/leave/give_to, engine update inventory
    d = deciders["Bjorn"]  # AI loot decider (demo)
    dist = offer_loot_to_party(party, loot, d.decide_loot)
    for entry in dist["distributed"]:
        player_say(entry["actor"], f"Nhận: {entry['item']}")
    for item in dist["left_items"]:
        player_say("Lyra", f"Bỏ qua: {item['name']}")
    if dist["total_gold"] > 0:
        engine_log(f"Mỗi thành viên nhận {dist['gold_each']} gold (tổng {dist['total_gold']}).")
    return {"loot": loot, "next": node.next}

def handle_town(node: SceneNode, party, rng, deciders):
    dm_say(node.narration)
    print("\n  Lựa chọn:")
    for i, c in enumerate(node.choices):
        print(f"    {i+1}. {c['label']}")
    # AI chọn (demo: decider đầu tiên chọn)
    d = deciders[party[0].name]
    decision = d.decide_town_choice(node.choices)
    chosen = decision["chosen"]
    chosen_label = next(c["label"] for c in node.choices if c["id"] == chosen)
    player_say(party[0].name, f"Tôi chọn: {chosen_label}")
    return {"chosen": chosen, "next": next_after_action(node, {"chosen": chosen})}

def handle_end(node: SceneNode, party, rng, deciders):
    dm_say(node.narration)
    return {"next": ""}


HANDLERS = {
    "INTRO":  handle_intro,
    "TRAVEL": handle_travel,
    "COMBAT": handle_combat,
    "LOOT":   handle_loot,
    "TOWN":   handle_town,
    "END":    handle_end,
}


# =============================================================================
# MAIN FLOW SIMULATOR
# =============================================================================

def run_flow(start_node_id: str = "neverwinter_inn", seed: int = 42,
             use_llm: bool = False):
    """Chạy flow graph từ node bắt đầu."""
    sep(f"LMoP FLOW SIMULATOR — seed={seed} — mode={'LLM' if use_llm else 'MOCK'}")
    rng = random.Random(seed)
    party = make_party()

    # Phase 1: mock deciders. Phase 2: LLM deciders (fallback mock nếu fail).
    if use_llm:
        from llm_client import LLMClient
        client = LLMClient()
        if client.is_available():
            print(f"  ✅ LLM mode: model={client.model}")
            deciders = {a.name: LLMDecisionMaker(a.name, client) for a in party}
        else:
            print("  ⚠️  Không có API key (LLM_API_KEY/OPENAI_API_KEY). Fallback MOCK mode.")
            deciders = {a.name: MockDecisionMaker(a.name) for a in party}
    else:
        deciders = {a.name: MockDecisionMaker(a.name) for a in party}

    sep(f"ĐỘI HÙNG (Party)")
    for a in party:
        print(f"  {a.name:8s} HP {a.hp}/{a.max_hp}  AC {a.ac}  "
              f"WIS {a.wis_score}(passive {a.passive_perception()})  DEX {a.dex_score}")
    print()

    node_id = start_node_id
    party_surprised = []
    max_visits = 50   # hard limit chống infinite loop
    visit_count = 0
    while node_id:
        visit_count += 1
        if visit_count > max_visits:
            print(f"\n⚠️ Dừng: vượt {max_visits} node visits (chống infinite loop).")
            break
        node = get_node(node_id)
        if not node:
            print(f"❌ Node không tồn tại: {node_id}")
            break
        sep(f"{node.title}")
        handler = HANDLERS.get(node.scene_type)
        if not handler:
            print(f"❌ Không có handler cho scene_type: {node.scene_type}")
            break

        # Pass party_surprised cho combat handler
        if node.scene_type == "COMBAT":
            result = handler(node, party, rng, deciders, party_surprised)
            party_surprised = []  # reset
        else:
            result = handler(node, party, rng, deciders)

        node_id = result.get("next", "")
        if not node_id:
            break
        # Preserve surprise state từ travel → combat
        if node.scene_type == "TRAVEL" and result.get("party_surprised"):
            party_surprised = result["party_surprised"]

    sep("HẾT FLOW SIMULATOR")
    # Final state
    print("\nTrạng thái party cuối:")
    for a in party:
        inv_summary = ", ".join(f"{i['name']}×{i['qty']}" for i in a.inventory) or "trống"
        print(f"  {a.name:8s} HP {a.hp}/{a.max_hp}  Túi: {inv_summary}")


# =============================================================================
# ENTRY POINT
# =============================================================================

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="LMoP Flow Simulator")
    parser.add_argument("--llm", action="store_true",
                        help="Dùng LLM thật (cần API key). Default: mock heuristic.")
    parser.add_argument("--seed", type=int, default=42, help="Random seed (default 42)")
    parser.add_argument("--start", default="neverwinter_inn", help="Node bắt đầu")
    args = parser.parse_args()
    run_flow(start_node_id=args.start, seed=args.seed, use_llm=args.llm)
