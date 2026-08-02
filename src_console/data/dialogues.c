/* =====================================================================
   DATA - Dialogue entries (21: DM intro + 20 NPC).
   Nguồn: docs/lmop/npc-dialogue.md, sáng tác nhẹ cho tự nhiên.
   Song ngữ VI/EN qua cặp _vi/_en trong mỗi DlgLine.
   ===================================================================== */
#include "../game/dialogue.h"

/* =====================================================================
   DLG_DM_INTRO - DM kể intro LMoP (Gundren thuê party)
   ===================================================================== */
static const DlgLine dlg_dm_intro[] = {
    { "DM","DM",
      "Thành phố Neverwinter, bên bờ biển sóng vỗ.",
      "The city of Neverwinter, by the sea where waves crash." },
    { "DM","DM",
      "Tại quán trọ Đồng Xuôi Vàng, một người lùn râu đỏ ngồi đợi các ngươi.",
      "At the Shield Guardian inn, a red-bearded dwarf waits for you." },
    { "Gundren","Gundren",
      "Ta tìm thấy rồi! Một lối vào mỏ cổ... Wave Echo Cave!",
      "I've found it! An entrance to the old mine... Wave Echo Cave!" },
    { "Gundren","Gundren",
      "Các ngươi hãy hộ tống xe bò chở đồ tiếp tế tới Phandalin cho ta.",
      "Escort my supply wagon to Phandalin for me." },
    { "Gundren","Gundren",
      "Ta sẽ đi trước ngựa. Phần thưởng: 10 vàng mỗi người.",
      "I'll ride ahead. Reward: 10 gold each." },
    { "DM","DM",
      "Ba ngày sau, trên con đường Triboar Trail... hai con ngựa chết chắn ngang.",
      "Three days later, on the Triboar Trail... two dead horses block the path." },
    { "DM","DM",
      "Chúng mang yên ghế của Neverwinter. Đây là ngựa của Gundren!",
      "They bear Neverwinter saddles. These are Gundren's horses!" },
};
static const Dialogue D_DM_INTRO = {
    DLG_DM_INTRO, "Phần mở đầu", "Introduction",
    dlg_dm_intro, (int)(sizeof(dlg_dm_intro)/sizeof(dlg_dm_intro[0])),
};

/* =====================================================================
   DLG_SILDAR_RESCUE - Sildar khi cứu ở Cragmaw Hideout
   ===================================================================== */
static const DlgLine dlg_sildar[] = {
    { "Sildar","Sildar",
      "Cảm ơn các ngươi! Ta tưởng mình đã chết trong hang này.",
      "Thank you! I thought I would die in this cave." },
    { "Sildar","Sildar",
      "Ta định tiếp tục tới Phandalin, nơi gần nhất.",
      "I intend to continue to Phandalin, the nearest settlement." },
    { "Sildar","Sildar",
      "Ta sẽ trả 50 vàng nếu các ngươi hộ tống ta.",
      "I'll pay 50 gold if you escort me there." },
    { "Sildar","Sildar",
      "Gundren bị chúng bắt đi hướng bắc. Hãy tìm và cứu bạn ta!",
      "Gundren was taken north. Find and rescue my friend!" },
};
static const Dialogue D_SILDAR = {
    DLG_SILDAR_RESCUE, "Cứu Sildar", "Sildar Rescued",
    dlg_sildar, (int)(sizeof(dlg_sildar)/sizeof(dlg_sildar[0])),
};

/* =====================================================================
   DLG_GUNDREN_RESCUE - Gundren khi cứu ở Cragmaw Castle
   ===================================================================== */
