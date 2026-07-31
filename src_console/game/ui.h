/* =====================================================================
   UI - Sidebar, combat log, inventory panel rendering helpers.
   ===================================================================== */
#ifndef CE_UI_H
#define CE_UI_H

#include "../engine/console.h"
#include "actor.h"
#include "inventory.h"
#include "items.h"

/* Ve HP bar tai (x,y), rong bl cells. fill = hp/max. */
void ui_hp_bar(int x, int y, int bl, int hp, int maxhp, int fill_col, int empty_col);

/* Ve stats panel cho actor (HP/AC/STR...). */
void ui_actor_panel(int x, int y, const Actor *a, const char *title, int title_col);

/* Ve combat log (mang chuoi). */
void ui_log(int x, int y, const char *lines[], int n, int max_lines);

/* Ve inventory panel. */
void ui_inventory(int x, int y, const Inventory *inv, int highlight_idx);

/* Ve button (clickable box). Tra ve 1 neu vua click. */
int ui_button(int x, int y, int w, const char *label, int col);

#endif /* CE_UI_H */
