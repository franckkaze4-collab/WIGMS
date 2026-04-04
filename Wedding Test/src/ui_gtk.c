#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ui_gtk.h"
#include "category.h"
#include "person.h"
#include "priority.h"
#include "gift.h"
#include "parking.h"
#include "seating.h"
#include "schedule.h"

#define GIFTS_FILE "data/gifts.dat"
#define PARKING_FILE "data/parking.dat"
#define SEATING_FILE "data/seating.dat"
#define SCHEDULE_FILE "data/schedule.dat"
#define APP_TITLE  "WIGMS - Wedding Invitation & Gift Management"
#define APP_W      1100
#define APP_H      720

/* =========================================================
   Per-tab contexts
   ========================================================= */

typedef struct {
    AppState      *state;
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *entry_code;
    GtkSpinButton *spin_guests;
    GtkEntry      *g_name[4];
    GtkSpinButton *g_age[4];
    GtkEntry      *g_class[4];
    GtkComboBoxText *g_side[4];
    int            selected_id; /* -1 = none */
} CatCtx;

typedef struct {
    AppState      *state;
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *entry_name;
    GtkSpinButton *spin_age;
    GtkEntry      *entry_class;
    GtkComboBoxText *combo_side;
    int            selected_cat_id;
    int            selected_guest_idx;
} PersonCtx;

typedef struct {
    AppState     *state;
    GtkWidget    *main_win;
    GtkListStore *store;
    GtkTreeView  *tree;
    GtkLabel     *lbl_status;
} PrioCtx;

typedef struct {
    AppState      *state;
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *entry_name;
    GtkSpinButton *spin_value;
    GtkSpinButton *spin_guest;
    GtkLabel      *lbl_total;
    int            selected_id;
} GiftCtx;

typedef struct {
    AppState *state;
    GtkLabel *lbl_cats;
    GtkLabel *lbl_guests;
    GtkLabel *lbl_gifts;
    GtkLabel *lbl_total;
} StatsCtx;

typedef struct {
    AppState           *state;
    GtkWidget          *main_win;
    GtkListStore       *store;
    GtkTreeView        *tree;
    GtkComboBoxText    *combo_zone_filter;
    GtkSpinButton      *spin_guest;
    GtkEntry           *entry_plate;
    GtkComboBoxText    *combo_vehicle;
    GtkComboBoxText    *combo_preferred_zone;
    GtkLabel           *lbl_stats;
    int                 selected_spot_id;
} ParkCtx;

typedef struct {
    AppState        *state;
    GtkWidget       *main_win;
    GtkListStore    *store;
    GtkTreeView     *tree;
    GtkEntry        *entry_table_name;
    GtkSpinButton   *spin_table_capacity;
    GtkComboBoxText *combo_table_type;
    GtkSpinButton   *spin_assign_table;
    GtkSpinButton   *spin_assign_guest;
    GtkEntry        *entry_assign_name;
    GtkComboBoxText *combo_meal;
    GtkComboBoxText *combo_diet;
    GtkSpinButton   *spin_move_guest;
    GtkSpinButton   *spin_move_table;
    GtkSpinButton   *spin_rsvp_guest;
    GtkComboBoxText *combo_rsvp;
    GtkLabel        *lbl_catering;
} SeatCtx;

typedef struct {
    AppState        *state;
    GtkWidget       *main_win;
    GtkListStore    *store;
    GtkTreeView     *tree;
    GtkEntry        *entry_title;
    GtkComboBoxText *combo_category;
    GtkComboBoxText *combo_priority;
    GtkSpinButton   *spin_start_h;
    GtkSpinButton   *spin_start_m;
    GtkSpinButton   *spin_dur;
    GtkEntry        *entry_resp;
    GtkEntry        *entry_loc;
    GtkSpinButton   *spin_delay;
    GtkLabel        *lbl_countdown;
    int              selected_event_id;
} SchedCtx;

/* Single instance contexts so callbacks can refresh other tabs */
static CatCtx    *g_cat_ctx    = NULL;
static PersonCtx *g_person_ctx = NULL;
static PrioCtx    *g_prio_ctx   = NULL;
static GiftCtx   *g_gift_ctx   = NULL;
static StatsCtx  *g_stats_ctx  = NULL;
static ParkCtx   *g_park_ctx   = NULL;
static SeatCtx   *g_seat_ctx   = NULL;
static SchedCtx  *g_sched_ctx  = NULL;

/* =========================================================
   CSS theme
   ========================================================= */

static const char *APP_CSS =
    "window { background-color: #F4F6F9; }"
    ".title-lbl { font-size: 20px; font-weight: bold; color: #1B3A6B; padding: 10px 0; }"
    ".action-btn { background: #E8650A; color: white; border-radius: 5px; padding: 7px 18px; font-weight: bold; border: none; margin: 2px; }"
    ".action-btn:hover { background: #C0392B; }"
    ".delete-btn { background: #C0392B; color: white; border-radius: 5px; padding: 7px 18px; font-weight: bold; border: none; margin: 2px; }"
    ".delete-btn:hover { background: #922B21; }"
    ".stat-box { background: white; border-radius: 8px; padding: 16px; border: 1px solid #DDDDDD; margin: 8px; }"
    ".stat-num { font-size: 36px; font-weight: bold; color: #1B3A6B; }"
    ".stat-label { font-size: 12px; color: #888888; }";

void ui_apply_css(void) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, APP_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);
}

static void style_btn(GtkWidget *btn, const char *css_class) {
    GtkStyleContext *ctx = gtk_widget_get_style_context(btn);
    gtk_style_context_add_class(ctx, css_class);
}

void ui_show_info_dialog(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

void ui_show_error_dialog(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

gboolean ui_confirm_dialog(GtkWindow *parent, const char *question) {
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "%s", question);
    gtk_window_set_title(GTK_WINDOW(d), "Confirm");
    gint r = gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    return (r == GTK_RESPONSE_YES);
}

/* =========================================================
   Small helpers
   ========================================================= */

static void cat_set_form_from_category(CatCtx *ctx, Category *c) {
    if (!ctx) return;
    if (!c) return;

    gtk_entry_set_text(ctx->entry_code, c->code);
    gtk_spin_button_set_value(ctx->spin_guests, c->guest_count);

    for (int i = 0; i < 4; i++) {
        if (i < c->guest_count) {
            gtk_entry_set_text(ctx->g_name[i], c->guests[i].name);
            gtk_spin_button_set_value(ctx->g_age[i], c->guests[i].age);
            gtk_entry_set_text(ctx->g_class[i], c->guests[i].social_class);
            gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->g_side[i]),
                                     c->guests[i].side == LE ? 0 : 1);
        } else {
            gtk_entry_set_text(ctx->g_name[i], "");
            gtk_spin_button_set_value(ctx->g_age[i], 18);
            gtk_entry_set_text(ctx->g_class[i], "");
            gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->g_side[i]), 0);
        }
    }
}

static void cat_clear_form(CatCtx *ctx) {
    if (!ctx) return;
    gtk_entry_set_text(ctx->entry_code, "");
    gtk_spin_button_set_value(ctx->spin_guests, 1);
    for (int i = 0; i < 4; i++) {
        gtk_entry_set_text(ctx->g_name[i], "");
        gtk_spin_button_set_value(ctx->g_age[i], 18);
        gtk_entry_set_text(ctx->g_class[i], "");
        gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->g_side[i]), 0);
    }
}

static void person_refresh_tree(PersonCtx *ctx);
static void prio_refresh_tree(PrioCtx *ctx);
static void stats_refresh(StatsCtx *ctx);
static void gift_refresh(GiftCtx *ctx);

/* =========================================================
   Refresh functions
   ========================================================= */

static void cat_refresh_tree(CatCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);

    Category *c = ctx->state->cat_head;
    while (c) {
        GtkTreeIter it;
        gtk_list_store_append(ctx->store, &it);
        gtk_list_store_set(ctx->store, &it,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
        c = c->next;
    }
}

static void person_refresh_tree(PersonCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);

    Category *c = ctx->state->cat_head;
    while (c) {
        for (int i = 0; i < c->guest_count; i++) {
            GtkTreeIter it;
            gtk_list_store_append(ctx->store, &it);
            gtk_list_store_set(ctx->store, &it,
                0, c->guests[i].id,
                1, c->guests[i].name,
                2, c->guests[i].age,
                3, c->guests[i].social_class,
                4, c->guests[i].side == LE ? 0 : 1,
                5, c->id,
                6, i,
                -1);
        }
        c = c->next;
    }
}