static const DlgLine dlg_gundren[] = {
    { "Gundren","Gundren",
      "Các anh hùng! Ta biết các ngươi sẽ đến!",
      "Heroes! I knew you would come!" },
    { "Gundren","Gundren",
      "Kẻ gọi là Nhện Đen đã bắt ta. Hắn muốn bản đồ tới Wave Echo Cave.",
      "The one called the Black Spider captured me. He wants the map to Wave Echo Cave." },
    { "Gundren","Gundren",
      "Hãy đưa ta về Phandalin, rồi đến mỏ xem số phận hai anh ta Nundro và Tharden.",
      "Escort me back to Phandalin, then go to the mine to learn the fate of my brothers." },
    { "Gundren","Gundren",
      "Nhất định phải đưa tên ác nhân kia ra công lý!",
      "That villain must be brought to justice!" },
};
static const Dialogue D_GUNDREN = {
    DLG_GUNDREN_RESCUE, "Cứu Gundren", "Gundren Rescued",
    dlg_gundren, (int)(sizeof(dlg_gundren)/sizeof(dlg_gundren[0])),
};

/* =====================================================================
   DLG_BARTHEN - Elmar Barthen (Barthen's Provisions)
   ===================================================================== */
static const DlgLine dlg_barthen[] = {
    { "Barthen","Barthen",
      "Chào các anh! Barthen's Provisions luôn chào đón người phiêu lưu.",
      "Hello friends! Barthen's Provisions welcomes adventurers." },
    { "Barthen","Barthen",
      "Gundren nợ các ngươi 10 vàng cho việc hộ tống. Đây, cầm lấy.",
      "Gundren owes you 10 gold for the escort. Here, take it." },
    { "Barthen","Barthen",
      "Buồn thay, ta nghe Gundren bị bắt. Hãy cứu bạn già này!",
      "Sadly, I hear Gundren was captured. Please rescue my old friend!" },
    { "Barthen","Barthen",
      "Hai anh nó là Nundro và Tharden cắm trại ngoài thị trấn. Lâu rồi ta không thấy.",
      "His brothers Nundro and Tharden camp outside town. I haven't seen them in days." },
    { "Barthen","Barthen",
      "Băng Redbrand quấy rối mọi quán hàng. Chúng thường ở quán Sleeping Giant.",
      "The Redbrand gang harasses every shop. They frequent the Sleeping Giant tavern." },
};
static const Dialogue D_BARTHEN = {
    DLG_BARTHEN, "Elmar Barthen", "Elmar Barthen",
    dlg_barthen, (int)(sizeof(dlg_barthen)/sizeof(dlg_barthen[0])),
};

/* =====================================================================
   DLG_LINENE - Linene Graywind (Lionshield Coster)
   ===================================================================== */
static const DlgLine dlg_linene[] = {
    { "Linene","Linene",
      "Vũ khí và áo giáp, có đủ trong phòng sau. Nhưng ta có nguyên tắc.",
      "Weapons and armor, all in the back room. But I have scruples." },
    { "Linene","Linene",
      "Ta không bán cho Redbrand hay kẻ nào đe dọa thị trấn.",
      "I won't sell to the Redbrands or anyone who threatens the town." },
    { "Linene","Linene",
      "Cẩn thận với bọn côn đồ đó. Tránh xa quán Sleeping Giant.",
      "Beware those ruffians. Avoid the Sleeping Giant tavern." },
    { "Linene","Linene",
      "Nếu tìm được hàng ta bị cướp, ta thưởng 50 vàng và tình bạn.",
      "If you recover my stolen goods, I'll reward 50 gold and friendship." },
};
static const Dialogue D_LINENE = {
    DLG_LINENE, "Linene Graywind", "Linene Graywind",
    dlg_linene, (int)(sizeof(dlg_linene)/sizeof(dlg_linene[0])),
};

/* =====================================================================
   DLG_STONEHILL - Toblen Stonehill (Stonehill Inn)
   ===================================================================== */
