/* =====================================================================
   UI - Implementation
   ===================================================================== */
#include "ui.h"
#include "actor.h"
#include <stdio.h>
#include <string.h>

void ui_hp_bar(int x, int y, int bl, int hp, int maxhp, int fill_col, int empty_col){
    ce_putc(x, y, '[', 7);
    int fill = (maxhp > 0) ? hp * bl / maxhp : 0;
    if(fill > bl) fill = bl;
    for(int i = 0; i < bl; i++){
        ce_putc(x+1+i, y, (i < fill) ? (WCHAR)0x2588 : '.', (i < fill) ? fill_col : empty_col);
    }
    ce_putc(x+1+bl, y, ']', 7);
}

void ui_actor_panel(int x, int y, const Actor *a, const char *title, int title_col){
    int cy = y;
    ce_text(x, cy++, title, title_col);
    cy++;
    char b[80];
    sprintf(b, "%s", a->type->name); ce_text(x, cy++, b, 7);
    sprintf(b, "HP %d/%d", a->hp, a->max_hp); ce_text(x, cy++, b, 10);
    ui_hp_bar(x, cy, 20, a->hp, a->max_hp, 10, 8);
    cy += 2;
    sprintf(b, "AC %d", a->type->ac); ce_text(x, cy++, b, 11);
    int str = a->type->scores[AB_STR];
    int dex = a->type->scores[AB_DEX];
    sprintf(b, "STR %d (%+d)  DEX %d (%+d)", str, actor_ability_mod(str), dex, actor_ability_mod(dex));
    ce_text(x, cy++, b, 7);
}

void ui_log(int x, int y, const char *lines[], int n, int max_lines){
    ce_text(x, y, "=== LOG ===", 14);
    int shown = (n > max_lines) ? max_lines : n;
    int start = n - shown;
    for(int i = 0; i < shown; i++){
        ce_text(x, y + 1 + i, lines[start + i], 7);
    }
}

void ui_inventory(int x, int y, const Inventory *inv, int highlight_idx){
    int cy = y;
    ce_text(x, cy++, "=== INVENTORY ===", 14);
    cy++;
    /* Equipped */
    ce_text(x, cy++, "Equipped:", 11);
    for(int s = SLOT_WEAPON; s < SLOT_COUNT; s++){
        if(inv->equipped[s].type){
            char b[80];
            const char *slot_name[] = {"","Weapon","Armor","","Ring"};
            sprintf(b, " %s: %s", slot_name[s] ? slot_name[s] : "?", inv->equipped[s].type->name);
            ce_text(x, cy++, b, 10);
        }
    }
    cy++;
    ce_text(x, cy++, "Backpack:", 11);
    for(int i = 0; i < inv->count; i++){
        char b[80];
        const char *hl = (i == highlight_idx) ? ">" : " ";
        if(inv->items[i].qty > 1)
            sprintf(b, "%s%d. %s x%d", hl, i+1, inv->items[i].type->name, inv->items[i].qty);
        else
            sprintf(b, "%s%d. %s", hl, i+1, inv->items[i].type->name);
        ce_text(x, cy++, b, (i == highlight_idx) ? 14 : 7);
    }
}

int ui_button(int x, int y, int w, const char *label, int col){
    int h = 3;
    int hover = ce_hoverBox(x, y, x + w, y + h);
    /* Box */
    int bg = hover ? col : 0;
    ce_fillbg(x, y, x+w, y+h, bg);
    ce_putc(x, y, (WCHAR)0x2554, col);
    ce_putc(x+w-1, y, (WCHAR)0x2557, col);
    ce_putc(x, y+h-1, (WCHAR)0x255A, col);
    ce_putc(x+w-1, y+h-1, (WCHAR)0x255D, col);
    for(int i = 1; i < w-1; i++){
        ce_putc(x+i, y, (WCHAR)0x2550, col);
        ce_putc(x+i, y+h-1, (WCHAR)0x2550, col);
    }
    for(int i = 1; i < h-1; i++){
        ce_putc(x, y+i, (WCHAR)0x2551, col);
        ce_putc(x+w-1, y+i, (WCHAR)0x2551, col);
    }
    int lbl_len = (int)strlen(label);
    ce_text(x + (w - lbl_len)/2, y+1, label, hover ? 15 : col);
    return ce_clickedBox(x, y, x+w, y+h);
}