static void prio_refresh_tree(PrioCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);

    Category *c = ctx->state->cat_head;
    while (c) {
        for (int i = 0; i < c->guest_count; i++) {
            GtkTreeIter it;
            gtk_list_store_append(ctx->store, &it);
            gtk_list_store_set(ctx->store, &it,
                0, c->code,
                1, c->guests[i].name,
                2, c->guests[i].age,
                3, c->guests[i].social_class,
                4, c->guests[i].side == LE ? 0 : 1,
                -1);
        }
        c = c->next;
    }
}

static void gift_refresh(GiftCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);

    for (int i = 0; i < ctx->state->gift_count; i++) {
        Gift *g = &ctx->state->gifts[i];
        GtkTreeIter it;
        gtk_list_store_append(ctx->store, &it);
        gtk_list_store_set(ctx->store, &it,
            0, g->gift_id,
            1, g->name,
            2, (gdouble)g->value,
            3, g->guest_id,
            -1);
    }

    char buf[128];
    g_snprintf(buf, sizeof(buf),
               "Total: %.0f FCFA (%d gifts)",
               gift_total_value(ctx->state->gifts, ctx->state->gift_count),
               ctx->state->gift_count);
    gtk_label_set_text(ctx->lbl_total, buf);
}

static void stats_refresh(StatsCtx *ctx) {
    if (!ctx) return;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "%d", category_count_nodes(ctx->state->cat_head));
    gtk_label_set_text(ctx->lbl_cats, buf);

    g_snprintf(buf, sizeof(buf), "%d", category_count_all_guests(ctx->state->cat_head));
    gtk_label_set_text(ctx->lbl_guests, buf);

    g_snprintf(buf, sizeof(buf), "%d", ctx->state->gift_count);
    gtk_label_set_text(ctx->lbl_gifts, buf);

    g_snprintf(buf, sizeof(buf), "%.0f FCFA",
               gift_total_value(ctx->state->gifts, ctx->state->gift_count));
    gtk_label_set_text(ctx->lbl_total, buf);
}

/* =========================================================
   CATEGORY TAB callbacks
   ========================================================= */

static void on_cat_row_selected(GtkTreeSelection *sel, gpointer d) {
    CatCtx *ctx = (CatCtx*)d;
    GtkTreeIter it;
    GtkTreeModel *model;

    if (!gtk_tree_selection_get_selected(sel, &model, &it)) {
        ctx->selected_id = -1;
        return;
    }

    int id = -1;
    gtk_tree_model_get(model, &it, 0, &id, -1);
    ctx->selected_id = id;

    Category *c = category_find_by_id(ctx->state->cat_head, id);
    if (!c) return;
    cat_set_form_from_category(ctx, c);
}

static void on_cat_add(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx*)d;

    const char *code = gtk_entry_get_text(ctx->entry_code);
    if (!code || !code[0]) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Missing Code", "Please enter a category code.");
        return;
    }

    int n = (int)gtk_spin_button_get_value(ctx->spin_guests);
    if (n < 0) n = 0;
    if (n > 4) n = 4;

    Category *c = category_create(category_id_counter++, code);
    c->guest_count = n;

    for (int i = 0; i < n; i++) {
        const char *nm  = gtk_entry_get_text(ctx->g_name[i]);
        int age          = (int)gtk_spin_button_get_value(ctx->g_age[i]);
        const char *cls = gtk_entry_get_text(ctx->g_class[i]);
        int sid          = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->g_side[i]));
        if (age < 0) age = 0;

        c->guests[i] = create_person_data(
            person_id_counter++,
            (nm && nm[0]) ? nm : "Unknown",
            age,
            (cls && cls[0]) ? cls : "Friend",
            sid == 0 ? LE : LA
        );
    }

    category_insert(&ctx->state->cat_head, c);
    ctx->selected_id = -1;
    cat_refresh_tree(ctx);

    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_prio_ctx) prio_refresh_tree(g_prio_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);

    cat_clear_form(ctx);
}

static void on_cat_update(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx*)d;

    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "No Selection", "Select a category first.");
        return;
    }

    Category *c = category_find_by_id(ctx->state->cat_head, ctx->selected_id);
    if (!c) return;

    const char *code = gtk_entry_get_text(ctx->entry_code);
    if (code && code[0]) {
        strncpy(c->code, code, sizeof(c->code) - 1);
        c->code[sizeof(c->code) - 1] = '\0';
    }

    int n = (int)gtk_spin_button_get_value(ctx->spin_guests);
    if (n < 0) n = 0;
    if (n > 4) n = 4;
    int old_count = c->guest_count;
    c->guest_count = n;

    for (int i = 0; i < n; i++) {
        const char *nm  = gtk_entry_get_text(ctx->g_name[i]);
        int age          = (int)gtk_spin_button_get_value(ctx->g_age[i]);
        const char *cls = gtk_entry_get_text(ctx->g_class[i]);
        int sid          = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->g_side[i]));
        if (age < 0) age = 0;

        /* Keep existing guest id for already-existing slots;
           allocate new ids for newly-added slots. */
        int guest_id;
        if (i < old_count && c->guests[i].id != 0) {
            guest_id = c->guests[i].id;
        } else {
            guest_id = person_id_counter++;
        }
        c->guests[i] = create_person_data(
            guest_id,
            (nm && nm[0]) ? nm : "Unknown",
            age,
            (cls && cls[0]) ? cls : "Friend",
            sid == 0 ? LE : LA
        );
    }

    cat_refresh_tree(ctx);
    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_prio_ctx) prio_refresh_tree(g_prio_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
}

static void on_cat_delete(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx*)d;

    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "No Selection", "Select a category first.");
        return;
    }

    if (!ui_confirm_dialog(GTK_WINDOW(ctx->main_win),
                            "Delete this category and all its guests?")) {
        return;
    }

    category_delete(&ctx->state->cat_head, ctx->selected_id);
    ctx->selected_id = -1;
    cat_refresh_tree(ctx);

    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_prio_ctx) prio_refresh_tree(g_prio_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);

    cat_clear_form(ctx);
}

static void on_cat_sort(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx*)d;

    category_sort_desc(&ctx->state->cat_head);
    ctx->selected_id = -1;
    cat_refresh_tree(ctx);

    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_prio_ctx) prio_refresh_tree(g_prio_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
    cat_clear_form(ctx);
}