static const DlgLine dlg_stonehill[] = {
    { "Toblen","Toblen",
      "Chào mừng tới Stonehill Inn! Phòng chỉ 5 bạc một đêm, có ale và cider.",
      "Welcome to the Stonehill Inn! Rooms are 5 silver a night, ale and cider." },
    { "Toblen","Toblen",
      "Ta từ Triboar tới đây tìm mỏ, nhưng hóa ra ta giỏi chạy quán trọ hơn.",
      "I came from Triboar to prospect, but I'm better at running an inn." },
    { "Toblen","Toblen",
      "Redbrand quấy rầy thị trấn mà townmaster Harbin chẳng làm gì!",
      "The Redbrands terrorize the town and townmaster Harbin does nothing!" },
    { "Toblen","Toblen",
      "Nhưng ta không dám chống lại chúng... sợ chúng hại vợ con ta.",
      "But I dare not oppose them... I fear for my wife and children." },
    { "Toblen","Toblen",
      "Ngồi đi, nghe vài tin đồn. Có nhiều chuyện đang xảy ra ở đây lắm.",
      "Sit down, hear some rumors. A lot is happening around here." },
};
static const Dialogue D_STONEHILL = {
    DLG_STONEHILL, "Toblen Stonehill", "Toblen Stonehill",
    dlg_stonehill, (int)(sizeof(dlg_stonehill)/sizeof(dlg_stonehill[0])),
};

/* =====================================================================
   DLG_EDERMATH - Daran Edermath (Order of the Gauntlet)
   ===================================================================== */
static const DlgLine dlg_edermath[] = {
    { "Daran","Daran",
      "Ta là Daran Edermath, đã nghỉ hưu nhưng vẫn để mắt tới mọi thứ.",
      "I'm Daran Edermath, retired but still keeping an eye on things." },
    { "Daran","Daran",
      "Đã đến lúc ai đó đứng lên chống tên Redbrand Glasstaff.",
      "It's time someone stood up to the Redbrand leader Glasstaff." },
    { "Daran","Daran",
      "Sào huyệt chính của chúng nằm dưới Tresendar Manor, phía đông thị trấn.",
      "Their main hideout lies beneath Tresendar Manor, east of town." },
    { "Daran","Daran",
      "Ta có việc cho các ngươi: phế tích Old Owl Well phía đông bắc.",
      "I have a task for you: the ruins of Old Owl Well to the northeast." },
    { "Daran","Daran",
      "Thợ mỏ báo có kẻ đào bới ở đó, và bị xác sống đuổi!",
      "Miners report someone digging there, chased away by undead!" },
    { "Daran","Daran",
      "Hãy tới điều tra. Đó là tháp canh cổ của đế chế Netheril.",
      "Go investigate. It's an old watchtower of the Netheril empire." },
};
static const Dialogue D_EDERMATH = {
    DLG_EDERMATH, "Daran Edermath", "Daran Edermath",
    dlg_edermath, (int)(sizeof(dlg_edermath)/sizeof(dlg_edermath[0])),
};

/* =====================================================================
   DLG_HALIA - Halia Thornton (Zhentarim, Miner's Exchange)
   ===================================================================== */
static const DlgLine dlg_halia[] = {
    { "Halia","Halia",
      "Chào anh bạn. Miner's Exchange luôn chào đón người kiếm tiền.",
      "Hello there. The Miner's Exchange welcomes those who seek profit." },
    { "Halia","Halia",
      "Ta nghe băng Redbrand đang quậy phá. Ta sẵn lòng trả tiền giải quyết chúng.",
      "I hear the Redbrands cause trouble. I'm willing to pay to deal with them." },
    { "Halia","Halia",
      "Đặc biệt là tên thủ lĩnh chúng gọi Glasstaff.",
      "Especially their leader, the one they call Glasstaff." },
    { "Halia","Halia",
      "100 vàng nếu các ngươi tiêu diệt hắn và mang thư từ phòng hắn về đây.",
      "100 gold if you eliminate him and bring me correspondence from his quarters." },
    { "Halia","Halia",
      "(thì thầm) Đừng tin townmaster Harbin. Hắn hèn nhát và yếu đuối.",
      "(whispered) Don't trust townmaster Harbin. He's cowardly and weak." },
};
static const Dialogue D_HALIA = {
    DLG_HALIA, "Halia Thornton", "Halia Thornton",
    dlg_halia, (int)(sizeof(dlg_halia)/sizeof(dlg_halia[0])),
};

