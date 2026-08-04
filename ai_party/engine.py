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
    reaction_used: bool = False    # D&D 5e: 1 reaction/round (cho opp attack)
    # Death save (D&D 5e: HP=0 = dying, chưa chết ngay)
    dying: bool = False            # True = HP 0, đang dying
    death_saves_success: int = 0   # 3 = stable
    death_saves_fail: int = 0      # 3 = dead

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
    if not attacker.alive or (not target.alive and not getattr(target, 'dying', False)):
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
        # D&D 5e: HP=0 = dying (chưa chết), có thể cứu bằng death save / heal
        if not target.dying and target.team == "party":
            target.dying = True   # party member → dying (có death save)
        else:
            target.alive = False  # monster hoặc damage sau khi dying → dead

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
        """Trả về team thắng hoặc None. Dying actors vẫn được tính (có thể cứu)."""
        # Party: alive OR dying (chưa chết hẳn, có thể cứu)
        party_up = any(a.alive or getattr(a, 'dying', False) for a in self.actors if a.team == "party")
        monsters_up = any(a.alive or getattr(a, 'dying', False) for a in self.actors if a.team == "monsters")
        if not party_up:
            return "monsters"
        if not monsters_up:
            return "party"
        return None


def roll_death_save(actor: Actor, rng: random.Random) -> dict:
    """
    D&D 5e death save. Gọi đầu turn khi actor đang dying.
    d20: 10+ = success, <10 = fail. nat20 = 2 success, nat1 = 2 fail.
    3 success = stable, 3 fail = dead.
    """
    if not actor.dying:
        return {"ok": False, "reason": "not dying"}
    roll = d20(rng)
    if roll == 20:
        actor.death_saves_success += 2
        desc = "nat20! 2 success"
        # nat20 = revive 1 HP
        actor.hp = 1
        actor.dying = False
        actor.death_saves_success = 0
        actor.death_saves_fail = 0
        return {"ok": True, "roll": roll, "result": "revived", "desc": desc + " — HỒI SINH 1HP!"}
    elif roll == 1:
        actor.death_saves_fail += 2
        desc = "nat1! 2 fail"
    elif roll >= 10:
        actor.death_saves_success += 1
        desc = f"success ({roll})"
    else:
        actor.death_saves_fail += 1
        desc = f"fail ({roll})"

    if actor.death_saves_success >= 3:
        actor.dying = False
        actor.death_saves_success = 0
        actor.death_saves_fail = 0
        return {"ok": True, "roll": roll, "result": "stable", "desc": desc + " — ỔN ĐỊNH (3 success)"}
    if actor.death_saves_fail >= 3:
        actor.alive = False
        actor.dying = False
        return {"ok": True, "roll": roll, "result": "dead", "desc": desc + " — CHẾT (3 fail)"}
    return {"ok": True, "roll": roll, "result": "ongoing",
            "desc": f"{desc} (success:{actor.death_saves_success}/fail:{actor.death_saves_fail})"}


# Patch: thêm init field để resolve turn order (đơn giản hóa demo)
def add_init(actor: Actor, rng: random.Random):
    actor.init = d20(rng) + actor.ability_mod("dex")


# =============================================================================
# SURPRISE MECHANIC - D&D 5e: Stealth vs passive Perception
# =============================================================================

def roll_surprise(party: list, monsters: list, rng: random.Random,
                  monsters_stealth_bonus: int = 6,
                  party_stealth_bonus: int = 0) -> dict:
    """
    D&D 5e ambush: cả 2 phía có thể surprised.
    - Monsters Stealth (d20 + stealth_bonus) vs Party passive Perception (10 + WIS).
    - Party Stealth (d20 + bonus) vs Monsters passive Perception.
    Ai thua = surprised (mất turn đầu round 1).

    Return: {party_surprised: [names], monsters_surprised: [names], spotted: bool}
    """
    # Monsters Stealth vs Party Perception
    monster_stealth = d20(rng) + monsters_stealth_bonus
    party_pp = max(a.passive_perception() for a in party)

    party_surprised = []
    if monster_stealth > party_pp:
        party_surprised = [a.name for a in party]

    # Party Stealth vs Monsters Perception (cho dungeon exploration)
    party_stealth = d20(rng) + party_stealth_bonus
    monster_pp = max(10 + a.ability_mod("wis") for a in monsters) if monsters else 10

    monsters_surprised = []
    if party_stealth > monster_pp:
        monsters_surprised = [a.name for a in monsters]

    return {
        "party_surprised": party_surprised,
        "monsters_surprised": monsters_surprised,
        "monster_stealth_roll": monster_stealth,
        "party_passive_perception": party_pp,
        "party_stealth_roll": party_stealth,
        "monster_passive_perception": monster_pp,
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
    "redbrand": [
        {"type": "gold", "dice": "2d6", "min": 2, "chance": 0.7},
        {"type": "item", "name": "Healing Potion", "chance": 0.15},
    ],
    "glasstaff": [   # Iarno's chest
        {"type": "gold", "dice": "10d6+50", "min": 50, "chance": 1.0},
        {"type": "item", "name": "Spider Staff", "chance": 1.0},
        {"type": "item", "name": "Scroll of Charm Person", "chance": 1.0},
        {"type": "item", "name": "Scroll of Fireball", "chance": 1.0},
    ],
}