static GtkWidget* build_category_tab(AppState *state, GtkWidget *main_win) {
    CatCtx *ctx = g_new0(CatCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    ctx->selected_id = -1;

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(hbox, 12);
    gtk_widget_set_margin_end(hbox, 12);
    gtk_widget_set_margin_top(hbox, 12);
    gtk_widget_set_margin_bottom(hbox, 12);

    /* Left: tree */
    GtkWidget *lbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(lbox, 360, -1);

    GtkWidget *lbl_tree = gtk_label_new("Categories");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_tree), "title-lbl");
    gtk_box_pack_start(GTK_BOX(lbox), lbl_tree, FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    GtkCellRenderer *r;
    GtkTreeViewColumn *col;

    r = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_column_set_expand(col, FALSE);
    gtk_tree_view_append_column(ctx->tree, col);

    r = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("Code", r, "text", 1, NULL);
    gtk_tree_view_column_set_expand(col, TRUE);
    gtk_tree_view_append_column(ctx->tree, col);

    r = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("Guests", r, "text", 2, NULL);
    gtk_tree_view_column_set_expand(col, FALSE);
    gtk_tree_view_append_column(ctx->tree, col);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(ctx->tree);
    g_signal_connect(sel, "changed", G_CALLBACK(on_cat_row_selected), ctx);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(lbox), scroll, TRUE, TRUE, 0);

    GtkWidget *btn_sort = gtk_button_new_with_label("Sort by Guests (desc)");
    GtkWidget *btn_del  = gtk_button_new_with_label("Delete Selected");
    style_btn(btn_sort, "action-btn");
    style_btn(btn_del, "delete-btn");
    g_signal_connect(btn_sort, "clicked", G_CALLBACK(on_cat_sort), ctx);
    g_signal_connect(btn_del,  "clicked", G_CALLBACK(on_cat_delete), ctx);

    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(btn_row), btn_sort, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_row), btn_del, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(lbox), btn_row, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), lbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);

    /* Right: form */
    GtkWidget *rbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(rbox, TRUE);

    GtkWidget *lbl_form = gtk_label_new("Add / Edit Category");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_form), "title-lbl");
    gtk_box_pack_start(GTK_BOX(rbox), lbl_form, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Category Code:"), 0, 0, 1, 1);
    ctx->entry_code = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(ctx->entry_code, "e.g. FAM_LE, VIP_LA");
    gtk_widget_set_hexpand(GTK_WIDGET(ctx->entry_code), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->entry_code), 1, 0, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Number of Guests (0-4):"), 0, 1, 1, 1);
    ctx->spin_guests = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 4, 1));
    gtk_spin_button_set_value(ctx->spin_guests, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->spin_guests), 1, 1, 1, 1);

    gtk_box_pack_start(GTK_BOX(rbox), grid, FALSE, FALSE, 0);

    GtkWidget *lbl_guest = gtk_label_new("Guest Details (up to 4)");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_guest), "title-lbl");
    gtk_box_pack_start(GTK_BOX(rbox), lbl_guest, FALSE, FALSE, 0);

    GtkWidget *ggrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(ggrid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(ggrid), 8);

    const char *headers[] = {"#", "Name", "Age", "Social Class", "Side"};
    for (int i = 0; i < 5; i++) {
        GtkWidget *h = gtk_label_new(headers[i]);
        gtk_widget_set_halign(h, GTK_ALIGN_CENTER);
        gtk_grid_attach(GTK_GRID(ggrid), h, i, 0, 1, 1);
    }

    for (int i = 0; i < 4; i++) {
        char idxbuf[8];
        g_snprintf(idxbuf, sizeof(idxbuf), "%d", i + 1);
        gtk_grid_attach(GTK_GRID(ggrid), gtk_label_new(idxbuf), 0, i + 1, 1, 1);

        ctx->g_name[i] = GTK_ENTRY(gtk_entry_new());
        gtk_entry_set_placeholder_text(ctx->g_name[i], "Full name");
        gtk_widget_set_size_request(GTK_WIDGET(ctx->g_name[i]), 160, -1);
        gtk_grid_attach(GTK_GRID(ggrid), GTK_WIDGET(ctx->g_name[i]), 1, i + 1, 1, 1);

        ctx->g_age[i] = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 120, 1));
        gtk_spin_button_set_value(ctx->g_age[i], 25);
        gtk_grid_attach(GTK_GRID(ggrid), GTK_WIDGET(ctx->g_age[i]), 2, i + 1, 1, 1);

        ctx->g_class[i] = GTK_ENTRY(gtk_entry_new());
        gtk_entry_set_placeholder_text(ctx->g_class[i], "VIP/Family/Friend");
        gtk_widget_set_size_request(GTK_WIDGET(ctx->g_class[i]), 120, -1);
        gtk_grid_attach(GTK_GRID(ggrid), GTK_WIDGET(ctx->g_class[i]), 3, i + 1, 1, 1);

        ctx->g_side[i] = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
        gtk_combo_box_text_append_text(ctx->g_side[i], "Le marie");
        gtk_combo_box_text_append_text(ctx->g_side[i], "La mariee");
        gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->g_side[i]), 0);
        gtk_grid_attach(GTK_GRID(ggrid), GTK_WIDGET(ctx->g_side[i]), 4, i + 1, 1, 1);
    }

    gtk_box_pack_start(GTK_BOX(rbox), ggrid, FALSE, FALSE, 0);

    GtkWidget *btn_add  = gtk_button_new_with_label("Add Category");
    GtkWidget *btn_upd  = gtk_button_new_with_label("Update Selected");
    style_btn(btn_add, "action-btn");
    style_btn(btn_upd, "action-btn");
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_cat_add), ctx);
    g_signal_connect(btn_upd, "clicked", G_CALLBACK(on_cat_update), ctx);

    GtkWidget *fbtn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(fbtn), btn_add, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(fbtn), btn_upd, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(rbox), fbtn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), rbox, TRUE, TRUE, 0);

    cat_refresh_tree(ctx);
    cat_clear_form(ctx);

    g_cat_ctx = ctx;
    return hbox;
}

/* =========================================================
   PERSON TAB callbacks
   ========================================================= */

static void on_person_row_selected(GtkTreeSelection *sel, gpointer d) {
    PersonCtx *ctx = (PersonCtx*)d;
    GtkTreeIter it;
    GtkTreeModel *model;

    if (!gtk_tree_selection_get_selected(sel, &model, &it)) {
        ctx->selected_cat_id = -1;
        ctx->selected_guest_idx = -1;
        return;
    }

    gchar *nm = NULL;
    gchar *cls = NULL;
    gint guest_id = 0;
    gint age = 0;
    gint side = 0;
    gint cat_id = 0;
    gint g_idx = 0;

    gtk_tree_model_get(model, &it,
        0, &guest_id,
        1, &nm,
        2, &age,
        3, &cls,
        4, &side,
        5, &cat_id,
        6, &g_idx,
        -1);

    gtk_entry_set_text(ctx->entry_name, nm ? nm : "");
    gtk_spin_button_set_value(ctx->spin_age, age);
    gtk_entry_set_text(ctx->entry_class, cls ? cls : "");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_side), side == 0 ? 0 : 1);

    ctx->selected_cat_id = cat_id;
    ctx->selected_guest_idx = g_idx;

    if (nm) g_free(nm);
    if (cls) g_free(cls);
    (void)guest_id;
}

static void on_person_update(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx*)d;
    if (ctx->selected_cat_id < 0 || ctx->selected_guest_idx < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "No Selection", "Select a guest row first.");
        return;
    }

    Category *c = category_find_by_id(ctx->state->cat_head, ctx->selected_cat_id);
    if (!c) return;

    int i = ctx->selected_guest_idx;
    if (i < 0 || i >= c->guest_count) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Invalid Index", "Selected guest is no longer available.");
        return;
    }

    const char *nm = gtk_entry_get_text(ctx->entry_name);
    int age = (int)gtk_spin_button_get_value(ctx->spin_age);
    const char *cls = gtk_entry_get_text(ctx->entry_class);
    int sid = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_side));
    if (age < 0) age = 0;

    c->guests[i] = create_person_data(
        c->guests[i].id,
        (nm && nm[0]) ? nm : "Unknown",
        age,
        (cls && cls[0]) ? cls : "Friend",
        sid == 0 ? LE : LA
    );

    person_refresh_tree(ctx);
    if (g_prio_ctx) prio_refresh_tree(g_prio_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
}

static GtkWidget* build_person_tab(AppState *state, GtkWidget *main_win) {
    PersonCtx *ctx = g_new0(PersonCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    ctx->selected_cat_id = -1;
    ctx->selected_guest_idx = -1;

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(hbox, 12);
    gtk_widget_set_margin_end(hbox, 12);
    gtk_widget_set_margin_top(hbox, 12);
    gtk_widget_set_margin_bottom(hbox, 12);

    /* Left: tree */
    GtkWidget *lbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(lbox, 480, -1);

    GtkWidget *lbl = gtk_label_new("All Guests");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(lbox), lbl, FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(7,
        G_TYPE_INT,    /* 0 guest id */
        G_TYPE_STRING, /* 1 name */
        G_TYPE_INT,    /* 2 age */
        G_TYPE_STRING, /* 3 social class */
        G_TYPE_INT,    /* 4 side (0/1) */
        G_TYPE_INT,    /* 5 category id */
        G_TYPE_INT);   /* 6 guest index */

    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    const char *cols[] = {"ID","Name","Age","Class","Side"};
    for (int i = 0; i < 5; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        int col_id = (i == 0) ? 0 : (i == 1) ? 1 : (i == 2) ? 2 : (i == 3) ? 3 : 4;
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", col_id, NULL);
        gtk_tree_view_column_set_expand(col, (i == 1 || i == 3));
        gtk_tree_view_append_column(ctx->tree, col);
    }

    GtkTreeSelection *sel = gtk_tree_view_get_selection(ctx->tree);
    g_signal_connect(sel, "changed", G_CALLBACK(on_person_row_selected), ctx);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(lbox), scroll, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), lbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);

    /* Right: editor */
    GtkWidget *rbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(rbox, TRUE);

    GtkWidget *lbl2 = gtk_label_new("Edit Selected Guest");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl2), "title-lbl");
    gtk_box_pack_start(GTK_BOX(rbox), lbl2, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Full Name:"), 0, 0, 1, 1);
    ctx->entry_name = GTK_ENTRY(gtk_entry_new());
    gtk_widget_set_hexpand(GTK_WIDGET(ctx->entry_name), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->entry_name), 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Age:"), 0, 1, 1, 1);
    ctx->spin_age = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 120, 1));
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->spin_age), 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Social Class:"), 0, 2, 1, 1);
    ctx->entry_class = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->entry_class), 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Side:"), 0, 3, 1, 1);
    ctx->combo_side = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_side, "Le marie");
    gtk_combo_box_text_append_text(ctx->combo_side, "La mariee");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_side), 0);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->combo_side), 1, 3, 1, 1);

    gtk_box_pack_start(GTK_BOX(rbox), grid, FALSE, FALSE, 0);

    GtkWidget *btn_upd = gtk_button_new_with_label("Update Guest");
    style_btn(btn_upd, "action-btn");
    g_signal_connect(btn_upd, "clicked", G_CALLBACK(on_person_update), ctx);
    gtk_box_pack_start(GTK_BOX(rbox), btn_upd, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), rbox, TRUE, TRUE, 0);

    person_refresh_tree(ctx);
    g_person_ctx = ctx;
    return hbox;
}