/* =====================================================================
   DLG_QELLENE - Qellene Alderleaf (halfling farmer)
   ===================================================================== */
static const DlgLine dlg_qellene[] = {
    { "Qellene","Qellene",
      "Chào các anh hùng! Ta là Qellene Alderleaf. Cần chỗ ngủ?",
      "Hello heroes! I'm Qellene Alderleaf. Need a place to sleep?" },
    { "Qellene","Qellene",
      "Ta có gác mái rơm, miễn phí nếu các ngươi không thích quán trọ.",
      "I have a hayloft, free if you don't fancy the inn." },
    { "Qellene","Qellene",
      "Con ta, Carp, kể nó tìm thấy đường hầm bí mật trong rừng gần Tresendar.",
      "My son Carp says he found a secret tunnel in the woods near Tresendar." },
    { "Qellene","Qellene",
      "Hai tên côn đồ lớn ra từ đó gặp hai Redbrand. Tụi nó không thấy nó.",
      "Two big ugly bandits came out and met two Redbrands. They didn't see him." },
    { "Qellene","Qellene",
      "Nếu tìm Cragmaw Castle hay Wave Echo Cave, hãy hỏi Reidoth - druid già.",
      "If you seek Cragmaw Castle or Wave Echo Cave, ask Reidoth - the old druid." },
    { "Qellene","Qellene",
      "Ông ấy vừa tới Thundertree phía tây bắc. Không tấc đất nào ông không biết.",
      "He just went to Thundertree to the northwest. He knows every inch of the land." },
};
static const Dialogue D_QELLENE = {
    DLG_QELLENE, "Qellene Alderleaf", "Qellene Alderleaf",
    dlg_qellene, (int)(sizeof(dlg_qellene)/sizeof(dlg_qellene[0])),
};

/* =====================================================================
   DLG_GARAELE - Sister Garaele (Harpers, Shrine of Luck)
   ===================================================================== */
static const DlgLine dlg_garaele[] = {
    { "Garaele","Garaele",
      "Chào các ngươi. Ta là Sister Garaele, chăm lo Đền May Mắn.",
      "Greetings. I'm Sister Garaele, keeper of the Shrine of Luck." },
    { "Garaele","Garaele",
      "Ta có nhiệm vụ tế nhị cần người trung gian đáng tin.",
      "I have a delicate mission requiring a trustworthy intermediary." },
    { "Garaele","Garaele",
      "Có con banshee tên Agatha ở Conyberry. Ta cần hỏi bà ta về sách phép Bowgentle.",
      "A banshee named Agatha dwells in Conyberry. I need to ask her about Bowgentle's spellbook." },
    { "Garaele","Garaele",
      "Ta đã tới nhưng bà ta không xuất hiện. Cần ai khéo léo hơn.",
      "I sought her out but she did not appear. I need someone more diplomatic." },
    { "Garaele","Garaele",
      "Hãy mang chiếc lược bạc đính ngọc tặng bà ta. Nịnh sự kiêu hãnh của bà.",
      "Bring her a jeweled silver comb as a gift. Flatter her vanity." },
    { "Garaele","Garaele",
      "Phần thưởng: 3 lọ thuốc hồi máu. Bà ta thích sự lịch thiệp.",
      "Reward: 3 healing potions. She responds well to courtesy." },
};
static const Dialogue D_GARAELE = {
    DLG_GARAELE, "Sister Garaele", "Sister Garaele",
    dlg_garaele, (int)(sizeof(dlg_garaele)/sizeof(dlg_garaele[0])),
};

/* =====================================================================
   DLG_HARBIN - Harbin Wester (townmaster, hèn nhát)
   ===================================================================== */