def gen_loot(monster_type: str, count: int, rng: random.Random) -> dict:
    """Roll treasure cho encounter (không phải per-monster).
    Mỗi loại item chỉ drop 1 lần (stack qty), gold scale theo count.
    Return {gold, items: [...]}."""
    table = LOOT_TABLES.get(monster_type, [])
    total_gold = 0
    items = []
    seen = set()   # track item names đã drop (tránh duplicate)
    # Roll gold: 1 lần cho cả encounter (scale theo count)
    for entry in table:
        if rng.random() < entry["chance"]:
            if entry["type"] == "gold":
                total_gold += roll_dice(entry["dice"], rng)
            elif entry["type"] == "item" and entry["name"] not in seen:
                seen.add(entry["name"])
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


# =============================================================================
# DISTANCE HELPERS - D&D 5e grid (5ft = 1 square)
# =============================================================================

def chebyshev(a, b) -> int:
    """Khoảng cách melee (max(dx,dy)). Dùng cho adjacency check.
    Chebyshev ≤ 1 = adjacent (trong 5ft)."""
    return max(abs(a.pos[0]-b.pos[0]), abs(a.pos[1]-b.pos[1]))

def manhattan(a, b) -> int:
    """Khoảng cách di chuyển (|dx|+|dy|). Dùng cho movement calc."""
    return abs(a.pos[0]-b.pos[0]) + abs(a.pos[1]-b.pos[1])

def distance_ft(a, b) -> int:
    """Khoảng cách feet (mỗi cell = 5ft). Dùng cho range check."""
    return chebyshev(a, b) * 5

def in_melee_range(a, b) -> bool:
    """Có trong tầm melee (5ft) không."""
    return chebyshev(a, b) <= 1

def in_range(a, b, range_ft) -> bool:
    """Có trong tầm bắn không. range_ft = 0 = melee only."""
    if range_ft <= 0:
        return in_melee_range(a, b)
    return distance_ft(a, b) <= range_ft

def threatens(attacker, target) -> bool:
    """Attacker có đe dọa target (opportunity attack range) không?
    D&D 5e: melee range + không incapacitated."""
    if not attacker.alive or not target.alive:
        return False
    if attacker.team == target.team:
        return False
    return in_melee_range(attacker, target)


# =============================================================================
# SPELL SYSTEM - port từ src_console/data/spells.c + spell_resolve.c
# =============================================================================

@dataclass
class SpellDef:
    """Tương đương SpellDef trong spells.h."""
    name: str
    level: int              # 0 = cantrip
    kind: str               # atk_ranged|magic_missile|save_half|buff_ac|heal|poison|stun
    damage: str             # dice spec "1d10"
    save: str = None        # "dex"|"con"|"wis"|None
    ac_bonus: int = 0       # for buff_ac
    aoe: bool = False       # True = area effect (nhiều target)
    aoe_ft: int = 0         # radius feet (cho AoE)
    description: str = ""   # cho LLM biết hiệu ứng

# Bang spells (port từ C + thêm Burning Hands AoE)
SPELLS = {
    "fire_bolt": SpellDef("Fire Bolt", 0, "atk_ranged", "1d10",
                          description="Tia lửa ranged, roll vs AC."),
    "magic_missile": SpellDef("Magic Missile", 1, "magic_missile", "3d4+3",
                              description="3 mũi tên force, auto-hit."),
    "fireball": SpellDef("Fireball", 3, "save_half", "8d6", save="dex",
                         aoe=True, aoe_ft=20,
                         description="Cầu lửa AoE 20ft, save DEX half."),
    "mage_armor": SpellDef("Mage Armor", 1, "buff_ac", "0d1", ac_bonus=3,
                           description="Buff +3 AC lên caster."),
    "cure_wounds": SpellDef("Cure Wounds", 1, "heal", "1d8+3",
                            description="Hồi máu 1d8+3 cho target."),
    "poison_spray": SpellDef("Poison Spray", 0, "poison", "1d12", save="con",
                             description="Save CON hoặc poison DOT."),
    "hold_person": SpellDef("Hold Person", 2, "stun", "0d1", save="wis",
                            description="Save WIS hoặc stunned (mất turn)."),
    # MỚI - AoE spells mà LLM hay muốn dùng
    "burning_hands": SpellDef("Burning Hands", 1, "save_half", "3d6", save="dex",
                              aoe=True, aoe_ft=15,
                              description="Cone lửa AoE 15ft, save DEX half."),
    "sacred_flame": SpellDef("Sacred Flame", 0, "save_half", "1d8", save="dex",
                             description="Ánh sáng thần thánh, save DEX half."),
    "healing_word": SpellDef("Healing Word", 1, "heal", "1d4+3",
                             description="Hồi máu 1d4+3, bonus action."),
}

