"""
AGENTS.PY - DM LLM + 4 player LLM (mock, không cần API key).

Đây minh họa cách 5 LLM tương tác. Trong bản thật sẽ thay mock bằng:
  - Claude/GPT/Gemini qua OpenAI-compatible API
  - function calling để LLM đưa INTENT thay vì text tự do
  - conversation history + memory summary

Kiến trúc:
  DM agent     -> mô tả scene, gọi NPC, kết nối các scene
  Player agent -> nhận state, chọn action (attack/cast/move)
  Engine       -> luật gia, thực thi action, trả kết quả text

Luồng mỗi turn:
  1. DM mô tả trạng thái (narration)
  2. Player (whoes_turn) đọc state -> chọn action
  3. Engine thực thi -> kết quả
  4. DM tường thuật kết quả
  5. Next turn
"""

import random
import json
from engine import Actor, GameState, resolve_attack, d20, add_init


# =============================================================================
# MOCK LLM CALL - thay bằng API thật
# =============================================================================

class MockLLM:
    """
    Stand-in cho LLM API. Trong bản thật:
        response = client.chat.completions.create(
            model="...",
            messages=[...],
            tools=[{"type":"function", "function": {...}}],
        )
    Ở đây ta chỉ log calls để bạn thấy luồng.
    """
    def __init__(self, role: str, name: str):
        self.role = role
        self.name = name
        self.calls = 0

    def __call__(self, prompt: str, state: dict = None) -> str:
        self.calls += 1
        return f"[{self.name} thinking...]"  # logic thật ở Agent


# =============================================================================
# PLAYER AGENT - chọn action dựa trên state
# =============================================================================

class PlayerAgent:
    """
    Trong bản thật: LLM nhận state (engine snapshot) + history,
    function-call trả về {"action":"attack", "target":"Goblin A"}.

    Ở đây: heuristic đơn giản để demo luồng hoạt động.
    """
    def __init__(self, actor: Actor, llm: MockLLM):
        self.actor = actor
        self.llm = llm

    def decide(self, state: GameState) -> dict:
        """Trả về intent: {action, target}. Engine sẽ thực thi."""
        self.llm(f"What do you do, {self.actor.name}?", state.snapshot())
        # Heuristic: đánh quái HP thấp nhất còn sống
        targets = [a for a in state.actors
                   if a.alive and a.team != self.actor.team]
        if not targets:
            return {"action": "wait"}
        target = min(targets, key=lambda a: a.hp)
        return {"action": "attack", "target": target.name}


# =============================================================================
# DM AGENT - narration + scene glue
# =============================================================================

class DMAgent:
    """
    Trong bản thật: LLM nhận world state + history + LMoP lore,
    tường thuật scene, lồng tiếng NPC, phản ứng linh hoạt.

    Ở đây: format template cơ bản để bạn thấy dữ liệu vào/ra.
    """
    def __init__(self, llm: MockLLM):
        self.llm = llm

    def narrate_scene(self, state: GameState) -> str:
        """Mô tả tình huống cho player thấy trước khi chọn action."""
        snap = state.snapshot()
        self.llm("Describe the battlefield scene", snap)
        # Template giả - bản thật là LLM sinh
        party = [a["name"] + f"({a['hp']}/{a['max_hp']})"
                 for a in snap["actors"] if a["team"] == "party" and a["alive"]]
        foes = [a["name"] + f"({a['hp']}/{a['max_hp']})"
                for a in snap["actors"] if a["team"] == "monsters" and a["alive"]]
        return (f"⚔️  Round {snap['round']}. "
                f"Party: {', '.join(party)}. "
                f"Foes: {', '.join(foes) if foes else 'none'}.")

    def narrate_result(self, result: dict) -> str:
        """Tường thuật kết quả đòn đánh sau khi engine tính xong."""
        if not result.get("ok"):
            return f"({result.get('reason','error')})"
        self.llm("Narrate this combat result", result)
        r = result
        if r.get("crit"):
            return f"💥 {r['attacker']} CRITS {r['target']} for {r['damage']}! ({r['target']} HP={r['target_hp_after']})"
        elif r["hit"]:
            return f"⚔️ {r['attacker']} hits {r['target']} for {r['damage']}. ({r['target']} HP={r['target_hp_after']})"
        else:
            return f"💨 {r['attacker']} misses {r['target']} ({r['desc']})."


