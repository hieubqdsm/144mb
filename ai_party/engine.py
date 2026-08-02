"""
ENGINE.PY - Source of truth (luật gia bất biến).

Đây là phần đóng vai engine C trong project thật. Toàn bộ:
  - dice rolling
  - AC check, damage calc
  - HP, status conditions
  - turn order
... chạy trong file này. LLM KHÔNG bao giờ được tự tính.

LLM chỉ đưa ra INTENT (move/attack/cast), engine thực thi và trả kết quả text.
Đây chính là nguyên lý "engine = luật gia, LLM = diễn viên".

Mock thay cho:
  - engine C (rng.h, d20.h, combat.h, actor.h, turn.h)
  - bridge JSON (state_to_json / apply_action)
  - deterministic replay (cùng seed -> cùng kết quả)
"""

import random
import json
from dataclasses import dataclass, field, asdict
from typing import Optional

# =============================================================================
# DATA MODEL - tương đương Actor struct trong src_console/game/actor.h
# =============================================================================

@dataclass
class Actor:
    name: str
    team: str                      # "party" | "monsters"
    hp: int
    max_hp: int
    ac: int
    atk_bonus: int
    damage_dice: str               # "1d6+2" - engine parse và roll
    glyph: str = "@"               # cho render (chưa dùng trong text mode)
    pos: tuple[int, int] = (0, 0)
    conditions: list = field(default_factory=list)   # ["poison", "stun", ...]
    alive: bool = True
    # Ability scores (D&D 5e) - cho skill check (Perception, Stealth, etc.)
    str_score: int = 10
    dex_score: int = 10
    con_score: int = 10
    int_score: int = 10
    wis_score: int = 10
    cha_score: int = 10
    stealth_bonus: int = 0         # +6 goblin, +0 mặc định
    inventory: list = field(default_factory=list)  # [{name, qty, type}]
    surprised: bool = False        # D&D 5e: surprised = mất turn đầu

    def ability_mod(self, ability: str) -> int:
        """Mod = floor((score-10)/2). ability = 'str'|'dex'|'con'|'int'|'wis'|'cha'."""
        score = {"str": self.str_score, "dex": self.dex_score, "con": self.con_score,
                 "int": self.int_score, "wis": self.wis_score, "cha": self.cha_score}.get(ability, 10)
        return (score - 10) // 2

    def passive_perception(self) -> int:
        """D&D 5e: 10 + WIS mod. Cho phát hiện quái ẩn."""
        return 10 + self.ability_mod("wis")

    def to_dict(self) -> dict:
        """Compact state cho LLM - chỉ những gì player/DM cần thấy."""
        return {
            "name": self.name,
            "hp": self.hp,
            "max_hp": self.max_hp,
            "ac": self.ac,
            "conditions": list(self.conditions),
            "alive": self.alive,
            "team": self.team,
            "inventory": list(self.inventory),
        }


# =============================================================================
# DICE - tương đương d20.h (xorshift64 deterministic trong engine C)
# =============================================================================

def roll_dice(spec: str, rng: random.Random) -> int:
    """
    Parse "1d6+2" / "2d8" / "1d20" và roll.
    Giống d20_roll_damage trong src_console/game/d20.c.
    """
    sign = 1
    mod = 0
    if "+" in spec:
        base, bonus = spec.split("+")
        mod = int(bonus)
    elif "-" in spec.split("d")[-1]:    # "1d6-1"
        parts = spec.split("-")
        base = parts[0]
        mod = -int(parts[1])
    else:
        base = spec

    count, sides = base.split("d")
    total = sum(rng.randint(1, int(sides)) for _ in range(int(count)))
    return total + mod


def d20(rng: random.Random, advantage: bool = False, disadvantage: bool = False) -> int:
    """d20 roll với advantage/disadvantage - giống d20_roll trong d20.c."""
    r1 = rng.randint(1, 20)
    if not (advantage or disadvantage):
        return r1
    r2 = rng.randint(1, 20)
    if advantage:
        return max(r1, r2)
    return min(r1, r2)


