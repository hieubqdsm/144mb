"""
CAMPAIGN.PY - LMoP flow graph (intro + town node).

Mỗi SceneNode = 1 node trong đồ thị campaign. Flow simulator duyệt theo:
  INTRO → TRAVEL (perception check) → AMBUSH (surprise) → COMBAT → LOOT → TOWN

Type scene:
  - INTRO    : DM narration, auto-next
  - TRAVEL   : DM narrate + ẩn perception check (roll passive vs stealth)
  - COMBAT   : initiative + round loop (engine adjudicate)
  - LOOT     : gen treasure + offer party (take/leave)
  - TOWN     : NPC list + choices (talk/shop/rest/leave)
"""

from dataclasses import dataclass, field
from typing import Optional


@dataclass
class SceneNode:
    node_id: str
    scene_type: str            # INTRO/TRAVEL/COMBAT/LOOT/TOWN/END
    title: str
    narration: str             # DM đọc khi vào node
    next: str = ""             # node kế tiếp (default path)

    # TRAVEL-specific: perception check
    perception_dc: int = 0     # 0 = không check
    on_spotted: str = ""       # node nếu phát hiện
    on_surprised: str = ""     # node nếu bị surprise
    stealth_bonus: int = 6     # monster stealth (goblin +6)

    # COMBAT-specific
    monster_type: str = ""     # "goblin" / "bugbear" / ...
    monster_count: int = 0

    # LOOT-specific
    loot_source: str = ""      # monster_type để gen_loot

    # TOWN-specific
    npcs: list = field(default_factory=list)     # ["Barthen", "Stonehill", ...]
    choices: list = field(default_factory=list)  # [{id, label}]


# =============================================================================
# LMoP INTRO + TOWN FLOW GRAPH
# =============================================================================