# =============================================================================
# GAME DRIVER - loop chính, bế engine và agents
# =============================================================================

def make_party() -> list:
    """4 hero BG3-style: Gale, Astarion, Jennevelle, Karlach.
    Class/Race đúng D&D 5e (Standard Array: 15,14,13,12,10,8)."""
    return [
        # Karlach — Fighter, Tiefling. STR cao, tank.
        Actor("Karlach", "party", hp=30, max_hp=30, ac=16, atk_bonus=5, damage_dice="1d8+3", glyph="K",
              str_score=16, dex_score=12, con_score=14, int_score=8, wis_score=10, cha_score=10,
              char_class="Fighter", race="Tiefling"),
        # Gale — Wizard, Human. INT cao, caster.
        Actor("Gale", "party", hp=22, max_hp=22, ac=13, atk_bonus=6, damage_dice="1d10+4", glyph="G",
              str_score=8, dex_score=14, con_score=12, int_score=16, wis_score=12, cha_score=10,
              char_class="Wizard", race="Human"),
        # Astarion — Rogue, Elf. DEX cao, scout/sneak.
        Actor("Astarion", "party", hp=20, max_hp=20, ac=14, atk_bonus=4, damage_dice="1d6+3", glyph="A",
              str_score=9, dex_score=16, con_score=14, int_score=12, wis_score=10, cha_score=13,
              char_class="Rogue", race="Elf"),
        # Jennevelle — Cleric, Half-Elf. WIS cao, healer.
        Actor("Jennevelle", "party", hp=28, max_hp=28, ac=17, atk_bonus=5, damage_dice="1d8+3", glyph="J",
              str_score=14, dex_score=10, con_score=13, int_score=11, wis_score=16, cha_score=12,
              char_class="Cleric", race="Half-Elf"),
    ]

def make_monsters() -> list:
    """Encounter - tương đương spawn trong dungeon.c.
    Goblin có stealth_bonus +6 (D&D 5e SRD) cho ambush."""
    return [
        Actor("Goblin A", "monsters", hp=7,  max_hp=7,  ac=15, atk_bonus=4, damage_dice="1d6+2", glyph="g",
              str_score=8, dex_score=14, con_score=10, int_score=10, wis_score=8, cha_score=8,
              stealth_bonus=6),
        Actor("Goblin B", "monsters", hp=7,  max_hp=7,  ac=15, atk_bonus=4, damage_dice="1d6+2", glyph="g",
              str_score=8, dex_score=14, con_score=10, int_score=10, wis_score=8, cha_score=8,
              stealth_bonus=6),
        Actor("Goblin C", "monsters", hp=7,  max_hp=7,  ac=15, atk_bonus=4, damage_dice="1d6+2", glyph="g",
              str_score=8, dex_score=14, con_score=10, int_score=10, wis_score=8, cha_score=8,
              stealth_bonus=6),
        Actor("Goblin D", "monsters", hp=7,  max_hp=7,  ac=15, atk_bonus=4, damage_dice="1d6+2", glyph="g",
              str_score=8, dex_score=14, con_score=10, int_score=10, wis_score=8, cha_score=8,
              stealth_bonus=6),
    ]

def make_redbrands(count=3) -> list:
    """Redbrand Ruffian - CR 1/2, AC 14, HP 16. Shortsword."""
    out = []
    for i in range(count):
        out.append(Actor(f"Redbrand {chr(65+i)}", "monsters", hp=16, max_hp=16, ac=14,
                         atk_bonus=4, damage_dice="1d6+2", glyph="R",
                         str_score=11, dex_score=14, con_score=12, int_score=9,
                         wis_score=9, cha_score=11))
    return out