def _apply_hp_zero(actor):
    """Helper: khi HP <= 0, set dying (party) hoặc dead (monster)."""
    actor.hp = 0
    if not getattr(actor, 'dying', False) and actor.team == "party":
        actor.dying = True
    else:
        actor.alive = False


def list_spells_for_class(char_class: str) -> list:
    """Trả list spell ID phù hợp class (cho LLM biết cast được gì)."""
    spell_lists = {
        "Fighter": ["fire_bolt"],  # Fighter không có phép (Eldritch Knight sau)
        "Wizard": ["fire_bolt", "magic_missile", "burning_hands", "mage_armor"],
        "Cleric": ["sacred_flame", "cure_wounds", "healing_word", "hold_person"],
        "Rogue": [],  # Rogue (Arcane Trickster sau)
    }
    return spell_lists.get(char_class, [])


def cast_spell(spell_id: str, caster: Actor, target: Actor,
               all_actors: list, rng: random.Random, dc: int = 13) -> dict:
    """
    Cast spell. Port từ spell_resolve.c.
    Hỗ trợ AoE: nếu spell.aoe, loop tất cả actors trong radius.
    Return dict result cho LLM narrate.
    """
    s = SPELLS.get(spell_id)
    if not s:
        return {"ok": False, "error": f"spell '{spell_id}' không tồn tại"}

    results = []  # cho AoE (nhiều target)

    def apply_single(tgt):
        """Apply spell lên 1 target. Return result dict."""
        if s.kind == "atk_ranged":
            # Roll vs AC (giống SP_ATK_RANGED trong C)
            nat = d20(rng)
            total = nat + 5  # mod_spell hardcoded 5 (giống C)
            if nat == 20:  # crit
                dmg = roll_dice(s.damage, rng) * 2
                tgt.hp -= dmg
                if tgt.hp <= 0: _apply_hp_zero(tgt)
                return {"target": tgt.name, "hit": True, "crit": True, "damage": dmg,
                        "hp_after": tgt.hp, "desc": "CRIT"}
            elif total >= tgt.ac:
                dmg = roll_dice(s.damage, rng)
                tgt.hp -= dmg
                if tgt.hp <= 0: _apply_hp_zero(tgt)
                return {"target": tgt.name, "hit": True, "crit": False, "damage": dmg,
                        "hp_after": tgt.hp, "desc": "hit"}
            else:
                return {"target": tgt.name, "hit": False, "damage": 0,
                        "hp_after": tgt.hp, "desc": f"miss (AC {tgt.ac})"}

        elif s.kind == "magic_missile":
            # Auto-hit, no save
            dmg = roll_dice(s.damage, rng)
            tgt.hp -= dmg
            if tgt.hp <= 0: _apply_hp_zero(tgt)
            return {"target": tgt.name, "hit": True, "damage": dmg,
                    "hp_after": tgt.hp, "desc": "auto-hit"}

        elif s.kind == "save_half":
            # Save for half (or full if fail)
            dmg = roll_dice(s.damage, rng)
            if s.save:
                score = getattr(tgt, s.save + "_score", 10)
                mod = (score - 10) // 2
                save_roll = d20(rng) + mod
                if save_roll >= dc:
                    dmg = dmg // 2  # half
                    desc = f"save ({s.save} {save_roll}≥{dc}), half"
                else:
                    desc = f"fail ({s.save} {save_roll}<{dc}), full"
            else:
                desc = "no save"
            tgt.hp -= dmg
            if tgt.hp <= 0: _apply_hp_zero(tgt)
            return {"target": tgt.name, "hit": True, "damage": dmg,
                    "hp_after": tgt.hp, "desc": desc}

        elif s.kind == "buff_ac":
            # Buff caster (giống C: Effect ac_bonus)
            caster.ac += s.ac_bonus
            return {"target": caster.name, "hit": True, "damage": 0,
                    "ac_bonus": s.ac_bonus, "desc": f"+{s.ac_bonus} AC"}

        elif s.kind == "heal":
            heal = roll_dice(s.damage, rng)
            tgt.hp = min(tgt.hp + heal, tgt.max_hp)
            if getattr(tgt, 'dying', False) and tgt.hp > 0:
                tgt.dying = False
                tgt.death_saves_success = 0
                tgt.death_saves_fail = 0
            return {"target": tgt.name, "hit": True, "heal": heal,
                    "hp_after": tgt.hp, "desc": f"+{heal} HP"}

        elif s.kind == "poison":
            # Save CON hoặc poison
            dmg = roll_dice(s.damage, rng)
            if s.save:
                score = getattr(tgt, s.save + "_score", 10)
                mod = (score - 10) // 2
                save_roll = d20(rng) + mod
                if save_roll >= dc:
                    return {"target": tgt.name, "hit": False, "damage": 0,
                            "desc": f"save ({s.save}), no poison"}
            tgt.hp -= dmg
            if tgt.hp <= 0: _apply_hp_zero(tgt)
            # Thêm condition poison (simplified - giảm HP mỗi turn)
            if "poisoned" not in tgt.conditions:
                tgt.conditions.append("poisoned")
            return {"target": tgt.name, "hit": True, "damage": dmg,
                    "hp_after": tgt.hp, "desc": "POISONED + DOT"}

        elif s.kind == "stun":
            # Save WIS hoặc stunned
            if s.save:
                score = getattr(tgt, s.save + "_score", 10)
                mod = (score - 10) // 2
                save_roll = d20(rng) + mod
                if save_roll >= dc:
                    return {"target": tgt.name, "hit": False, "damage": 0,
                            "desc": f"save ({s.save}), không stunned"}
            if "stunned" not in tgt.conditions:
                tgt.conditions.append("stunned")
            return {"target": tgt.name, "hit": True, "damage": 0,
                    "desc": "STUNNED (mất turn)"}

        return {"target": tgt.name, "hit": False, "desc": "unknown kind"}

    # Execute: single hoặc AoE
    if s.aoe:
        # AoE: tìm tất cả target trong radius từ target center
        targets_hit = []
        for a in all_actors:
            if not a.alive or a.team == caster.team:
                continue
            if distance_ft(target, a) <= s.aoe_ft:
                r = apply_single(a)
                targets_hit.append(r)
        return {"ok": True, "spell": s.name, "aoe": True, "aoe_ft": s.aoe_ft,
                "results": targets_hit}
    else:
        r = apply_single(target)
        return {"ok": True, "spell": s.name, "aoe": False, "result": r}