static const DlgLine dlg_harbin[] = {
    { "Harbin","Harbin",
      "Ồ, v-vậy... các ngươi là nhóm phiêu lưu mới tới?",
      "Oh, s-so... you're the new adventuring party?" },
    { "Harbin","Harbin",
      "Redbrand à? Ch-chúng chỉ là hội lính đánh thuê, không rắc rối lắm đâu...",
      "The Redbrands? Th-they're just a mercenary guild, not much trouble really..." },
    { "Harbin","Harbin",
      "(lau mồ hôi) Ta... ta không thể can thiệp. Phải giữ hòa bình chứ!",
      "(wipes sweat) I... I can't interfere. Must keep the peace!" },
    { "Harbin","Harbin",
      "Nhưng... có việc này. Orc cướp bóc gần Wyvern Tor phía đông.",
      "But... there is this. Orcs raid near Wyvern Tor to the east." },
    { "Harbin","Harbin",
      "100 vàng cho ai đuổi chúng đi. Xin hãy giúp thị trấn!",
      "100 gold to whoever drives them off. Please help the town!" },
};
static const Dialogue D_HARBIN = {
    DLG_HARBIN, "Harbin Wester", "Harbin Wester",
    dlg_harbin, (int)(sizeof(dlg_harbin)/sizeof(dlg_harbin[0])),
};

/* =====================================================================
   DLG_IARNO_LETTER - Bức thư Black Spider cho Iarno (reveal cốt truyện)
   ===================================================================== */
static const DlgLine dlg_iarno_letter[] = {
    { "Thư","Letter",
      "Albrek thân mến,",
      "Dear Lord Albrek," },
    { "Thư","Letter",
      "Gián điệp ta ở Neverwinter báo có người lạ sắp tới Phandalin.",
      "My spies in Neverwinter tell me that strangers are due to arrive in Phandalin." },
    { "Thư","Letter",
      "Có thể chúng làm việc cho người lùn. Bắt nếu được, giết nếu phải.",
      "They could be working for the dwarves. Capture them if you can, kill them if you must." },
    { "Thư","Letter",
      "Nhưng đừng để chúng phá kế hoạch. Mọi bản đồ người lùn phải gửi ta ngay.",
      "But don't allow them to upset our plans. See that any dwarven maps are delivered to me with haste." },
    { "Thư","Letter",
      "Ta trông cậy vào ngươi, Iarno. Đừng làm ta thất vọng.",
      "I'm counting on you, Iarno. Don't disappoint me." },
    { "Thư","Letter",
      "— Ký hiệu: Nhện Đen",
      "— Signed: The Black Spider" },
};
static const Dialogue D_IARNO_LETTER = {
    DLG_IARNO_LETTER, "Bức thư bí ẩn", "A Mysterious Letter",
    dlg_iarno_letter, (int)(sizeof(dlg_iarno_letter)/sizeof(dlg_iarno_letter[0])),
};

/* =====================================================================
   DLG_REIDOTH - Reidoth (druid, Emerald Enclave)
   ===================================================================== */
static const DlgLine dlg_reidoth[] = {
    { "Reidoth","Reidoth",
      "Hừ. Ít khách. Thôi được.",
      "Hmph. Few visitors. Fine." },
    { "Reidoth","Reidoth",
      "Cragmaw Castle? Ta chỉ đường được. Phía đông bắc, trong rừng.",
      "Cragmaw Castle? I can give directions. Northeast, in the forest." },
    { "Reidoth","Reidoth",
      "Wave Echo Cave cũng gần đó. Mười lăm dặm đông Phandalin, trong núi.",
      "Wave Echo Cave is nearby too. Fifteen miles east of Phandalin, in the mountains." },
    { "Reidoth","Reidoth",
      "Nếu muốn giúp ta, đuổi con rồng Venomfang ở tháp bên kia đi.",
      "If you want to help me, drive off the dragon Venomfang in the tower yonder." },
    { "Reidoth","Reidoth",
      "Nó chiếm tổ của pháp sư chết. Nguy hiểm, nhưng có kho báu.",
      "It claimed a dead wizard's tower. Dangerous, but there is treasure." },
};
static const Dialogue D_REIDOTH = {
    DLG_REIDOTH, "Reidoth", "Reidoth",
    dlg_reidoth, (int)(sizeof(dlg_reidoth)/sizeof(dlg_reidoth[0])),
};