/* =========================================================
   PRIORITY TAB callbacks
   ========================================================= */

static void on_prio_by_age(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx*)d;
    priority_sort_by_age(ctx->state->cat_head);
    prio_refresh_tree(ctx);
    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
    gtk_label_set_text(ctx->lbl_status, "Guests sorted by age (ascending) within categories.");
}

static void on_prio_by_class(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx*)d;
    priority_sort_by_class(ctx->state->cat_head);
    prio_refresh_tree(ctx);
    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
    gtk_label_set_text(ctx->lbl_status, "Guests sorted by social class (VIP first) within categories.");
}

static void on_prio_merge_sort(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx*)d;
    priority_merge_sort_categories(&ctx->state->cat_head);
    if (g_cat_ctx) cat_refresh_tree(g_cat_ctx);
    prio_refresh_tree(ctx);
    if (g_person_ctx) person_refresh_tree(g_person_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
    gtk_label_set_text(ctx->lbl_status, "Categories sorted by guest count (Merge Sort / DPR).");
}

static GtkWidget* build_priority_tab(AppState *state, GtkWidget *main_win) {
    PrioCtx *ctx = g_new0(PrioCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new("Priority Management (DPR)");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    GtkWidget *brow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *b1 = gtk_button_new_with_label("Sort Guests by Age");
    GtkWidget *b2 = gtk_button_new_with_label("Sort Guests by Social Class");
    GtkWidget *b3 = gtk_button_new_with_label("Merge Sort Categories by Guest Count");
    style_btn(b1, "action-btn");
    style_btn(b2, "action-btn");
    style_btn(b3, "action-btn");
    g_signal_connect(b1, "clicked", G_CALLBACK(on_prio_by_age), ctx);
    g_signal_connect(b2, "clicked", G_CALLBACK(on_prio_by_class), ctx);
    g_signal_connect(b3, "clicked", G_CALLBACK(on_prio_merge_sort), ctx);
    gtk_box_pack_start(GTK_BOX(brow), b1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), brow, FALSE, FALSE, 0);

    ctx->lbl_status = GTK_LABEL(gtk_label_new("Select a sort action above."));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(ctx->lbl_status), FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(5,
        G_TYPE_STRING, /* category code */
        G_TYPE_STRING, /* guest name */
        G_TYPE_INT,    /* age */
        G_TYPE_STRING, /* social class */
        G_TYPE_INT);   /* side (0/1) */

    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    const char *cols[] = {"Category","Guest Name","Age","Social Class","Side"};
    for (int i = 0; i < 5; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        int col_id = i;
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", col_id, NULL);
        gtk_tree_view_column_set_expand(col, (i == 0 || i == 1 || i == 3));
        gtk_tree_view_append_column(ctx->tree, col);
    }

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    prio_refresh_tree(ctx);
    g_prio_ctx = ctx;
    return vbox;
}

/* =========================================================
   GIFT TAB callbacks
   ========================================================= */

static void on_gift_row_selected(GtkTreeSelection *sel, gpointer d) {
    GiftCtx *ctx = (GiftCtx*)d;
    GtkTreeIter it;
    GtkTreeModel *model;

    if (!gtk_tree_selection_get_selected(sel, &model, &it)) {
        ctx->selected_id = -1;
        return;
    }

    int gid = -1;
    gchar *nm = NULL;
    gdouble val = 0.0;
    int guest_id = 0;

    gtk_tree_model_get(model, &it,
        0, &gid,
        1, &nm,
        2, &val,
        3, &guest_id,
        -1);

    ctx->selected_id = gid;
    gtk_entry_set_text(ctx->entry_name, nm ? nm : "");
    gtk_spin_button_set_value(ctx->spin_value, (gdouble)val);
    gtk_spin_button_set_value(ctx->spin_guest, (gdouble)guest_id);

    if (nm) g_free(nm);
}

static void on_gift_add(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx*)d;
    const char *nm = gtk_entry_get_text(ctx->entry_name);
    if (!nm || !nm[0]) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Missing Name", "Enter a gift name.");
        return;
    }

    float val = (float)gtk_spin_button_get_value(ctx->spin_value);
    int gid = (int)gtk_spin_button_get_value(ctx->spin_guest);

    gift_register(ctx->state->gifts, &ctx->state->gift_count, nm, val, gid);
    gift_refresh(ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);

    gtk_entry_set_text(ctx->entry_name, "");
}

static void on_gift_update(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx*)d;
    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "No Selection", "Select a gift first.");
        return;
    }

    const char *nm = gtk_entry_get_text(ctx->entry_name);
    float val = (float)gtk_spin_button_get_value(ctx->spin_value);
    int guest_id = (int)gtk_spin_button_get_value(ctx->spin_guest);
    gift_update(ctx->state->gifts, ctx->state->gift_count, ctx->selected_id,
                 nm && nm[0] ? nm : "Gift", val, guest_id);
    gift_refresh(ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
}

static void on_gift_delete(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx*)d;
    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "No Selection", "Select a gift first.");
        return;
    }

    if (!ui_confirm_dialog(GTK_WINDOW(ctx->main_win), "Delete this gift?")) return;

    gift_delete(ctx->state->gifts, &ctx->state->gift_count, ctx->selected_id);
    ctx->selected_id = -1;
    gift_refresh(ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);
}

static void on_gift_save(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx*)d;
    gift_save(ctx->state->gifts, ctx->state->gift_count, GIFTS_FILE);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Saved", "Gifts saved to data/gifts.dat");
}

static GtkWidget* build_gift_tab(AppState *state, GtkWidget *main_win) {
    GiftCtx *ctx = g_new0(GiftCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    ctx->selected_id = -1;

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(hbox, 12);
    gtk_widget_set_margin_end(hbox, 12);
    gtk_widget_set_margin_top(hbox, 12);
    gtk_widget_set_margin_bottom(hbox, 12);

    /* Left: tree */
    GtkWidget *lbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(lbox, 420, -1);

    GtkWidget *lbl = gtk_label_new("Gift Registry");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(lbox), lbl, FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(4,
        G_TYPE_INT,    /* gift id */
        G_TYPE_STRING, /* name */
        G_TYPE_DOUBLE, /* value */
        G_TYPE_INT);   /* guest id */

    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    const char *cols[] = {"ID","Gift Name","Value (FCFA)","Guest ID"};
    for (int i = 0; i < 4; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", i, NULL);
        gtk_tree_view_column_set_expand(col, (i == 1));
        gtk_tree_view_append_column(ctx->tree, col);
    }

    GtkTreeSelection *sel = gtk_tree_view_get_selection(ctx->tree);
    g_signal_connect(sel, "changed", G_CALLBACK(on_gift_row_selected), ctx);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(lbox), scroll, TRUE, TRUE, 0);

    ctx->lbl_total = GTK_LABEL(gtk_label_new("Total: 0 FCFA"));
    gtk_box_pack_start(GTK_BOX(lbox), GTK_WIDGET(ctx->lbl_total), FALSE, FALSE, 0);

    GtkWidget *btn_del = gtk_button_new_with_label("Delete Selected");
    style_btn(btn_del, "delete-btn");
    g_signal_connect(btn_del, "clicked", G_CALLBACK(on_gift_delete), ctx);
    gtk_box_pack_start(GTK_BOX(lbox), btn_del, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), lbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_separator_new(GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);

    /* Right: form */
    GtkWidget *rbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(rbox, TRUE);

    GtkWidget *lbl2 = gtk_label_new("Add / Edit Gift");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl2), "title-lbl");
    gtk_box_pack_start(GTK_BOX(rbox), lbl2, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Gift Name:"), 0, 0, 1, 1);
    ctx->entry_name = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(ctx->entry_name, "e.g. Suitcase, Perfume");
    gtk_widget_set_hexpand(GTK_WIDGET(ctx->entry_name), TRUE);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->entry_name), 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Value (FCFA):"), 0, 1, 1, 1);
    ctx->spin_value = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 9999999, 500));
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->spin_value), 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Given by Guest ID:"), 0, 2, 1, 1);
    ctx->spin_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->spin_guest), 1, 2, 1, 1);

    gtk_box_pack_start(GTK_BOX(rbox), grid, FALSE, FALSE, 0);

    GtkWidget *brow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *badd = gtk_button_new_with_label("Add Gift");
    GtkWidget *bupd = gtk_button_new_with_label("Update Selected");
    GtkWidget *bsave = gtk_button_new_with_label("Save to File");
    style_btn(badd, "action-btn");
    style_btn(bupd, "action-btn");
    style_btn(bsave, "action-btn");
    g_signal_connect(badd, "clicked", G_CALLBACK(on_gift_add), ctx);
    g_signal_connect(bupd, "clicked", G_CALLBACK(on_gift_update), ctx);
    g_signal_connect(bsave, "clicked", G_CALLBACK(on_gift_save), ctx);
    gtk_box_pack_start(GTK_BOX(brow), badd, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), bupd, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), bsave, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(rbox), brow, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), rbox, TRUE, TRUE, 0);

    gift_refresh(ctx);
    g_gift_ctx = ctx;
    return hbox;
}