# =============================================================================
# MOVEMENT + OPPORTUNITY ATTACK - mới (C chưa có)
# =============================================================================

def resolve_move(actor: Actor, dx: int, dy: int, all_actors: list,
                 rng: random.Random) -> dict:
    """
    Di chuyển 1 cell. Return dict result.
    Nếu rời khỏi threatened square → trigger opportunity attack từ enemy adjacent.
    """
    if dx == 0 and dy == 0:
        return {"ok": False, "reason": "không di chuyển"}

    old_pos = actor.pos
    new_pos = (actor.pos[0] + dx, actor.pos[1] + dy)

    # Check collision (có actor khác ở new_pos không)
    for a in all_actors:
        if a is not actor and a.alive and a.pos == new_pos:
            return {"ok": False, "reason": f"blocked by {a.name}"}

    # Check opportunity attack: ai đang threaten vị trí cũ?
    opp_attackers = []
    for a in all_actors:
        if a is actor or not a.alive or a.team == actor.team:
            continue
        if threatens(a, actor) and not getattr(a, "reaction_used", False):
            opp_attackers.append(a)

    # Di chuyển
    actor.pos = new_pos

    # Trigger opportunity attacks
    opp_results = []
    for attacker in opp_attackers:
        r = resolve_attack(attacker, actor, rng)
        attacker.reaction_used = True  # dùng reaction
        opp_results.append({"attacker": attacker.name, "result": r})

    return {
        "ok": True,
        "from": old_pos,
        "to": new_pos,
        "opportunity_attacks": opp_results,
    }

def step_toward(actor: Actor, target: Actor, all_actors: list,
                rng: random.Random) -> dict:
    """
    Move 1 cell về phía target (giống ai_melee_chaser trong C).
    Tự chọn dx/dy tối ưu. Return resolve_move result.
    """
    dx = target.pos[0] - actor.pos[0]
    dy = target.pos[1] - actor.pos[1]
    # Chọn axis lớn hơn trước (giống C ai.c)
    step_x = 1 if dx > 0 else (-1 if dx < 0 else 0)
    step_y = 1 if dy > 0 else (-1 if dy < 0 else 0)
    if abs(dx) >= abs(dy):
        return resolve_move(actor, step_x, 0, all_actors, rng)
    else:
        return resolve_move(actor, 0, step_y, all_actors, rng)