# =============================================================================
# COMBAT RESOLUTION - tương đương combat.h
# =============================================================================

def resolve_attack(attacker: Actor, target: Actor, rng: random.Random) -> dict:
    """
    Một đòn tấnmp cơ bản. Trả về dict kết quả để LLM tường thuật.

    Logic: roll d20 + atk_bonus >= target.ac ?
      - nat 20 -> crit (double damage dice)
      - nat 1  -> fumble (auto miss)
      - else so AC

    Giống combat_resolve_attack trong src_console/game/combat.c.
    """
    if not attacker.alive or not target.alive:
        return {"ok": False, "reason": "actor_dead"}

    nat = d20(rng)
    total = nat + attacker.atk_bonus

    # Crit / fumble
    if nat == 20:
        dmg = roll_dice(attacker.damage_dice, rng) * 2
        target.hp -= dmg
        crit = True
        hit = True
        desc = "CRIT!"
    elif nat == 1:
        dmg = 0
        crit = False
        hit = False
        desc = "fumble (nat 1)"
    else:
        hit = total >= target.ac
        crit = False
        if hit:
            dmg = roll_dice(attacker.damage_dice, rng)
            target.hp -= dmg
            desc = f"hit AC {target.ac} (rolled {total})"
        else:
            dmg = 0
            desc = f"miss AC {target.ac} (rolled {total})"

    if target.hp <= 0:
        target.hp = 0
        target.alive = False

    return {
        "ok": True,
        "attacker": attacker.name,
        "target": target.name,
        "roll": nat,
        "total": total,
        "hit": hit,
        "crit": crit,
        "damage": dmg,
        "target_hp_after": target.hp,
        "target_alive": target.alive,
        "desc": desc,
    }


# =============================================================================
# STATE + REPLAY - đây chính là "bridge JSON" của engine C
# =============================================================================

@dataclass
class GameState:
    """Toàn bộ state cần thiết để replay - tương đương save.h."""
    actors: list
    round: int = 1
    turn_index: int = 0            # ai đang đi trong round
    log: list = field(default_factory=list)
    seed: int = 0
    combat_over: bool = False

    def snapshot(self) -> dict:
        """Dump state -> dict. Trong engine C đây là state_to_json()."""
        return {
            "round": self.round,
            "turn_index": self.turn_index,
            "combat_over": self.combat_over,
            "actors": [a.to_dict() for a in self.actors],
            "recent_log": self.log[-6:],
        }

    def whose_turn(self) -> Actor:
        alive = [a for a in self.actors if a.alive]
        alive.sort(key=lambda a: a.init)  # type: ignore
        return alive[self.turn_index % len(alive)]

    def check_combat_over(self) -> Optional[str]:
        """Trả về team thắng hoặc None."""
        party_alive = any(a.alive for a in self.actors if a.team == "party")
        monsters_alive = any(a.alive for a in self.actors if a.team == "monsters")
        if not party_alive:
            return "monsters"
        if not monsters_alive:
            return "party"
        return None


# Patch: thêm init field để resolve turn order (đơn giản hóa demo)
def add_init(actor: Actor, rng: random.Random):
    actor.init = d20(rng) + actor.ability_mod("dex")


# =============================================================================
# SURPRISE MECHANIC - D&D 5e: Stealth vs passive Perception
# =============================================================================

def roll_surprise(party: list, monsters: list, rng: random.Random,
                  monsters_stealth_bonus: int = 6) -> dict:
    """
    Monsters roll Stealth (d20 + stealth_bonus). Party passive Perception = 10 + WIS mod.
    D&D 5e: ai dưới DC Stealth = surprised (mất turn đầu round 1).

    Return: {party_surprised: [names], monsters_surprised: [names], rolls: [...]}
    """
    # Monsters Stealth - lấy roll cao nhất (đại diện best spotter của nhóm)
    monster_stealth = d20(rng) + monsters_stealth_bonus
    # Party passive Perception - best spotter
    party_pp = max(a.passive_perception() for a in party)

    party_surprised = []
    if monster_stealth > party_pp:
        # Toàn bộ party bị surprised (không ai phát hiện)
        party_surprised = [a.name for a in party]

    # Monsters luôn surprise vì party lộ trên đường (không ẩn)
    # Trong D&D 5e thật cả 2 phía roll, nhưng LMoP ambush monsters chủ động

    return {
        "party_surprised": party_surprised,
        "monsters_surprised": [],   # monsters không bị surprise trong LMoP ambush
        "monster_stealth_roll": monster_stealth,
        "party_passive_perception": party_pp,
        "spotted": len(party_surprised) == 0,
    }