/* =====================================================================
   DLG_AGATHA - Agatha (banshee, Conyberry)
   ===================================================================== */
static const DlgLine dlg_agatha[] = {
    { "Agatha","Agatha",
      "Lũ ngu ngốc chết tắt... Các ngươi muốn gì ở đây?",
      "Foolish mortals... What do you want here?" },
    { "Agatha","Agatha",
      "Các ngươi không biết rằng tìm ta là chết đấy sao?",
      "Do you not know it is death to seek me out?" },
    { "Agatha","Agatha",
      "...Chiếc lược đẹp đấy. Được rồi, một câu hỏi. Hỏi đi.",
      "...A lovely comb. Very well, one question. Ask." },
    { "Agatha","Agatha",
      "Sách phép của Bowgentle? Ta từng thấy nó. Nó ở trong phế tích cũ.",
      "Bowgentle's spellbook? I have seen it. It lies in the old ruins." },
    { "Agatha","Agatha",
      "Bây giờ, đi đi. Đừng bao giờ quay lại quấy rầy ta nữa.",
      "Now, begone. Never return to disturb me again." },
};
static const Dialogue D_AGATHA = {
    DLG_AGATHA, "Agatha", "Agatha",
    dlg_agatha, (int)(sizeof(dlg_agatha)/sizeof(dlg_agatha[0])),
};

/* =====================================================================
   DLG_NEZZNAR_BOSS - Nezznar (boss cuối Wave Echo Cave)
   ===================================================================== */
static const DlgLine dlg_nezznar[] = {
    { "Nezznar","Nezznar",
      "Vậy... các ngươi đã tới được tận đây. Quả thật đáng kinh ngạc.",
      "So... you made it all the way here. Truly impressive." },
    { "Nezznar","Nezznar",
      "Ta là Nezznar, kẻ các ngươi gọi là Nhện Đen.",
      "I am Nezznar, the one you call the Black Spider." },
    { "Nezznar","Nezznar",
      "Lò Phép sắp thuộc về ta. Quyền năng chế tạo phép thuật vô tận!",
      "The Forge of Spells will be mine. Infinite power to craft magic!" },
    { "Nezznar","Nezznar",
      "Nhưng các ngươi sẽ không cản bước ta được. Nhẫn con, tiêu diệt chúng!",
      "But you will not stop me. My spiders, destroy them!" },
    { "Nezznar","Nezznar",
      "(lúc hấp hối) Thất bại... chỉ là... tạm thời...",
      "(dying) Defeat... is only... temporary..." },
};
static const Dialogue D_NEZZNAR = {
    DLG_NEZZNAR_BOSS, "Nhện Đen Nezznar", "Nezznar the Black Spider",
    dlg_nezznar, (int)(sizeof(dlg_nezznar)/sizeof(dlg_nezznar[0])),
};

/* =====================================================================
   DLG_KLARG - Klarg (bugbear, Cragmaw Hideout area 8)
   ===================================================================== */
static const DlgLine dlg_klarg[] = {
    { "Klarg","Klarg",
      "GRRR! Klarg là thủ lĩnh ở đây! Klarg phục vụ Nhện Đen vĩ đại!",
      "GRRR! Klarg is leader here! Klarg serves the great Black Spider!" },
    { "Klarg","Klarg",
      "Klarg bắt người lùn già theo lệnh chủ nhân. Klarg được thưởng!",
      "Klarg captured the old dwarf on master's orders. Klarg gets reward!" },
    { "Klarg","Klarg",
      "Các ngươi sẽ không cướp chiến lợi phẩm của Klarg! Sói, cắn chúng!",
      "You won't take Klarg's loot! Wolf, bite them!" },
};
static const Dialogue D_KLARG = {
    DLG_KLARG, "Klarg", "Klarg",
    dlg_klarg, (int)(sizeof(dlg_klarg)/sizeof(dlg_klarg[0])),
};

