"""
PROTOCOL.PY - Action JSON schema + validator.

Mọi action mà player/DM agent đưa ra đều qua đây để validate trước khi engine thực thi.
Đảm bảo LLM không thể "làm bậy" - engine chỉ nhận intent hợp lệ.

Action schema:
{
  "verb": "attack|cast|move|loot|talk|say|use_item|rest|leave|wait",
  "target": "goblin_a|npc_barthen|party_thorin|item_x",  # optional, tùy verb
  "args": {"spell": "fire_bolt", "item": "potion"},       # optional
  "say": "text chat to party (optional)"                   # coop communication
}
"""

import json
from typing import Optional

# Valid verbs theo context
COMBAT_VERBS = {"attack", "cast", "move", "use_item", "wait", "say"}
TOWN_VERBS = {"talk", "shop", "rest", "leave", "say"}
LOOT_VERBS = {"take", "leave", "give_to", "say"}
TRAVEL_VERBS = {"continue", "search", "say"}


def make_action(verb: str, target: str = None, args: dict = None,
                say: str = None) -> dict:
    """Tạo action dict chuẩn."""
    action = {"verb": verb}
    if target:
        action["target"] = target
    if args:
        action["args"] = args
    if say:
        action["say"] = say
    return action


def validate_action(action: dict, valid_verbs: set, state_snapshot: dict) -> dict:
    """
    Validate 1 action. Return {"ok": bool, "action": cleaned_action, "error": str}.

    Engine gọi hàm này TRƯỚC khi thực thi để chặn LLM nonsense.
    """
    if not isinstance(action, dict):
        return {"ok": False, "error": "action phải là JSON object", "action": None}

    verb = action.get("verb", "").lower().strip()
    if verb not in valid_verbs:
        return {
            "ok": False,
            "error": f"verb '{verb}' không hợp lệ. Phải là 1 trong: {sorted(valid_verbs)}",
            "action": None,
        }

    # Clean action (chỉ giữ field hợp lệ)
    cleaned = {"verb": verb}
    if action.get("target"):
        cleaned["target"] = str(action["target"])
    if action.get("args") and isinstance(action["args"], dict):
        cleaned["args"] = action["args"]
    if action.get("say"):
        cleaned["say"] = str(action["say"])[:200]  # limit chat length

    # Verb-specific checks
    if verb == "attack" and not cleaned.get("target"):
        return {"ok": False, "error": "attack cần target", "action": None}

    if verb == "cast" and not cleaned.get("args", {}).get("spell"):
        return {"ok": False, "error": "cast cần args.spell", "action": None}

    if verb == "talk" and not cleaned.get("target"):
        return {"ok": False, "error": "talk cần target (NPC name)", "action": None}

    return {"ok": True, "action": cleaned, "error": None}


def parse_llm_json(raw: str) -> Optional[dict]:
    """
    Parse JSON từ LLM response (có thể kèm text thừa).
    Tìm JSON object đầu tiên trong chuỗi. Return None nếu parse fail.
    """
    if not raw:
        return None
    # Thử parse thẳng
    try:
        return json.loads(raw)
    except json.JSONDecodeError:
        pass
    # Tìm { ... } đầu tiên
    start = raw.find("{")
    end = raw.rfind("}")
    if start >= 0 and end > start:
        try:
            return json.loads(raw[start:end+1])
        except json.JSONDecodeError:
            pass
    return None