/* =========================================================
   PARKING TAB (Module 6)
   ========================================================= */

static int parking_zone_filter_active(ParkCtx *ctx) {
    /* 0 = All, 1..4 = ZONE_VIP .. ZONE_BUS */
    int a = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_zone_filter));
    return a;
}

static void parking_refresh_tree(ParkCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);
    ParkingLot *lot = &ctx->state->parking;
    int filt = parking_zone_filter_active(ctx);

    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (filt > 0 && (int)s->zone != (filt - 1))
            continue;

        char tbuf[32];
        parking_format_time(s->entry_time, tbuf, sizeof(tbuf));

        GtkTreeIter it;
        gtk_list_store_append(ctx->store, &it);
        gtk_list_store_set(ctx->store, &it,
            0, s->spot_id,
            1, parking_zone_name(s->zone),
            2, parking_status_name(s->status),
            3, s->guest_id,
            4, s->plate[0] ? s->plate : "-",
            5, parking_vehicle_name(s->vehicle_type),
            6, tbuf,
            -1);
    }

    ParkingStats st = parking_get_stats(lot);
    char buf[256];
    g_snprintf(buf, sizeof(buf),
        "Spots: %d | Avail: %d | Occ: %d | Res: %d | Block: %d | VIP occ/avail: %d/%d",
        st.total_spots, st.available, st.occupied, st.reserved, st.blocked,
        st.vip_occupied, st.vip_available);
    gtk_label_set_text(ctx->lbl_stats, buf);
}

static void on_parking_row_selected(GtkTreeSelection *sel, gpointer d) {
    ParkCtx *ctx = (ParkCtx*)d;
    GtkTreeIter it;
    GtkTreeModel *model;
    if (!gtk_tree_selection_get_selected(sel, &model, &it)) {
        ctx->selected_spot_id = -1;
        return;
    }
    int sid = -1;
    gtk_tree_model_get(model, &it, 0, &sid, -1);
    ctx->selected_spot_id = sid;
}

static void on_parking_assign(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx*)d;
    int guest = (int)gtk_spin_button_get_value(ctx->spin_guest);
    const char *plate = gtk_entry_get_text(ctx->entry_plate);
    if (!plate || !plate[0]) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Enter a plate number.");
        return;
    }
    int v = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_vehicle));
    if (v < 0) v = 0;
    VehicleType vt = (VehicleType)v;

    int pz = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_preferred_zone));
    if (pz < 0) pz = (int)ZONE_STANDARD;
    ParkingZone pref = (ParkingZone)pz;

    int sid = parking_assign(&ctx->state->parking, guest, plate, vt, pref);
    if (sid < 0)
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Parking", "No available spot.");
    else
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Vehicle assigned to a spot.");
    parking_refresh_tree(ctx);
}

static void on_parking_release(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx*)d;
    if (ctx->selected_spot_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Select a spot in the list.");
        return;
    }
    if (!parking_release(&ctx->state->parking, ctx->selected_spot_id))
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Cannot release (not occupied).");
    else
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Spot released.");
    ctx->selected_spot_id = -1;
    parking_refresh_tree(ctx);
}

static void on_parking_save(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx*)d;
    parking_save(&ctx->state->parking, PARKING_FILE);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Parking", "Saved to data/parking.dat");
}

static void on_parking_filter_changed(GtkWidget *w, gpointer d) {
    (void)w;
    parking_refresh_tree((ParkCtx*)d);
}

static GtkWidget* build_parking_tab(AppState *state, GtkWidget *main_win) {
    ParkCtx *ctx = g_new0(ParkCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    ctx->selected_spot_id = -1;
    g_park_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new("Parking (Module 6)");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    ctx->lbl_stats = GTK_LABEL(gtk_label_new(""));
    gtk_widget_set_halign(GTK_WIDGET(ctx->lbl_stats), GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(ctx->lbl_stats), FALSE, FALSE, 0);

    GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(h), gtk_label_new("Zone filter:"), FALSE, FALSE, 0);
    ctx->combo_zone_filter = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_zone_filter, "All zones");
    gtk_combo_box_text_append_text(ctx->combo_zone_filter, "VIP");
    gtk_combo_box_text_append_text(ctx->combo_zone_filter, "Standard");
    gtk_combo_box_text_append_text(ctx->combo_zone_filter, "Moto");
    gtk_combo_box_text_append_text(ctx->combo_zone_filter, "Bus");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_zone_filter), 0);
    g_signal_connect(ctx->combo_zone_filter, "changed", G_CALLBACK(on_parking_filter_changed), ctx);
    gtk_box_pack_start(GTK_BOX(h), GTK_WIDGET(ctx->combo_zone_filter), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), h, FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(7,
        G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    const char *cols[] = {"Spot","Zone","Status","Guest","Plate","Vehicle","Entry"};
    for (int i = 0; i < 7; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", i, NULL);
        gtk_tree_view_column_set_expand(col, i == 1 || i == 4 || i == 5);
        gtk_tree_view_append_column(ctx->tree, col);
    }
    GtkTreeSelection *sel = gtk_tree_view_get_selection(ctx->tree);
    g_signal_connect(sel, "changed", G_CALLBACK(on_parking_row_selected), ctx);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    int row = 0;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Guest ID:"), 0, row, 1, 1);
    ctx->spin_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->spin_guest), 1, row, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Plate:"), 2, row, 1, 1);
    ctx->entry_plate = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->entry_plate), 3, row, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Vehicle:"), 4, row, 1, 1);
    ctx->combo_vehicle = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Car");
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Motorcycle");
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Bus");
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "VIP");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_vehicle), 0);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->combo_vehicle), 5, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Preferred zone:"), 0, row, 1, 1);
    ctx->combo_preferred_zone = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "VIP");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "Standard");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "Moto");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "Bus");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_preferred_zone), (int)ZONE_STANDARD);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(ctx->combo_preferred_zone), 1, row, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 0);

    GtkWidget *brow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *b1 = gtk_button_new_with_label("Assign vehicle");
    GtkWidget *b2 = gtk_button_new_with_label("Release selected");
    GtkWidget *b3 = gtk_button_new_with_label("Save parking");
    style_btn(b1, "action-btn");
    style_btn(b2, "delete-btn");
    style_btn(b3, "action-btn");
    g_signal_connect(b1, "clicked", G_CALLBACK(on_parking_assign), ctx);
    g_signal_connect(b2, "clicked", G_CALLBACK(on_parking_release), ctx);
    g_signal_connect(b3, "clicked", G_CALLBACK(on_parking_save), ctx);
    gtk_box_pack_start(GTK_BOX(brow), b1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), brow, FALSE, FALSE, 0);

    parking_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   SEATING TAB (Module 7)
   ========================================================= */

