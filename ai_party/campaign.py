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

    # === CHAPTER 2: REDBRAND HIDEOUT (dưới Tresendar Manor) ===
    "tresendar_manor": SceneNode(
        node_id="tresendar_manor",
        scene_type="INTRO",
        title="Tresendar Manor — Cánh cửa hầm",
        narration=(
            "Team tới Tresendar Manor — dinh thự cổ bỏ hoang ở rìa đông Phandalin. "
            "Bụi rậm bao phủ, tường đá rêu phong. Carp (con trai Qellene) đã chỉ "
            "đường hầm bí mật trong bụi cây.\n\n"
            "Team lách qua khe hẹp, xuống hầm tối. Mùi ẩm mốc, tiếng nước nhỏ giọt. "
            "Đây là sào huyệt Redbrand — băng côn đồ áo choàng đỏ khủng bố thị trấn."
        ),
        next="redbrand_cellar",
    ),

    "redbrand_cellar": SceneNode(
        node_id="redbrand_cellar",
        scene_type="TRAVEL",
        title="Hầm rượu — Bẫy hố",
        narration=(
            "Hầm rượu tối om, thùng beer xếp dọc tường. Bể nước trong góc "
            "giấu túi chống nước (Potion of Invisibility). \n\n"
            "Đi qua hành lang, Lyra phát hiện viên đá lỏng lẻo — BẪY HỐ! "
            "(DM đang âm thầm thả d20 check Perception của cả team...)"
        ),
        perception_dc=13,
        stealth_bonus=0,    # trap, không phải monster stealth
        on_spotted="redbrand_barracks",
        on_surprised="redbrand_trap_hit",
        next="redbrand_barracks",
    ),

    "redbrand_trap_hit": SceneNode(
        node_id="redbrand_trap_hit",
        scene_type="INTRO",
        title="Bẫy hố kích hoạt!",
        narration=(
            "Team không phát hiện bẫy! Viên đá lún xuống, lộ hố sâu 10ft. "
            "Ai đi đầu rơi xuống, mất 1d6 damage (bludgeoning).\n\n"
            "Tiếng động vang khắp hầm — Redbrand biết team tới!"
        ),
        next="redbrand_barracks_alerted",
    ),

    "redbrand_barracks": SceneNode(
        node_id="redbrand_barracks",
        scene_type="COMBAT",
        title="Phòng lính Redbrand",
        narration=(
            "Phòng có giường đôi, thùng gỗ. 3 tên Redbrand Ruffian đang chơi bài. "
            "Chúng vồ vũ khí khi thấy team!\n\n"
            "3 Redbrand Ruffian (AC 14, HP 16, CR 1/2). "
            "Chúng chưa kịp phản ứng đầy đủ."
        ),
        monster_type="redbrand",
        monster_count=3,
        next="redbrand_loot",
    ),

    "redbrand_barracks_alerted": SceneNode(
        node_id="redbrand_barracks_alerted",
        scene_type="COMBAT",
        title="Phòng lính Redbrand — Bị phát hiện!",
        narration=(
            "Tiếng bẫy đánh thức 3 tên Redbrand Ruffian! Chúng đã sẵn sàng vũ khí, "
            "đợi team ở cửa phòng.\n\n"
            "3 Redbrand Ruffian (AC 14, HP 16, CR 1/2). "
            "Team có thể bị bao vây ở cửa vào!"
        ),
        monster_type="redbrand",
        monster_count=3,
        next="redbrand_loot",
    ),

    "redbrand_loot": SceneNode(
        node_id="redbrand_loot",
        scene_type="LOOT",
        title="Loot phòng lính",
        narration=(
            "Sau khi hạ 3 Redbrand, team lục phòng. "
            "Tiền đồng, áo choàng đỏ, vài lọ thuốc."
        ),
        loot_source="redbrand",
        next="redbrand_crypt",
    ),

    "redbrand_crypt": SceneNode(
        node_id="redbrand_crypt",
        scene_type="COMBAT",
        title="Crypt Tresendar — Xác sống!",
        narration=(
            "Team vào khu lăng mộ cổ. 3 quan tài đá (sarcophagi) mở nắp. "
            "Bên trong, BỘ XƯƠNG SỐNG chổi dậy! Mắt hố rỗng phát sáng xanh lạnh.\n\n"
            "3 Skeleton (AC 13, HP 11, CR 1/4). Chúng cầm kiếm gỉ."
        ),
        monster_type="skeleton",
        monster_count=3,
        next="redbrand_slave_pens",
    ),

    "redbrand_slave_pens": SceneNode(
        node_id="redbrand_slave_pens",
        scene_type="COMBAT",
        title="Chuồng nô lệ — Cứu Mirna",
        narration=(
            "Cell chia bằng thanh sắt. 2 tên Redbrand canh gác. "
            "Bên trong, Mirna Dendrar + 2 con (Nars, Nilsa) — vợ/con Thel Dendrar "
            "bắt cóc sau khi Redbrand giết chồng.\n\n"
            "2 Redbrand Ruffian canh tù nhân. Phải cứu Mirna!"
        ),
        monster_type="redbrand",
        monster_count=2,
        next="redbrand_glasstaff",
    ),

    "redbrand_glasstaff": SceneNode(
        node_id="redbrand_glasstaff",
        scene_type="COMBAT",
        title="Phòng Glasstaff — Boss Iarno Albrek",
        narration=(
            "Phòng làm việc sang trọng giữa hầm bẩn thỉu. "
            "Iarno 'Glasstaff' Albrek — pháp sư phản bội Lords' Alliance — "
            "đứng sau bàn đọc sách.\n\n"
            "Hắn quay lại, cầm Spider Staff (gậy thủy tinh nhện). "
            "'Vậy các ngươi tới rồi. Black Spider đã báo trước.'\n\n"
            "Iarno (AC 11/14 mage armor, HP 27, CR 2). "
            "Boss cuối Redbrand Hideout!"
        ),
        monster_type="glasstaff",
        monster_count=1,
        next="redbrand_glasstaff_loot",
    ),

    "redbrand_glasstaff_loot": SceneNode(
        node_id="redbrand_glasstaff_loot",
        scene_type="LOOT",
        title="Loot Glasstaff — Kho báu lớn",
        narration=(
            "Sau khi hạ Iarno, team lục phòng. Rương gỗ khóa kỹ. "
            "Trên bàn, bức thư ký hiệu Nhện Đen — bằng chứng Iarno làm việc cho ai!"
        ),
        loot_source="glasstaff",
        next="redbrand_mirna_reward",
    ),

    "redbrand_mirna_reward": SceneNode(
        node_id="redbrand_mirna_reward",
        scene_type="INTRO",
        title="Mirna Dendrar — Lời cảm ơn",
        narration=(
            "Mirna Dendrar khóc nức nở: 'Cảm ơn các anh hùng! "
            "Chồng tôi đã chết bảo vệ thị trấn. Các ngươi đã trả thù cho ông ấy.'\n\n"
            "'Nhà tôi có một thứ cho các ngươi. Vị trí viên đá phát sáng, "
            "trong hang sâu phía bắc. Hãy tìm nó.'\n\n"
            "Team cũng tìm bức thư Black Spider: "
            "'Lord Albrek, kẻ lạ sắp tới Phandalin. Bắt chúng nếu được, "
            "giết nếu phải. Mọi bản đồ người lùn phải giao ta. — Nhện Đen'"
        ),
        next="end_chapter2",
    ),

    "end_chapter2": SceneNode(
        node_id="end_chapter2",
        scene_type="END",
        title="Hết Chương 2 — Redbrand Hideout hoàn thành",
        narration=(
            "Team rời Tresendar Manor, mang theo:\n"
            "- Spider Staff (vũ khí phép từ Iarno)\n"
            "- Bức thư Black Spider (bằng chứng cốt truyện)\n"
            "- Thông tin từ Mirna (viên đá phát sáng)\n"
            "- Quest hoàn thành: Halia trả 100gp, Sildar trả 200gp\n\n"
            "Flow simulator kết thúc Chương 2. "
            "LMoP thật sẽ tiếp tục Chương 3: The Spider's Web."
        ),
        next="",
    ),

    "end_failed": SceneNode(
        node_id="end_failed",
        scene_type="END",
        title="Campaign thất bại",
        narration=(
            "Party thất bại trong encounter. "
            "Không thể tiếp tục LMoP — cần hồi sức hoặc tuyển thêm thành viên."
        ),
        next="",
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
            return "tresendar_manor"   # Chương 2: Redbrand Hideout
        return node.node_id   # ở lại town
    return node.next
