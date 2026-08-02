"""
LLM_CLIENT.PY - OpenAI-compatible API wrapper.

Hỗ trợ Claude / GPT / Gemini / bất kỳ API nào tương thích OpenAI chat completions.
Qua function calling để LLM đưa INTENT JSON thay vì free text.

Cách dùng:
    client = LLMClient(model="gpt-4o-mini", api_key=os.environ["OPENAI_API_KEY"])
    # HOẶC dùng Claude qua Anthropic (cần base_url proxy hoặc Anthropic SDK)
    response = client.chat(system="...", messages=[...], tools=[...])

Config qua env var:
    - LLM_API_KEY    : API key (bắt buộc cho LLM mode)
    - LLM_MODEL      : model name (default "gpt-4o-mini")
    - LLM_BASE_URL   : base URL (default OpenAI; đổi cho proxy/local)

 KHÔNG cài thư viện: dùng urllib stdlib (tránh phụ thuộc requests/openai).
 Nếu có requests thì ưu tiên dùng (đáng tin hơn).
"""

import os
import json
import urllib.request
import urllib.error
from typing import Optional


class LLMClient:
    """OpenAI-compatible chat client (stdlib only, no external deps)."""

    def __init__(self,
                 model: str = None,
                 api_key: str = None,
                 base_url: str = None,
                 temperature: float = 0.7,
                 max_tokens: int = 300):
        self.model = model or os.environ.get("LLM_MODEL", "gpt-4o-mini")
        self.api_key = api_key or os.environ.get("LLM_API_KEY") or os.environ.get("OPENAI_API_KEY")
        self.base_url = (base_url or os.environ.get("LLM_BASE_URL")
                         or "https://api.openai.com/v1").rstrip("/")
        self.temperature = temperature
        self.max_tokens = max_tokens
        self.calls = 0

    def is_available(self) -> bool:
        return bool(self.api_key)

    def chat(self, system: str, messages: list,
             tools: list = None, tool_choice: str = None) -> dict:
        """
        Gọi chat completions. Return:
          {"content": str, "tool_calls": [...], "raw": dict}

        messages = [{"role": "user"|"assistant", "content": "..."}]
        tools = [{"type":"function", "function":{"name", "description", "parameters":{...}}}]
        """
        if not self.api_key:
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": "no API key"}

        url = f"{self.base_url}/chat/completions"
        payload = {
            "model": self.model,
            "messages": [{"role": "system", "content": system}] + messages,
            "temperature": self.temperature,
            "max_tokens": self.max_tokens,
        }
        if tools:
            payload["tools"] = tools
            if tool_choice:
                payload["tool_choice"] = tool_choice

        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(url, data=data, method="POST")
        req.add_header("Content-Type", "application/json")
        req.add_header("Authorization", f"Bearer {self.api_key}")

        self.calls += 1
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                raw = json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            err_body = e.read().decode("utf-8", errors="replace")
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": f"HTTP {e.code}: {err_body[:200]}"}
        except Exception as e:
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": str(e)}

        choice = raw.get("choices", [{}])[0].get("message", {})
        content = choice.get("content", "") or ""
        tool_calls = choice.get("tool_calls", []) or []
        return {"content": content, "tool_calls": tool_calls, "raw": raw,
                "error": None}


# =============================================================================
# TOOL SCHEMAS (function calling) - cho LLM đưa INTENT JSON
# =============================================================================

# Tool cho PlayerAgent.decide_combat
TOOL_COMBAT_ACTION = {
    "type": "function",
    "function": {
        "name": "choose_action",
        "description": "Chọn action trong combat. Engine sẽ thực thi.",
        "parameters": {
            "type": "object",
            "properties": {
                "verb": {"type": "string",
                         "enum": ["attack", "cast", "move", "use_item", "wait", "say"]},
                "target": {"type": "string",
                           "description": "Tên mục tiêu (vd 'Goblin A'). Cần cho attack/cast."},
                "args": {"type": "object",
                         "description": "Cho cast: {spell:'fire_bolt'}. Cho use_item: {item:'potion'}."},
                "say": {"type": "string",
                        "description": "Tin nhắn chat với đội (coop). Tùy chọn."},
            },
            "required": ["verb"],
        },
    },
}

# Tool cho loot decision
TOOL_LOOT_DECISION = {
    "type": "function",
    "function": {
        "name": "decide_loot",
        "description": "Quyết định nhận hay bỏ item loot.",
        "parameters": {
            "type": "object",
            "properties": {
                "take": {"type": "boolean"},
                "give_to": {"type": "string",
                            "description": "Tên party member nhận item."},
                "reason": {"type": "string"},
            },
            "required": ["take"],
        },
    },
}

# Tool cho town choice
TOOL_TOWN_CHOICE = {
    "type": "function",
    "function": {
        "name": "choose_town_action",
        "description": "Chọn hành động trong thị trấn.",
        "parameters": {
            "type": "object",
            "properties": {
                "chosen": {"type": "string",
                           "description": "ID lựa chọn (vd 'talk_barthen', 'leave_town')."},
                "reason": {"type": "string"},
            },
            "required": ["chosen"],
        },
    },
}