/* =====================================================================
   DLG_KING_GROL - King Grol (Cragmaw Castle, giữ Gundren)
   ===================================================================== */
static const DlgLine dlg_grol[] = {
    { "King Grol","King Grol",
      "Kẻ xâm nhập! Grol là vua bầy Cragmaw!",
      "Intruders! Grol is king of the Cragmaw tribe!" },
    { "King Grol","King Grol",
      "Người lùn và bản đồ của hắn thuộc về Grol. Nhện Đen sẽ trả giá cao!",
      "The dwarf and his map belong to Grol. The Black Spider will pay handsomely!" },
    { "King Grol","King Grol",
      "Các ngươi sẽ không rời lâu đài này sống sót! Tiêu diệt chúng!",
      "You will not leave this castle alive! Destroy them!" },
};
static const Dialogue D_GROL = {
    DLG_KING_GROL, "Vua Grol", "King Grol",
    dlg_grol, (int)(sizeof(dlg_grol)/sizeof(dlg_grol[0])),
};

/* =====================================================================
   DLG_VENOMFANG - Venomfang (Young Green Dragon, Thundertree)
   ===================================================================== */
static const DlgLine dlg_venomfang[] = {
    { "Venomfang","Venomfang",
      "Hừmmmm... kẻ trần tục dám bước vào lãnh địa của ta?",
      "Hmmmssss... mortals dare enter my domain?" },
    { "Venomfang","Venomfang",
      "Ta là Nanh Độc, rồng xanh trẻ. Tháp này là của ta.",
      "I am Venomfang, young green dragon. This tower is mine." },
    { "Venomfang","Venomfang",
      "Các ngươi có thể... đàm phán. Cống nạp vàng, ta tha mạng.",
      "You may... negotiate. Tribute of gold, and I spare your lives." },
    { "Venomfang","Venomfang",
      "Hoặc... các ngươi sẽ thành bữa tiệc của ta. Cœur của kẻ liều mạng rất ngon.",
      "Or... you will become my feast. An adventurer's heart is delicious." },
    { "Venomfang","Venomfang",
      "(khi bị thương) Đủ rồi! Ta sẽ trở lại...更强的 hình thái!",
      "(when wounded) Enough! I shall return... in greater form!" },
};
static const Dialogue D_VENOMFANG = {
    DLG_VENOMFANG, "Nanh Độc", "Venomfang",
    dlg_venomfang, (int)(sizeof(dlg_venomfang)/sizeof(dlg_venomfang[0])),
};

/* =====================================================================
   DLG_MIRNA - Mirna Dendrar (cứu khỏi Redbrand slave pens)
   ===================================================================== */
static const DlgLine dlg_mirna[] = {
    { "Mirna","Mirna",
      "Cảm ơn trời! Ta tưởng chúng sẽ giết bầy ta!",
      "Thank the heavens! I thought they would kill us!" },
    { "Mirna","Mirna",
      "Chồng ta, Thel, đã đứng ra chống Redbrand. Chúng giết ông ấy trước mặt ta.",
      "My husband Thel stood up to the Redbrands. They killed him before my eyes." },
    { "Mirna","Mirna",
      "Rồi chúng bắt vợ chồng con ta vào chuồng nô lệ này.",
      "Then they dragged my children and me into these slave pens." },
    { "Mirna","Mirna",
      "Nhà ta có một thứ cho các ngươi. Vị trí viên đá phát sáng, trong hang sâu.",
      "Our family has something for you. The location of a glowing stone, deep in a cave." },
};
static const Dialogue D_MIRNA = {
    DLG_MIRNA, "Mirna Dendrar", "Mirna Dendrar",
    dlg_mirna, (int)(sizeof(dlg_mirna)/sizeof(dlg_mirna[0])),
};