NODES = {
    # === CHAPTER 1: GOBLIN ARROWS ===
    "neverwinter_inn": SceneNode(
        node_id="neverwinter_inn",
        scene_type="INTRO",
        title="Phần mở đầu — Quán trọ Neverwinter",
        narration=(
            "Thành phố Neverwinter, bên bờ biển sóng vỗ. "
            "Tại quán trọ, một người lùn râu đỏ ngồi đợi các ngươi. "
            "Hắn tên Gundren Rockseeker — một thợ mỏ thăm dò kỳ cựu.\n\n"
            "Gundren: 'Ta tìm thấy rồi! Một lối vào mỏ cổ... Wave Echo Cave! "
            "Các ngươi hãy hộ tống xe bò chở đồ tiếp tế tới Phandalin. "
            "Ta sẽ đi trước ngựa. Phần thưởng: 10 vàng mỗi người.'\n\n"
            "Đội các ngươi gật đầu đồng ý. Cuộc phiêu lưu bắt đầu."
        ),
        next="triboar_travel",
    ),

    "triboar_travel": SceneNode(
        node_id="triboar_travel",
        scene_type="TRAVEL",
        title="Ba ngày trên Triboar Trail",
        narration=(
            "Ba ngày đi trên con đường đất Triboar Trail. "
            "Buổi sáng thứ tư, rừng im lặng một cách bất thường. "
            "Không tiếng chim, không gió lay cành. Chỉ có bánh xe bò kẽo kẹt.\n\n"
            "Bỗng nhiên — các ngươi thấy hai con ngựa chết chắn ngang đường. "
            "Chúng mang yên ghế của Neverwinter. Đây là ngựa của Gundren!\n\n"
            "Cảnh tượng quá đáng ngờ. Rừng nín thở. "
            "(DM đang âm thầm thả d20 check Perception của cả team...)"
        ),
        perception_dc=12,
        stealth_bonus=6,          # goblin Stealth +6
        on_spotted="combat_spotted",      # party phát hiện
        on_surprised="combat_surprised",  # party bị surprise
        next="combat_surprised",  # default nếu không có spotted branch
    ),

    "combat_spotted": SceneNode(
        node_id="combat_spotted",
        scene_type="COMBAT",
        title="Phát hiện phục kích!",
        narration=(
            "Xạ thủ của đội (Bjorn, Cleric WIS cao nhất) phát hiện bóng nhỏ xanh "
            "nấp trong lùm cây. 4 con goblin!\n\n"
            "Đội không bị bất ngờ — cả team giành initiative bình thường."
        ),
        monster_type="goblin",
        monster_count=4,
        next="post_ambush_loot",
    ),

    "combat_surprised": SceneNode(
        node_id="combat_surprised",
        scene_type="COMBAT",
        title="Phục kích bất ngờ!",
        narration=(
            "Từ trong bụi rậm, bóng nhỏ xanh lao ra — GOBLIN! "
            "Bốn con goblin phục kích!\n\n"
            "Đội bị bất ngờ! Các thành viên bị surprised sẽ mất turn đầu round 1."
        ),
        monster_type="goblin",
        monster_count=4,
        next="post_ambush_loot",
    ),

    "post_ambush_loot": SceneNode(
        node_id="post_ambush_loot",
        scene_type="LOOT",
        title="Loot sau trận phục kích",
        narration=(
            "Sau khi đánh bại 4 con goblin, team lục túi đồ của chúng. "
            "Tiền đồng rơi vãi, vài lọ thuốc, cung ngắn gãy."
        ),
        loot_source="goblin",
        next="phandalin_arrival",
    ),

    # === CHAPTER 2: PHANDALIN (TOWN HUB) ===
    "phandalin_arrival": SceneNode(
        node_id="phandalin_arrival",
        scene_type="TOWN",
        title="Phandalin — Thị trấn biên giới",
        narration=(
            "Sau vài giờ đi bộ, team tới Phandalin — thị trấn biên giới nhỏ "
            "xây trên tàn tích cổ. Đường chính trải sỏi, vài tòa nhà gỗ, "
            "khói bếp nấu chiều lững lờ.\n\n"
            "Barthen's Provisions (cửa hàng), Stonehill Inn (quán trọ), "
            "Miner's Exchange (Halia Thornton), Shrine of Luck (Sister Garaele), "
            "Townmaster's Hall (Harbin Wester) đều mở cửa.\n\n"
            "Team nên làm gì trước?"
        ),
        npcs=["Barthen", "Stonehill", "Halia", "Sister Garaele", "Harbin Wester"],
        choices=[
            {"id": "talk_barthen", "label": "Gặp Elmar Barthen (giao supplies + hỏi Gundren)"},
            {"id": "rest_inn", "label": "Nghỉ tại Stonehill Inn (long rest, full heal)"},
            {"id": "talk_halia", "label": "Gặp Halia Thornton (Zhentarim quest: 100gp giết Glasstaff)"},
            {"id": "talk_garaele", "label": "Gặp Sister Garaele (Harpers quest: banshee Agatha)"},
            {"id": "leave_town", "label": "Rời thị trấn, tiếp tục hành trình"},
        ],
        next="end_demo",
    ),

    "end_demo": SceneNode(
        node_id="end_demo",
        scene_type="END",
        title="Hết demo flow",
        narration="Flow simulator kết thúc ở đây. LMoP thật sẽ tiếp tục Part 3 Spider's Web.",
        next="",
    ),
}


def get_node(node_id: str) -> Optional[SceneNode]:
    return NODES.get(node_id)


def next_after_action(node: SceneNode, action_result: dict) -> str:
    """
    Quyết định node kế tiếp dựa trên action result.
    - TRAVEL: check spotted → on_spotted / on_surprised
    - COMBAT: luôn next (combat over)
    - LOOT: luôn next
    - TOWN: dựa choice_id → map sang node (demo: tất cả → end)
    """
    if node.scene_type == "TRAVEL":
        if action_result.get("spotted"):
            return node.on_spotted or node.next
        return node.on_surprised or node.next
    if node.scene_type == "TOWN":
        choice = action_result.get("chosen")
        if choice == "leave_town":
            return "end_demo"
        return node.node_id   # ở lại town (chưa map NPC sub-node)
    return node.next