static void seating_refresh_tree(SeatCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);
    DiningHall *h = &ctx->state->dining;

    for (int t = 0; t < h->table_count; t++) {
        DiningTable *tbl = &h->tables[t];
        for (int s = 0; s < tbl->capacity; s++) {
            SeatAssignment *se = &tbl->seats[s];
            GtkTreeIter it;
            gtk_list_store_append(ctx->store, &it);
            gtk_list_store_set(ctx->store, &it,
                0, se->seat_id,
                1, tbl->table_id,
                2, se->seat_number,
                3, se->guest_id,
                4, se->guest_id >= 0 ? se->guest_name : "-",
                5, seating_meal_name(se->meal),
                6, seating_diet_name(se->diet),
                7, seating_rsvp_name(se->rsvp),
                -1);
        }
    }

    MealSummary ms = seating_meal_summary(h);
    char buf[256];
    g_snprintf(buf, sizeof(buf),
        "Catering snapshot: Meat %d | Fish %d | Veg %d | Kids %d | Halal %d | Allergy %d",
        ms.meat_count, ms.fish_count, ms.vegetarian_count, ms.kids_count,
        ms.halal_count, ms.allergy_count);
    gtk_label_set_text(ctx->lbl_catering, buf);
}

static void on_seating_add_table(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    const char *nm = gtk_entry_get_text(ctx->entry_table_name);
    int cap = (int)gtk_spin_button_get_value(ctx->spin_table_capacity);
    int tt = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_table_type));
    if (tt < 0) tt = 0;
    int tid = seating_add_table(&ctx->state->dining, (TableType)tt, cap,
        (nm && nm[0]) ? nm : "Table");
    if (tid < 0)
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Could not add table.");
    else
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Table added.");
    seating_refresh_tree(ctx);
}

static void on_seating_assign(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    int table_id = (int)gtk_spin_button_get_value(ctx->spin_assign_table);
    int gid = (int)gtk_spin_button_get_value(ctx->spin_assign_guest);
    const char *nm = gtk_entry_get_text(ctx->entry_assign_name);
    int m = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_meal));
    int di = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_diet));
    if (m < 0) m = 0;
    if (di < 0) di = 0;
    int sid = seating_assign_guest(&ctx->state->dining, table_id, gid,
        (nm && nm[0]) ? nm : "Guest",
        (MealPreference)m, (DietaryRestriction)di);
    if (sid < 0)
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Assign failed (table full or guest already seated).");
    else
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Guest seated.");
    seating_refresh_tree(ctx);
}

static void on_seating_remove(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    int gid = (int)gtk_spin_button_get_value(ctx->spin_assign_guest);
    if (!seating_remove_guest(&ctx->state->dining, gid))
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Guest not found.");
    seating_refresh_tree(ctx);
}

static void on_seating_move(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    int gid = (int)gtk_spin_button_get_value(ctx->spin_move_guest);
    int newtid = (int)gtk_spin_button_get_value(ctx->spin_move_table);
    if (!seating_move_guest(&ctx->state->dining, gid, newtid))
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Move failed.");
    else
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Guest moved.");
    seating_refresh_tree(ctx);
}

static void on_seating_rsvp(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    int gid = (int)gtk_spin_button_get_value(ctx->spin_rsvp_guest);
    int r = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_rsvp));
    if (r < 0) r = 0;
    if (!seating_update_rsvp(&ctx->state->dining, gid, (RsvpStatus)r))
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Guest not seated.");
    seating_refresh_tree(ctx);
}

static void on_seating_save(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx*)d;
    seating_save(&ctx->state->dining, SEATING_FILE);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Seating", "Saved to data/seating.dat");
}

static GtkWidget* build_seating_tab(AppState *state, GtkWidget *main_win) {
    SeatCtx *ctx = g_new0(SeatCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    g_seat_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new("Seating (Module 7)");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    ctx->lbl_catering = GTK_LABEL(gtk_label_new(""));
    gtk_widget_set_halign(GTK_WIDGET(ctx->lbl_catering), GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(ctx->lbl_catering), FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(8,
        G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);

    const char *cols[] = {"Seat","Table","#","Guest","Name","Meal","Diet","RSVP"};
    for (int i = 0; i < 8; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", i, NULL);
        gtk_tree_view_append_column(ctx->tree, col);
    }
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Add table row */
    GtkWidget *g1 = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g1), 8);
    gtk_grid_attach(GTK_GRID(g1), gtk_label_new("New table name:"), 0, 0, 1, 1);
    ctx->entry_table_name = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(g1), GTK_WIDGET(ctx->entry_table_name), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g1), gtk_label_new("Capacity:"), 2, 0, 1, 1);
    ctx->spin_table_capacity = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 12, 1));
    gtk_spin_button_set_value(ctx->spin_table_capacity, 8);
    gtk_grid_attach(GTK_GRID(g1), GTK_WIDGET(ctx->spin_table_capacity), 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g1), gtk_label_new("Type:"), 4, 0, 1, 1);
    ctx->combo_table_type = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_table_type, "VIP");
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Family");
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Friends");
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Children");
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Staff");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_table_type), 0);
    gtk_grid_attach(GTK_GRID(g1), GTK_WIDGET(ctx->combo_table_type), 5, 0, 1, 1);
    GtkWidget *b_addt = gtk_button_new_with_label("Add table");
    style_btn(b_addt, "action-btn");
    g_signal_connect(b_addt, "clicked", G_CALLBACK(on_seating_add_table), ctx);
    gtk_grid_attach(GTK_GRID(g1), b_addt, 6, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), g1, FALSE, FALSE, 0);

    /* Assign row */
    GtkWidget *g2 = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g2), 8);
    gtk_grid_attach(GTK_GRID(g2), gtk_label_new("Table ID:"), 0, 0, 1, 1);
    ctx->spin_assign_table = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(g2), GTK_WIDGET(ctx->spin_assign_table), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g2), gtk_label_new("Guest ID:"), 2, 0, 1, 1);
    ctx->spin_assign_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(g2), GTK_WIDGET(ctx->spin_assign_guest), 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g2), gtk_label_new("Name:"), 4, 0, 1, 1);
    ctx->entry_assign_name = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(g2), GTK_WIDGET(ctx->entry_assign_name), 5, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g2), gtk_label_new("Meal:"), 0, 1, 1, 1);
    ctx->combo_meal = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_meal, "Meat");
    gtk_combo_box_text_append_text(ctx->combo_meal, "Fish");
    gtk_combo_box_text_append_text(ctx->combo_meal, "Vegetarian");
    gtk_combo_box_text_append_text(ctx->combo_meal, "Kids");
    gtk_combo_box_text_append_text(ctx->combo_meal, "Not set");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_meal), 0);
    gtk_grid_attach(GTK_GRID(g2), GTK_WIDGET(ctx->combo_meal), 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(g2), gtk_label_new("Diet:"), 2, 1, 1, 1);
    ctx->combo_diet = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_diet, "None");
    gtk_combo_box_text_append_text(ctx->combo_diet, "Halal");
    gtk_combo_box_text_append_text(ctx->combo_diet, "Kosher");
    gtk_combo_box_text_append_text(ctx->combo_diet, "Allergy");
    gtk_combo_box_text_append_text(ctx->combo_diet, "Diabetic");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_diet), 0);
    gtk_grid_attach(GTK_GRID(g2), GTK_WIDGET(ctx->combo_diet), 3, 1, 1, 1);
    GtkWidget *b_asg = gtk_button_new_with_label("Seat guest");
    GtkWidget *b_rm = gtk_button_new_with_label("Remove guest (by Guest ID above)");
    style_btn(b_asg, "action-btn");
    style_btn(b_rm, "delete-btn");
    g_signal_connect(b_asg, "clicked", G_CALLBACK(on_seating_assign), ctx);
    g_signal_connect(b_rm, "clicked", G_CALLBACK(on_seating_remove), ctx);
    gtk_grid_attach(GTK_GRID(g2), b_asg, 4, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(g2), b_rm, 5, 1, 2, 1);
    gtk_box_pack_start(GTK_BOX(vbox), g2, FALSE, FALSE, 0);

    /* Move + RSVP */
    GtkWidget *g3 = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g3), 8);
    gtk_grid_attach(GTK_GRID(g3), gtk_label_new("Move guest ID:"), 0, 0, 1, 1);
    ctx->spin_move_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(g3), GTK_WIDGET(ctx->spin_move_guest), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(g3), gtk_label_new("To table ID:"), 2, 0, 1, 1);
    ctx->spin_move_table = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(g3), GTK_WIDGET(ctx->spin_move_table), 3, 0, 1, 1);
    GtkWidget *b_mv = gtk_button_new_with_label("Move");
    style_btn(b_mv, "action-btn");
    g_signal_connect(b_mv, "clicked", G_CALLBACK(on_seating_move), ctx);
    gtk_grid_attach(GTK_GRID(g3), b_mv, 4, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(g3), gtk_label_new("RSVP guest ID:"), 0, 1, 1, 1);
    ctx->spin_rsvp_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 9999, 1));
    gtk_grid_attach(GTK_GRID(g3), GTK_WIDGET(ctx->spin_rsvp_guest), 1, 1, 1, 1);
    ctx->combo_rsvp = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_rsvp, "Pending");
    gtk_combo_box_text_append_text(ctx->combo_rsvp, "Confirmed");
    gtk_combo_box_text_append_text(ctx->combo_rsvp, "Declined");
    gtk_combo_box_text_append_text(ctx->combo_rsvp, "No-show");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_rsvp), 0);
    gtk_grid_attach(GTK_GRID(g3), GTK_WIDGET(ctx->combo_rsvp), 2, 1, 1, 1);
    GtkWidget *b_rsvp = gtk_button_new_with_label("Set RSVP");
    style_btn(b_rsvp, "action-btn");
    g_signal_connect(b_rsvp, "clicked", G_CALLBACK(on_seating_rsvp), ctx);
    gtk_grid_attach(GTK_GRID(g3), b_rsvp, 3, 1, 1, 1);
    GtkWidget *b_sv = gtk_button_new_with_label("Save seating");
    style_btn(b_sv, "action-btn");
    g_signal_connect(b_sv, "clicked", G_CALLBACK(on_seating_save), ctx);
    gtk_grid_attach(GTK_GRID(g3), b_sv, 4, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), g3, FALSE, FALSE, 0);

    seating_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   SCHEDULE TAB (Module 8)
   ========================================================= */

