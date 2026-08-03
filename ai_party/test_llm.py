"""Test GLM API raw response - xem format thật để sửa parser."""
from llm_client import LLMClient

c = LLMClient()
print(f"Model: {c.model}")
print(f"Base URL: {c.base_url}")
print(f"Key: {c.api_key[:8]}..." if c.api_key else "Key: (none)")
print()

# Test 1: chat đơn giản (không tools)
print("=" * 60)
print("TEST 1: Chat đơn giản (no tools)")
print("=" * 60)
r = c.chat("Bạn là DM D&D 5e.", [{"role": "user", "content": "Nói 'xin chào' bằng tiếng Việt, 1 câu ngắn."}])
print(f"Error: {r.get('error')}")
print(f"Content: {r.get('content', '')[:200]}")
print()

if not r.get("error"):
    # Test 2: có tools (function calling)
    print("=" * 60)
    print("TEST 2: Chat với tools (function calling)")
    print("=" * 60)
    tool = {
        "type": "function",
        "function": {
            "name": "choose_action",
            "description": "Chọn hành động combat. Phải gọi function này.",
            "parameters": {
                "type": "object",
                "properties": {
                    "verb": {"type": "string", "enum": ["attack", "wait", "say"]},
                    "target": {"type": "string"},
                    "say": {"type": "string"},
                },
                "required": ["verb"],
            },
        },
    }
    r2 = c.chat(
        "Bạn là Thorin, Fighter dũng cảm. PHẢI dùng function choose_action.",
        [{"role": "user", "content": "Lượt của bạn. 1 con Goblin A (HP 7). Chọn action."}],
        tools=[tool],
    )
    print(f"Error: {r2.get('error')}")
    print(f"Content: {r2.get('content', '')[:300]}")
    print(f"Tool calls: {r2.get('tool_calls', [])}")
    print()
    print("--- RAW RESPONSE (full) ---")
    import json
    print(json.dumps(r2.get("raw", {}), ensure_ascii=False, indent=2)[:1000])