/* =====================================================================
   DLG_DROOP - Droop (goblin captured, biết layout Redbrand Hideout)
   ===================================================================== */
static const DlgLine dlg_droop[] = {
    { "Droop","Droop",
      "Đ-đừng giết Droop! Droop chỉ là goblin bé nhỏ!",
      "D-don't kill Droop! Droop is just a small goblin!" },
    { "Droop","Droop",
      "Redbrand bắt Droop, bắt đánh nhau. Droop ghét chúng!",
      "Redbrands captured Droop, made him fight. Droop hates them!" },
    { "Droop","Droop",
      "Droop biết sào huyệt! Droop biết cửa bí mật, biết bẫy!",
      "Droop knows the hideout! Droop knows secret doors, knows traps!" },
    { "Droop","Droop",
      "Thả Droop, Droop chỉ đường! Droop hứa!",
      "Spare Droop, Droop shows the way! Droop promises!" },
};
static const Dialogue D_DROOP = {
    DLG_DROOP, "Droop", "Droop",
    dlg_droop, (int)(sizeof(dlg_droop)/sizeof(dlg_droop[0])),
};

/* =====================================================================
   DLG_NART - Narth (old farmer, rumor tại Stonehill Inn)
   ===================================================================== */
static const DlgLine dlg_narth[] = {
    { "Narth","Narth",
      "(höhöhö) Chào các anh trẻ. Ngồi xuống, uống ale đi.",
      "(heh heh) Hello young ones. Sit, have an ale." },
    { "Narth","Narth",
      "Có nghe nói Sister Garaele ở đền May Mắn đi đâu mấy hôm rồi?",
      "Heard Sister Garaele of the Shrine left town for a few days." },
    { "Narth","Narth",
      "Bà ấy về nhìn thương tích đầy mình! Chắc gặp chuyện gì nguy hiểm.",
      "She came back wounded and exhausted! Must've met something dangerous." },
    { "Narth","Narth",
      "Có gì bí ẩn ở người elf đó... Chắc bà ấy giấu chuyện gì.",
      "Something mysterious about that elf... She must be hiding something." },
};
static const Dialogue D_NART = {
    DLG_NART, "Narth", "Narth",
    dlg_narth, (int)(sizeof(dlg_narth)/sizeof(dlg_narth[0])),
};

/* =====================================================================
   MASTER TABLE
   ===================================================================== */
const Dialogue * const DIALOGUES[DLG_COUNT] = {
    [DLG_NONE]          = NULL,
    [DLG_DM_INTRO]      = &D_DM_INTRO,
    [DLG_SILDAR_RESCUE] = &D_SILDAR,
    [DLG_GUNDREN_RESCUE]= &D_GUNDREN,
    [DLG_BARTHEN]       = &D_BARTHEN,
    [DLG_LINENE]        = &D_LINENE,
    [DLG_STONEHILL]     = &D_STONEHILL,
    [DLG_EDERMATH]      = &D_EDERMATH,
    [DLG_HALIA]         = &D_HALIA,
    [DLG_QELLENE]       = &D_QELLENE,
    [DLG_GARAELE]       = &D_GARAELE,
    [DLG_HARBIN]        = &D_HARBIN,
    [DLG_IARNO_LETTER]  = &D_IARNO_LETTER,
    [DLG_REIDOTH]       = &D_REIDOTH,
    [DLG_AGATHA]        = &D_AGATHA,
    [DLG_NEZZNAR_BOSS]  = &D_NEZZNAR,
    [DLG_KLARG]         = &D_KLARG,
    [DLG_KING_GROL]     = &D_GROL,
    [DLG_VENOMFANG]     = &D_VENOMFANG,
    [DLG_MIRNA]         = &D_MIRNA,
    [DLG_DROOP]         = &D_DROOP,
    [DLG_NART]          = &D_NART,
};
