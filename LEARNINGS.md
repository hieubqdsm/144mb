# LEARNINGS — Bài học từ bug đã gặp (tránh lặp lại)

> File này ghi các bug khó tìm + root cause + cách tránh. Cập nhật mỗi khi gặp bug mới.

---

## 🔴 Bug #1: Mouse click không phản ứng (message pump sai chỗ)

**Ngày**: 2026-08-02
**Triệu chứng**: Click chuột vào nút menu (ui_button) không có phản ứng. Hover thấy highlight, nhưng click không vào. Phím thì OK.

**Thời gian debug**: ~2 giờ (đoán sai hướng nhiều lần: tưởng do dialogue, tưởng do env/DPI)

### Root cause
Message pump `PeekMessage` nằm **sai chỗ** — trong `ce_flip()` (CUỐI vòng lặp), sau khi `game_update()` đã đọc input.

**Chuỗi sự kiện khi click (bug):**
```
ce_inputUpdate()  ← đọc g_mouseBtn (lúc này WM_LBUTTONDOWN chưa dispatch!)
game_update()     ← ui_button check g_mouseBtn[0]==2 → FALSE
ce_flip()         ← MỚI pump message → WM_LBUTTONDOWN set g_mouseBtn[0]=2 (MUỘN 1 frame!)
ce_inputEnd()     ← demote edge 2→1 → MẤT edge ngay
```

→ Edge click `2` không bao giờ sống sót đến `game_update` frame tiếp theo.

### Fix
Chuyển message pump lên **ĐẦU vòng lặp** main loop, TRƯỚC `ce_inputUpdate`:
```c
while(ce_running){
    /* Pump Windows messages TRUOC input update */
    MSG msg;
    while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)){
        if(msg.message == WM_QUIT){ ce_running = 0; break; }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    ce_inputUpdate();      /* giờ g_mouseBtn[0]==2 đã được set */
    game_update(dt);       /* ui_button đọc được edge → click vào */
    ce_flip();
    ce_inputEnd();         /* demote 2→1 SAU khi game_update đã đọc */
}
```

### Bài học
1. **Message pump phải ở ĐẦU frame**, không phải cuối. Windows messages (WM_LBUTTONDOWN, WM_KEYDOWN, WM_MOUSEMOVE) phải được dispatch vào state TRƯỚC khi game logic đọc.
2. **Edge detection (state==2) cần sống sót qua 1 game_update**. Nếu pump + demote cùng frame → edge bị nuốt.
3. **Khi debug input**: thêm overlay hiện raw state (`g_mouseBtn[0]`, `ce_mouseX/Y`) lên màn hình. Nhìn số liệu thật thay vì đoán. Đặc biệt phân biệt `held(1)` vs `clicked-edge(2)`.
4. **Bug "click không vào" nhưng hover OK** = signature của lỗi edge detection. Hover dùng `GetAsyncKeyState`/tọa độ (polling), click dùng WM_xBUTTONDOWN (event) → 2 cơ chế khác nhau, 1 cái hỏng không nghĩa là cái kia hỏng.

### Cách tránh lặp
- Main loop pattern chuẩn: **Pump → Input Update → Game Update → Render → Input End**
- Không bao giờ pump message trong hàm render/flip
- Test click chuột + keyboard ngay khi viết menu/UI đầu tiên

### Files liên quan
- `src_console/engine/gdi_renderer.c`: `ce_run()` (main loop), `ce_flip()` (đã bỏ pump), `ce_inputUpdate/End`
- `src_console/game/ui.c`: `ui_button` (caller, dùng `ce_clickedBox`)

---

## 🟡 Bug #2: (dành cho bug tiếp theo)

*Mô tả khi gặp.*