# =============================================================================
# LOOT TABLE + REWARD POOL - CR-based treasure
# =============================================================================

# Loot table theo monster type (LMoP thật)
LOOT_TABLES = {
    "goblin": [
        {"type": "gold", "dice": "1d6", "min": 0, "chance": 0.5},     # pouch CP/GP
        {"type": "item", "name": "Healing Potion", "chance": 0.1},
        {"type": "item", "name": "Shortbow", "chance": 0.05},
    ],
    "bugbear": [
        {"type": "gold", "dice": "2d6+5", "min": 5, "chance": 0.9},
        {"type": "item", "name": "Potion of Healing", "chance": 0.3},
        {"type": "item", "name": "Javelin x3", "chance": 0.5},
    ],
    "goblin_camp": [   # Klarg's cave
        {"type": "gold", "dice": "5d6", "min": 10, "chance": 1.0},
        {"type": "item", "name": "Potion of Healing x2", "chance": 1.0},
        {"type": "item", "name": "Lionshield Supplies", "chance": 1.0},
    ],
}


def gen_loot(monster_type: str, count: int, rng: random.Random) -> dict:
    """Roll treasure cho N quái cùng loại. Return {gold, items: [...]}."""
    table = LOOT_TABLES.get(monster_type, [])
    total_gold = 0
    items = []
    for _ in range(count):
        for entry in table:
            if rng.random() < entry["chance"]:
                if entry["type"] == "gold":
                    total_gold += roll_dice(entry["dice"], rng)
                elif entry["type"] == "item":
                    items.append({"name": entry["name"], "qty": 1})
    return {"gold": max(total_gold, 0), "items": items}


def offer_loot_to_party(party: list, loot: dict, ai_decider) -> dict:
    """
    Mỗi item: AI chọn take/leave/give_to. ai_decider(item, party) -> decision dict.
    Return: {distributed: [{actor, item}], gold_to: name, left: [...]}
    """
    distributed = []
    remaining_items = []
    for item in loot.get("items", []):
        decision = ai_decider(item, party)
        if decision.get("take"):
            target_name = decision.get("give_to") or party[0].name
            target = next((a for a in party if a.name == target_name), party[0])
            target.inventory.append({"name": item["name"], "qty": item.get("qty", 1)})
            distributed.append({"actor": target.name, "item": item["name"]})
        else:
            remaining_items.append(item)

    # Gold chia đều
    gold_per = loot["gold"] // len(party) if party else 0
    gold_remainder = loot["gold"] - gold_per * len(party)
    for a in party:
        a.inventory.append({"name": "Gold", "qty": gold_per})
    if party:
        party[0].inventory[-1]["qty"] += gold_remainder

    return {
        "distributed": distributed,
        "gold_each": gold_per,
        "total_gold": loot["gold"],
        "left_items": remaining_items,
    }


# =============================================================================
# CHOICE NODE - branch flow graph
# =============================================================================

def resolve_choice(options: list, ai_decider) -> str:
    """
    Options = [{id, label, requires: {...}}]. AI chọn 1. Return chosen id.
    ai_decider(options, context) -> {chosen: id, reason: str}
    """
    decision = ai_decider(options)
    chosen_id = decision.get("chosen")
    # Validate
    valid_ids = [o["id"] for o in options]
    if chosen_id not in valid_ids:
        chosen_id = options[0]["id"]   # fallback default
    return chosen_id