static time_t sched_today_at(int hour, int minute) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (!tm_info) return now;
    struct tm t = *tm_info;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = 0;
    return mktime(&t);
}

static void schedule_refresh_tree(SchedCtx *ctx) {
    if (!ctx) return;
    gtk_list_store_clear(ctx->store);
    WeddingSchedule *sched = &ctx->state->schedule;
    EventNode *cur = sched->head;
    char s1[16], s2[16];

    while (cur) {
        schedule_format_hhmm(cur->scheduled_start, s1, sizeof(s1));
        schedule_format_hhmm(cur->scheduled_end, s2, sizeof(s2));
        GtkTreeIter it;
        gtk_list_store_append(ctx->store, &it);
        gtk_list_store_set(ctx->store, &it,
            0, cur->event_id,
            1, s1,
            2, s2,
            3, cur->title,
            4, schedule_category_name(cur->category),
            5, schedule_status_name(cur->status),
            6, cur->duration_minutes,
            -1);
        cur = cur->next;
    }

    long mn = schedule_minutes_to_next(sched);
    char buf[128];
    if (mn < 0)
        g_snprintf(buf, sizeof(buf), "Next event: (none scheduled)");
    else
        g_snprintf(buf, sizeof(buf), "Approx. minutes to next upcoming event: %ld", mn);
    gtk_label_set_text(ctx->lbl_countdown, buf);
}

static void on_sched_row_selected(GtkTreeSelection *sel, gpointer d) {
    SchedCtx *ctx = (SchedCtx*)d;
    GtkTreeIter it;
    GtkTreeModel *model;
    if (!gtk_tree_selection_get_selected(sel, &model, &it)) {
        ctx->selected_event_id = -1;
        return;
    }
    int eid = -1;
    gtk_tree_model_get(model, &it, 0, &eid, -1);
    ctx->selected_event_id = eid;
}

static void on_sched_add(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    const char *title = gtk_entry_get_text(ctx->entry_title);
    if (!title || !title[0]) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Enter an event title.");
        return;
    }
    int h = (int)gtk_spin_button_get_value(ctx->spin_start_h);
    int m = (int)gtk_spin_button_get_value(ctx->spin_start_m);
    int dur = (int)gtk_spin_button_get_value(ctx->spin_dur);
    int c = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_category));
    int p = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_priority));
    if (c < 0) c = 0;
    if (p < 0) p = 0;
    time_t st = sched_today_at(h, m);
    const char *resp = gtk_entry_get_text(ctx->entry_resp);
    const char *loc = gtk_entry_get_text(ctx->entry_loc);

    schedule_add_event(&ctx->state->schedule, title, "",
        (EventCategory)c, (EventPriority)p, st, dur,
        resp && resp[0] ? resp : "MC",
        loc && loc[0] ? loc : "Venue");
    schedule_refresh_tree(ctx);
}

static void on_sched_start(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    if (ctx->selected_event_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Select an event.");
        return;
    }
    schedule_start_event(&ctx->state->schedule, ctx->selected_event_id);
    schedule_refresh_tree(ctx);
}

static void on_sched_end(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    if (ctx->selected_event_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Select an event.");
        return;
    }
    schedule_end_event(&ctx->state->schedule, ctx->selected_event_id);
    schedule_refresh_tree(ctx);
}

static void on_sched_delay(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    if (ctx->selected_event_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Select an event.");
        return;
    }
    int dm = (int)gtk_spin_button_get_value(ctx->spin_delay);
    schedule_delay_event(&ctx->state->schedule, ctx->selected_event_id, dm);
    schedule_refresh_tree(ctx);
}

static void on_sched_remove(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    if (ctx->selected_event_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Select an event.");
        return;
    }
    if (!ui_confirm_dialog(GTK_WINDOW(ctx->main_win), "Remove this event from the schedule?"))
        return;
    schedule_remove_event(&ctx->state->schedule, ctx->selected_event_id);
    ctx->selected_event_id = -1;
    schedule_refresh_tree(ctx);
}

static void on_sched_save(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx*)d;
    schedule_save(&ctx->state->schedule, SCHEDULE_FILE);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "Saved to data/schedule.dat");
}