def make_skeletons(count=3) -> list:
    """Skeleton - CR 1/4, AC 13, HP 11. Shortsword + Shortbow."""
    out = []
    for i in range(count):
        out.append(Actor(f"Skeleton {chr(65+i)}", "monsters", hp=11, max_hp=11, ac=13,
                         atk_bonus=4, damage_dice="1d6+2", glyph="s",
                         str_score=10, dex_score=14, con_score=15, int_score=6,
                         wis_score=8, cha_score=5))
    return out

def make_glasstaff() -> list:
    """Iarno 'Glasstaff' Albrek - CR 2 wizard boss.
    AC 11 (14 mage armor), HP 27. Spider Staff + spells."""
    return [Actor("Iarno Glasstaff", "monsters", hp=27, max_hp=27, ac=14,
                  atk_bonus=5, damage_dice="1d6+3", glyph="I",
                  str_score=9, dex_score=13, con_score=10, int_score=16,
                  wis_score=14, cha_score=13)]


def run_combat(seed: int = 42, max_rounds: int = 20):
    """Combat loop - mỗi round mỗi actor đi 1 lần theo initiative."""
    rng = random.Random(seed)
    state = GameState(actors=[], seed=seed)

    # Setup
    party = make_party()
    monsters = make_monsters()
    for a in party + monsters:
        add_init(a, rng)
    state.actors = party + monsters
    state.actors.sort(key=lambda a: a.init, reverse=True)

    # Mock LLM cho mỗi agent
    dm_llm = MockLLM("DM", "DM-LLM")
    dm = DMAgent(dm_llm)

    players = {a.name: PlayerAgent(a, MockLLM("player", a.name)) for a in party}

    print("=" * 70)
    print(f"COMBAT START - seed={seed}  (cùng seed -> cùng kết quả)")
    print("Initiative:", [(a.name, a.init) for a in state.actors])
    print("=" * 70)

    while state.round <= max_rounds and not state.combat_over:
        print(f"\n--- ROUND {state.round} ---")
        for actor in list(state.actors):
            if not actor.alive or state.combat_over:
                continue

            # 1. DM narration
            scene = dm.narrate_scene(state)
            print(scene)

            # 2. Player/monster chọn action
            if actor.team == "party":
                intent = players[actor.name].decide(state)
            else:
                # Monster AI heuristic (giống ai_melee_chaser trong ai.c)
                targets = [a for a in state.actors if a.alive and a.team != actor.team]
                if not targets:
                    continue
                target = min(targets, key=lambda a: a.hp)
                intent = {"action": "attack", "target": target.name}

            # 3. Engine thực thi - LUẬT GIA, không để LLM tự tính
            if intent["action"] == "attack":
                target = next(a for a in state.actors if a.name == intent["target"])
                result = resolve_attack(actor, target, rng)
                state.log.append(dm.narrate_result(result))
                print(f"  {actor.name} -> {result['desc']}")
            else:
                print(f"  {actor.name} waits.")

            # 4. Check end condition
            winner = state.check_combat_over()
            if winner:
                state.combat_over = True
                print(f"\n🏆 {winner.upper()} WINS after round {state.round}!")
                break

        state.round += 1

    # Stats
    print("\n" + "=" * 70)
    print("LLM CALL COUNTS:")
    print(f"  DM: {dm_llm.calls}")
    for name, pa in players.items():
        print(f"  {name}: {pa.llm.calls}")
    print("=" * 70)
    return state


if __name__ == "__main__":
    print("Demo 1: seed=42")
    run_combat(seed=42)
    print("\n\nDemo 2: seed=42 (PHẢI giống Demo 1 - determinism check)")
    run_combat(seed=42)
