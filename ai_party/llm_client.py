"""
LLM_CLIENT.PY - OpenAI-compatible API wrapper.

Hỗ trợ Claude / GPT / Gemini / bất kỳ API nào tương thích OpenAI chat completions.
Qua function calling để LLM đưa INTENT JSON thay vì free text.

Cách dùng:
    1. Tạo file ai_party/.env (copy từ .env.example, điền key thật)
    2. python flow_sim.py --llm
    Code tự load .env (không cần cài python-dotenv).

Config qua .env (tự load) HOẶC env var:
    - LLM_API_KEY    : API key (bắt buộc cho LLM mode)
    - LLM_MODEL      : model name (default "gpt-4o-mini")
    - LLM_BASE_URL   : base URL (default OpenAI; đổi cho proxy/local/Ollama)
"""

import os
import json
import urllib.request
import urllib.error
from typing import Optional


# =============================================================================
# .ENV LOADER - tự parse, không cần python-dotenv package
# =============================================================================

def _load_dotenv(path: str = None):
    """Load file .env (KEY=value per line) vào os.environ.
    Không ghi đè env var đã có sẵn (env var thật ưu tiên hơn file)."""
    if path is None:
        # Tìm .env cùng thư mục file này
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".env")
    if not os.path.exists(path):
        return
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            # Skip comment / empty
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                continue
            key, _, val = line.partition("=")
            key = key.strip()
            val = val.strip().strip('"').strip("'")  # bỏ quote nếu có
            if key and key not in os.environ:   # KHÔNG ghi đè env thật
                os.environ[key] = val

# Load ngay khi import
_load_dotenv()


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

        Tự detect format dựa base_url:
          - .../anthropic → Anthropic /v1/messages + x-api-key
          - .../v4 or /v1 → OpenAI /chat/completions + Bearer
        """
        if not self.api_key:
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": "no API key"}

        # Detect format từ base_url
        is_anthropic = "anthropic" in self.base_url

        if is_anthropic:
            return self._chat_anthropic(system, messages, tools, tool_choice)
        return self._chat_openai(system, messages, tools, tool_choice)

    def _chat_anthropic(self, system, messages, tools, tool_choice):
        """Gọi theo Anthropic Messages API (/v1/messages)."""
        url = f"{self.base_url}/v1/messages"
        # Anthropic: system riêng, messages chỉ user/assistant
        payload = {
            "model": self.model,
            "max_tokens": self.max_tokens,
            "system": system,
            "messages": messages,
        }
        # Anthropic tools format khác OpenAI: {name, description, input_schema}
        if tools:
            anth_tools = []
            for t in tools:
                fn = t.get("function", t)
                anth_tools.append({
                    "name": fn["name"],
                    "description": fn.get("description", ""),
                    "input_schema": fn.get("parameters", {"type": "object", "properties": {}}),
                })
            payload["tools"] = anth_tools
            if tool_choice and "function" in tool_choice:
                payload["tool_choice"] = {"type": "tool", "name": tool_choice["function"]["name"]}
            elif tool_choice:
                payload["tool_choice"] = {"type": "any"}

        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(url, data=data, method="POST")
        req.add_header("Content-Type", "application/json")
        req.add_header("x-api-key", self.api_key)
        req.add_header("anthropic-version", "2023-06-01")

        self.calls += 1
        try:
            with urllib.request.urlopen(req, timeout=20) as resp:
                raw = json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            err_body = e.read().decode("utf-8", errors="replace")
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": f"HTTP {e.code}: {err_body[:200]}"}
        except Exception as e:
            return {"content": "", "tool_calls": [], "raw": {},
                    "error": str(e)}

        # Parse Anthropic response: content[] có thể có type=text hoặc type=tool_use
        content_text = ""
        tool_calls = []
        for block in raw.get("content", []):
            if block.get("type") == "text":
                content_text += block.get("text", "")
            elif block.get("type") == "tool_use":
                tool_calls.append({
                    "id": block.get("id", ""),
                    "type": "function",
                    "function": {
                        "name": block.get("name", ""),
                        "arguments": json.dumps(block.get("input", {}), ensure_ascii=False),
                    }
                })
        if tools and not tool_calls:
            import sys
            print(f"    [DEBUG-ANTHROPIC] No tool_use. Content: {content_text[:200]}", file=sys.stderr)
        return {"content": content_text, "tool_calls": tool_calls, "raw": raw,
                "error": None}

    def _chat_openai(self, system, messages, tools, tool_choice):
        """Gọi theo OpenAI Chat Completions (/chat/completions)."""
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
            with urllib.request.urlopen(req, timeout=15) as resp:
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
        if tools and not tool_calls:
            import sys
            print(f"    [DEBUG-OPENAI] No tool_calls. Raw message: {json.dumps(choice, ensure_ascii=False)[:300]}", file=sys.stderr)
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