static GtkWidget* build_schedule_tab(AppState *state, GtkWidget *main_win) {
    SchedCtx *ctx = g_new0(SchedCtx, 1);
    ctx->state = state;
    ctx->main_win = main_win;
    ctx->selected_event_id = -1;
    g_sched_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new("Schedule (Module 8) - times use today's date on this PC");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    ctx->lbl_countdown = GTK_LABEL(gtk_label_new(""));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(ctx->lbl_countdown), FALSE, FALSE, 0);

    ctx->store = gtk_list_store_new(7,
        G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    g_object_unref(ctx->store);
    const char *cols[] = {"ID","Start","End","Title","Category","Status","Min"};
    for (int i = 0; i < 7; i++) {
        GtkCellRenderer *r = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(cols[i], r, "text", i, NULL);
        gtk_tree_view_column_set_expand(col, i == 3);
        gtk_tree_view_append_column(ctx->tree, col);
    }
    GtkTreeSelection *sel = gtk_tree_view_get_selection(ctx->tree);
    g_signal_connect(sel, "changed", G_CALLBACK(on_sched_row_selected), ctx);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(g), 8);
    gtk_grid_set_row_spacing(GTK_GRID(g), 6);
    int r = 0;
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Title:"), 0, r, 1, 1);
    ctx->entry_title = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->entry_title), 1, r, 3, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Category:"), 4, r, 1, 1);
    ctx->combo_category = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_category, "Ceremony");
    gtk_combo_box_text_append_text(ctx->combo_category, "Reception");
    gtk_combo_box_text_append_text(ctx->combo_category, "Dinner");
    gtk_combo_box_text_append_text(ctx->combo_category, "Speech");
    gtk_combo_box_text_append_text(ctx->combo_category, "Entertainment");
    gtk_combo_box_text_append_text(ctx->combo_category, "Photo");
    gtk_combo_box_text_append_text(ctx->combo_category, "Logistics");
    gtk_combo_box_text_append_text(ctx->combo_category, "Break");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_category), 0);
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->combo_category), 5, r, 1, 1);
    r++;
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Priority:"), 0, r, 1, 1);
    ctx->combo_priority = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_priority, "Critical");
    gtk_combo_box_text_append_text(ctx->combo_priority, "High");
    gtk_combo_box_text_append_text(ctx->combo_priority, "Normal");
    gtk_combo_box_text_append_text(ctx->combo_priority, "Optional");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_priority), 2);
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->combo_priority), 1, r, 1, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Start (H:M):"), 2, r, 1, 1);
    ctx->spin_start_h = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 23, 1));
    gtk_spin_button_set_value(ctx->spin_start_h, 12);
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->spin_start_h), 3, r, 1, 1);
    ctx->spin_start_m = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 59, 1));
    gtk_spin_button_set_value(ctx->spin_start_m, 0);
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->spin_start_m), 4, r, 1, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Dur (min):"), 5, r, 1, 1);
    ctx->spin_dur = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 600, 5));
    gtk_spin_button_set_value(ctx->spin_dur, 30);
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->spin_dur), 6, r, 1, 1);
    r++;
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Responsible:"), 0, r, 1, 1);
    ctx->entry_resp = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->entry_resp), 1, r, 2, 1);
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Location:"), 3, r, 1, 1);
    ctx->entry_loc = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->entry_loc), 4, r, 3, 1);
    r++;
    gtk_grid_attach(GTK_GRID(g), gtk_label_new("Delay sel. (min):"), 0, r, 1, 1);
    ctx->spin_delay = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 180, 5));
    gtk_grid_attach(GTK_GRID(g), GTK_WIDGET(ctx->spin_delay), 1, r, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), g, FALSE, FALSE, 0);

    GtkWidget *brow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *b_add = gtk_button_new_with_label("Add event");
    GtkWidget *b_st = gtk_button_new_with_label("Start");
    GtkWidget *b_en = gtk_button_new_with_label("End");
    GtkWidget *b_dl = gtk_button_new_with_label("Delay");
    GtkWidget *b_rm = gtk_button_new_with_label("Remove");
    GtkWidget *b_sv = gtk_button_new_with_label("Save schedule");
    style_btn(b_add, "action-btn");
    style_btn(b_st, "action-btn");
    style_btn(b_en, "action-btn");
    style_btn(b_dl, "action-btn");
    style_btn(b_rm, "delete-btn");
    style_btn(b_sv, "action-btn");
    g_signal_connect(b_add, "clicked", G_CALLBACK(on_sched_add), ctx);
    g_signal_connect(b_st, "clicked", G_CALLBACK(on_sched_start), ctx);
    g_signal_connect(b_en, "clicked", G_CALLBACK(on_sched_end), ctx);
    g_signal_connect(b_dl, "clicked", G_CALLBACK(on_sched_delay), ctx);
    g_signal_connect(b_rm, "clicked", G_CALLBACK(on_sched_remove), ctx);
    g_signal_connect(b_sv, "clicked", G_CALLBACK(on_sched_save), ctx);
    gtk_box_pack_start(GTK_BOX(brow), b_add, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b_st, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b_en, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b_dl, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b_rm, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(brow), b_sv, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), brow, FALSE, FALSE, 0);

    schedule_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   STATS TAB
   ========================================================= */

static GtkWidget* make_stat_card(const char *number, const char *label, GtkLabel **out_num) {
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(frame), "stat-box");

    GtkWidget *vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(vb, 16);
    gtk_widget_set_margin_end(vb, 16);
    gtk_widget_set_margin_top(vb, 16);
    gtk_widget_set_margin_bottom(vb, 16);

    GtkWidget *n = gtk_label_new(number);
    gtk_style_context_add_class(gtk_widget_get_style_context(n), "stat-num");
    gtk_widget_set_halign(n, GTK_ALIGN_CENTER);

    if (out_num) *out_num = GTK_LABEL(n);

    GtkWidget *l = gtk_label_new(label);
    gtk_style_context_add_class(gtk_widget_get_style_context(l), "stat-label");
    gtk_widget_set_halign(l, GTK_ALIGN_CENTER);

    gtk_box_pack_start(GTK_BOX(vb), n, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vb), l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(frame), vb);
    return frame;
}

static GtkWidget* build_stats_tab(AppState *state) {
    StatsCtx *ctx = g_new0(StatsCtx, 1);
    ctx->state = state;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(vbox, 24);
    gtk_widget_set_margin_end(vbox, 24);
    gtk_widget_set_margin_top(vbox, 24);
    gtk_widget_set_margin_bottom(vbox, 24);

    GtkWidget *lbl = gtk_label_new("Dashboard - Live Statistics");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title-lbl");
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    GtkWidget *cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    ctx->lbl_cats = NULL;
    ctx->lbl_guests = NULL;
    ctx->lbl_gifts = NULL;
    ctx->lbl_total = NULL;

    gtk_box_pack_start(GTK_BOX(cards),
        make_stat_card("0", "Categories", &ctx->lbl_cats), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(cards),
        make_stat_card("0", "Registered Guests", &ctx->lbl_guests), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(cards),
        make_stat_card("0", "Gifts", &ctx->lbl_gifts), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(cards),
        make_stat_card("0 FCFA", "Total Gift Value", &ctx->lbl_total), TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), cards, FALSE, FALSE, 0);

    GtkWidget *note = gtk_label_new(
        "Statistics update automatically after changes in other tabs.");
    gtk_widget_set_halign(note, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), note, FALSE, FALSE, 0);

    g_stats_ctx = ctx;
    stats_refresh(ctx);

    return vbox;
}

/* =========================================================
   Main window callbacks
   ========================================================= */

static void on_main_destroy(GtkWidget *w, gpointer d) {
    (void)w;
    AppState *state = (AppState*)d;
    gift_save(state->gifts, state->gift_count, GIFTS_FILE);
    parking_save(&state->parking, PARKING_FILE);
    seating_save(&state->dining, SEATING_FILE);
    schedule_save(&state->schedule, SCHEDULE_FILE);
    category_free_all(&state->cat_head);

    if (g_cat_ctx) g_free(g_cat_ctx);
    if (g_person_ctx) g_free(g_person_ctx);
    if (g_prio_ctx) g_free(g_prio_ctx);
    if (g_gift_ctx) g_free(g_gift_ctx);
    if (g_park_ctx) g_free(g_park_ctx);
    if (g_seat_ctx) g_free(g_seat_ctx);
    if (g_sched_ctx) g_free(g_sched_ctx);
    if (g_stats_ctx) g_free(g_stats_ctx);

    gtk_main_quit();
}

/* =========================================================
   GTK entry point
   ========================================================= */

void ui_gtk_run(int *argc, char ***argv, AppState *state) {
    gtk_init(argc, argv);
    ui_apply_css();

    /* Load persisted gifts before building Gift tab */
    gift_load(state->gifts, &state->gift_count, GIFTS_FILE);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), APP_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(win), APP_W, APP_H);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    g_signal_connect(win, "destroy", G_CALLBACK(on_main_destroy), state);

    GtkWidget *hbar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(hbar), "WIGMS");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(hbar), "Wedding Invitation & Gift Management System");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(hbar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(win), hbar);

    GtkWidget *nb = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(nb), GTK_POS_TOP);

    GtkWidget *tabs[8];
    tabs[0] = build_category_tab(state, win);
    tabs[1] = build_person_tab(state, win);
    tabs[2] = build_priority_tab(state, win);
    tabs[3] = build_gift_tab(state, win);
    tabs[4] = build_parking_tab(state, win);
    tabs[5] = build_seating_tab(state, win);
    tabs[6] = build_schedule_tab(state, win);
    tabs[7] = build_stats_tab(state);

    const char *tab_labels[] = {
        "Categories", "Persons", "Priority", "Gifts",
        "Parking", "Seating", "Schedule", "Dashboard"
    };
    for (int i = 0; i < 8; i++) {
        GtkWidget *tab_lbl = gtk_label_new(tab_labels[i]);
        gtk_notebook_append_page(GTK_NOTEBOOK(nb), tabs[i], tab_lbl);
    }

    gtk_container_add(GTK_CONTAINER(win), nb);
    gtk_widget_show_all(win);

    /* Ensure totals are correct even if gifts file existed */
    if (g_gift_ctx) gift_refresh(g_gift_ctx);
    if (g_park_ctx) parking_refresh_tree(g_park_ctx);
    if (g_seat_ctx) seating_refresh_tree(g_seat_ctx);
    if (g_sched_ctx) schedule_refresh_tree(g_sched_ctx);
    if (g_stats_ctx) stats_refresh(g_stats_ctx);

    gtk_main();
}

