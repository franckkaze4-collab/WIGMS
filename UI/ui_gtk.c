/*
 * =========================================================
 *  MODULE 5 - GTK Graphical User Interface
 *  ui_gtk.c : GTK3 implementation with admin/guest mode
 * =========================================================
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
#define CATEGORIES_FILE "data/categories.dat"
#define APP_TITLE  "WIGMS - Wedding Invitation & Gift Management"
#define APP_W      1200
#define APP_H      800

/* =========================================================
   Per-tab contexts
   ========================================================= */

typedef struct {
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *search_entry;
    GtkEntry      *entry_code;
    GtkSpinButton *spin_guests;
    GtkEntry      *g_name[4];
    GtkSpinButton *g_age[4];
    GtkEntry      *g_class[4];
    GtkComboBoxText *g_side[4];
    int            selected_id;
} CatCtx;

typedef struct {
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *entry_name;
    GtkSpinButton *spin_age;
    GtkEntry      *entry_class;
    GtkComboBoxText *combo_side;
    GtkWidget     *lbl_status;
    GtkLabel      *lbl_total;
    GtkLabel      *lbl_le;
    GtkLabel      *lbl_la;
    GtkLabel      *lbl_vip;
    int            selected_cat_id;
    int            selected_guest_idx;
} PersonCtx;

typedef struct {
    GtkWidget    *main_win;
    GtkListStore *store;
    GtkTreeView  *tree;
    GtkLabel     *lbl_status;
} PrioCtx;

typedef struct {
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *entry_name;
    GtkSpinButton *spin_value;
    GtkSpinButton *spin_guest;
    GtkLabel      *lbl_total;
    GtkLabel      *lbl_count;
    GtkLabel      *lbl_avg;
    GtkLabel      *lbl_highest;
    int            selected_id;
} GiftCtx;

typedef struct {
    GtkWidget          *main_win;
    GtkListStore       *store;
    GtkTreeView        *tree;
    GtkComboBoxText    *combo_zone_filter;
    GtkSpinButton      *spin_guest;
    GtkEntry           *entry_plate;
    GtkComboBoxText    *combo_vehicle;
    GtkComboBoxText    *combo_preferred_zone;
    GtkLabel           *lbl_stats;
    GtkWidget          *parking_modal;
    GtkWidget          *parking_drawing;
    GtkEntry           *parking_search_guest;
    GtkEntry           *parking_search_spot;
    int                 selected_spot_id;
} ParkCtx;

typedef struct {
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
    GtkWidget       *seating_modal;
    GtkWidget       *seating_drawing;
    GtkEntry        *seating_search_guest;
    GtkEntry        *seating_search_table;
} SeatCtx;

typedef struct {
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

typedef struct {
    GtkWidget     *main_win;
    GtkLabel      *lbl_cats;
    GtkLabel      *lbl_guests;
    GtkLabel      *lbl_gifts;
    GtkLabel      *lbl_total;
} StatsCtx;

typedef struct {
    GtkWidget     *main_win;
    GtkLabel      *lbl_cats;
    GtkLabel      *lbl_guests;
    GtkLabel      *lbl_gifts;
    GtkLabel      *lbl_total;
} HomeCtx;

/* Single instance contexts so callbacks can refresh other tabs */
static HomeCtx   *g_home_ctx   = NULL;
static CatCtx    *g_cat_ctx    = NULL;
static PersonCtx *g_person_ctx = NULL;
static PrioCtx   *g_prio_ctx   = NULL;
static GiftCtx   *g_gift_ctx   = NULL;
static StatsCtx  *g_stats_ctx  = NULL;
static ParkCtx   *g_park_ctx   = NULL;
static SeatCtx   *g_seat_ctx   = NULL;
static SchedCtx  *g_sched_ctx  = NULL;
static gboolean   g_admin_mode = TRUE;
static GtkWidget *g_notebook   = NULL;

/* Forward declarations */
static void switch_to_tab(int tab_index);
static void on_toggle_admin(GtkWidget *w, gpointer d);
static void on_toggle_guest(GtkWidget *w, gpointer d);
static void on_nav_home(GtkWidget *w, gpointer d);
static void on_nav_categories(GtkWidget *w, gpointer d);
static void on_nav_persons(GtkWidget *w, gpointer d);
static void on_nav_priority(GtkWidget *w, gpointer d);
static void on_nav_gifts(GtkWidget *w, gpointer d);
static void on_nav_parking(GtkWidget *w, gpointer d);
static void on_nav_seating(GtkWidget *w, gpointer d);
static void on_nav_schedule(GtkWidget *w, gpointer d);
static void on_nav_dashboard(GtkWidget *w, gpointer d);

/* =========================================================
   CSS Styling
   ========================================================= */
static const char *APP_CSS =
    "window { background: #6f6bd6; }\n"
    ".app-outer { padding: 22px; }\n"
    ".page-shell { background: rgba(255,255,255,0.96); border-radius: 18px; padding: 18px; }\n"
    ".page-title { font-size: 22px; font-weight: 700; color: #1f2937; }\n"
    ".page-subtitle { color: #6b7280; }\n"
    ".divider { background: #ef4444; min-height: 2px; }\n"
    ".pill-row { margin-top: 10px; margin-bottom: 10px; }\n"
    ".pill { border-radius: 12px; padding: 10px 16px; background: #ffffff; border: 1px solid #e5e7eb; }\n"
    ".pill.active { background: #6c63ff; color: #ffffff; border-color: #6c63ff; }\n"
    ".pill.secondary.active { background: #111827; color: #ffffff; border-color: #111827; }\n"
    ".nav-pill { border-radius: 10px; padding: 10px 14px; background: #ffffff; border: 1px solid #e5e7eb; }\n"
    ".nav-pill.active { background: #6c63ff; color: #ffffff; border-color: #6c63ff; }\n"
    ".card { background: #ffffff; border-radius: 14px; padding: 16px; border: 1px solid #eef2f7; }\n"
    ".card-title { font-weight: 700; color: #374151; }\n"
    ".btn-primary { background: #6c63ff; color: #ffffff; border-radius: 10px; padding: 10px 16px; }\n"
    ".btn-danger { background: #ef4444; color: #ffffff; border-radius: 10px; padding: 10px 16px; }\n"
    ".btn-muted { background: #eef2ff; color: #4f46e5; border-radius: 10px; padding: 10px 16px; }\n"
    "entry, spinbutton, combobox { border-radius: 10px; padding: 10px; }\n"
    "treeview { font-size: 13px; }\n"
    "notebook tab { padding: 0px; }\n"
    "notebook header { background: transparent; }\n";

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
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), css_class);
}

static void add_class(GtkWidget *w, const char *css_class) {
    if (!w || !css_class) return;
    gtk_style_context_add_class(gtk_widget_get_style_context(w), css_class);
}

typedef struct {
    GtkWidget *shell;
    GtkWidget *content;
    GtkWidget *admin_btn;
    GtkWidget *guest_btn;
    GtkWidget *nav_buttons[9];
} PageChrome;

static void chrome_set_active(PageChrome *ch, int active_tab) {
    if (!ch) return;
    for (int i = 0; i < 9; i++) {
        if (!ch->nav_buttons[i]) continue;
        if (i == active_tab) add_class(ch->nav_buttons[i], "active");
        else gtk_style_context_remove_class(gtk_widget_get_style_context(ch->nav_buttons[i]), "active");
    }
}

static PageChrome build_chrome(const char *title, const char *subtitle, int active_tab) {
    PageChrome ch;
    memset(&ch, 0, sizeof(ch));

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    add_class(outer, "app-outer");

    GtkWidget *shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    add_class(shell, "page-shell");
    gtk_box_pack_start(GTK_BOX(outer), shell, TRUE, TRUE, 0);

    /* Header */
    GtkWidget *hdr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_pack_start(GTK_BOX(shell), hdr, FALSE, FALSE, 0);

    GtkWidget *t = gtk_label_new(title ? title : "");
    gtk_label_set_xalign(GTK_LABEL(t), 0.0);
    add_class(t, "page-title");
    gtk_box_pack_start(GTK_BOX(hdr), t, FALSE, FALSE, 0);

    if (subtitle && subtitle[0]) {
        GtkWidget *st = gtk_label_new(subtitle);
        gtk_label_set_xalign(GTK_LABEL(st), 0.0);
        add_class(st, "page-subtitle");
        gtk_box_pack_start(GTK_BOX(hdr), st, FALSE, FALSE, 0);
    }

    GtkWidget *div = gtk_event_box_new();
    add_class(div, "divider");
    gtk_box_pack_start(GTK_BOX(shell), div, FALSE, FALSE, 0);

    /* Mode toggle row (Administrator / Guest) */
    GtkWidget *mode_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    add_class(mode_row, "pill-row");
    gtk_box_pack_start(GTK_BOX(shell), mode_row, FALSE, FALSE, 0);

    GtkWidget *admin_btn = gtk_button_new_with_label("🔑  Administrator");
    GtkWidget *guest_btn = gtk_button_new_with_label("👤  Guest");
    add_class(admin_btn, "pill");
    add_class(guest_btn, "pill");
    gtk_box_pack_start(GTK_BOX(mode_row), admin_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mode_row), guest_btn, FALSE, FALSE, 0);

    /* Default active pill based on current mode */
    if (g_admin_mode) add_class(admin_btn, "active");
    else add_class(guest_btn, "active");

    g_signal_connect(admin_btn, "clicked", G_CALLBACK(on_toggle_admin), guest_btn);
    g_signal_connect(guest_btn, "clicked", G_CALLBACK(on_toggle_guest), admin_btn);

    /* Nav row */
    GtkWidget *nav = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(shell), nav, FALSE, FALSE, 0);

    const char *labels[9] = {"Home","Category","Person","Priority","Gift","Parking","Schedule","Seating","Dashboard"};
    void (*handlers[9])(GtkWidget*,gpointer) = {
        on_nav_home, on_nav_categories, on_nav_persons, on_nav_priority, on_nav_gifts,
        on_nav_parking, on_nav_schedule, on_nav_seating, on_nav_dashboard
    };
    for (int i = 0; i < 9; i++) {
        GtkWidget *b = gtk_button_new_with_label(labels[i]);
        add_class(b, "nav-pill");
        gtk_box_pack_start(GTK_BOX(nav), b, FALSE, FALSE, 0);
        g_signal_connect(b, "clicked", G_CALLBACK(handlers[i]), NULL);
        ch.nav_buttons[i] = b;
    }

    /* Match HTML behavior: admin-only nav items are disabled in Guest mode */
    if (!g_admin_mode) {
        int admin_only[] = {1, 3, 4, 8};
        for (size_t k = 0; k < sizeof(admin_only) / sizeof(admin_only[0]); k++) {
            int idx = admin_only[k];
            if (ch.nav_buttons[idx]) gtk_widget_set_sensitive(ch.nav_buttons[idx], FALSE);
        }
    }

    chrome_set_active(&ch, active_tab);

    /* Content container */
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_box_pack_start(GTK_BOX(shell), content, TRUE, TRUE, 0);

    ch.shell = outer;
    ch.content = content;
    ch.admin_btn = admin_btn;
    ch.guest_btn = guest_btn;
    return ch;
}

/* =========================================================
   Dialog helpers
   ========================================================= */
void ui_show_info_dialog(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *dlg = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(dlg), title);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

void ui_show_error_dialog(GtkWindow *parent, const char *title, const char *msg) {
    GtkWidget *dlg = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(dlg), title);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

gboolean ui_confirm_dialog(GtkWindow *parent, const char *question) {
    GtkWidget *dlg = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", question);
    gint resp = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    return (resp == GTK_RESPONSE_YES);
}

/* =========================================================
   Navigation helpers
   ========================================================= */
static void switch_to_tab(int tab_index) {
    if (g_notebook) {
        /* Guest mode: block admin-only modules like the HTML simulation */
        if (!g_admin_mode && (tab_index == 1 || tab_index == 3 || tab_index == 4 || tab_index == 8)) {
            GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(g_notebook));
            ui_show_info_dialog(parent, "Guest Mode", "This module is available in Administrator mode only.");
            return;
        }
        gtk_notebook_set_current_page(GTK_NOTEBOOK(g_notebook), tab_index);
    }
}

static void on_nav_home(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(0);
}

static void on_nav_categories(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(1);
}

static void on_nav_persons(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(2);
}

static void on_nav_priority(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(3);
}

static void on_nav_gifts(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(4);
}

static void on_nav_parking(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(5);
}

static void on_nav_seating(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(6);
}

static void on_nav_schedule(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(7);
}

static void on_nav_dashboard(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(8);
}

/* =========================================================
   Admin/Guest mode toggle
   ========================================================= */
static void on_admin_categories_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(1);
}

static void on_admin_gifts_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(4);
}

static void on_admin_priority_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(3);
}

static void on_guest_register_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(2);
}

static void on_guest_seating_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(6);
}

static void on_guest_parking_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(5);
}

static void on_guest_schedule_btn(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    switch_to_tab(7);
}

static void on_toggle_admin(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *guest_btn = GTK_WIDGET(d);
    g_admin_mode = TRUE;
    gtk_style_context_add_class(gtk_widget_get_style_context(w), "suggested-action");
    gtk_style_context_remove_class(gtk_widget_get_style_context(guest_btn), "suggested-action");

    /* Show admin tabs (Categories=1, Priority=3, Gifts=4, Dashboard=8) */
    if (g_notebook) {
        GtkWidget *cat_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 1);
        GtkWidget *prio_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 3);
        GtkWidget *gift_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 4);
        GtkWidget *dash_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 8);
        if (cat_tab) gtk_widget_show(cat_tab);
        if (prio_tab) gtk_widget_show(prio_tab);
        if (gift_tab) gtk_widget_show(gift_tab);
        if (dash_tab) gtk_widget_show(dash_tab);
    }
}

static void on_toggle_guest(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *admin_btn = GTK_WIDGET(d);
    g_admin_mode = FALSE;
    gtk_style_context_add_class(gtk_widget_get_style_context(w), "suggested-action");
    gtk_style_context_remove_class(gtk_widget_get_style_context(admin_btn), "suggested-action");

    /* Hide admin tabs (Categories=1, Priority=3, Gifts=4, Dashboard=8) */
    if (g_notebook) {
        GtkWidget *cat_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 1);
        GtkWidget *prio_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 3);
        GtkWidget *gift_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 4);
        GtkWidget *dash_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 8);
        if (cat_tab) gtk_widget_hide(cat_tab);
        if (prio_tab) gtk_widget_hide(prio_tab);
        if (gift_tab) gtk_widget_hide(gift_tab);
        if (dash_tab) gtk_widget_hide(dash_tab);
    }
}

/* =========================================================
   HOME TAB
   ========================================================= */
static GtkWidget *build_home_tab(void) {
    HomeCtx *ctx = g_malloc0(sizeof(HomeCtx));
    g_home_ctx = ctx;

    PageChrome ch = build_chrome(
        "WIGMS - Wedding Invitation & Gift Management",
        "Complete wedding management system. Switch between Administrator and Guest access modes.",
        0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 16);
    gtk_box_pack_start(GTK_BOX(ch.content), grid, FALSE, FALSE, 0);

    /* Admin card */
    GtkWidget *admin_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    add_class(admin_card, "card");
    gtk_grid_attach(GTK_GRID(grid), admin_card, 0, 0, 1, 1);

    GtkWidget *admin_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(admin_card), admin_hdr, FALSE, FALSE, 0);
    GtkWidget *admin_pill = gtk_button_new_with_label("🔑  Administrator");
    add_class(admin_pill, "pill");
    add_class(admin_pill, "active");
    gtk_box_pack_start(GTK_BOX(admin_hdr), admin_pill, FALSE, FALSE, 0);

    GtkWidget *admin_title = gtk_label_new("Administrator Access");
    gtk_label_set_xalign(GTK_LABEL(admin_title), 0.0);
    add_class(admin_title, "card-title");
    gtk_box_pack_start(GTK_BOX(admin_card), admin_title, FALSE, FALSE, 0);

    GtkWidget *admin_banner = gtk_label_new("Full access to all modules: category management, guest registration, priority sorting, gift tracking, parking management, seating arrangements, and event scheduling.");
    gtk_label_set_xalign(GTK_LABEL(admin_banner), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(admin_banner), TRUE);
    gtk_box_pack_start(GTK_BOX(admin_card), admin_banner, FALSE, FALSE, 0);

    GtkWidget *admin_btnrow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(admin_card), admin_btnrow, FALSE, FALSE, 0);
    GtkWidget *ab1 = gtk_button_new_with_label("CATEGORIES");
    GtkWidget *ab2 = gtk_button_new_with_label("TRACK GIFTS");
    GtkWidget *ab3 = gtk_button_new_with_label("SET PRIORITIES");
    add_class(ab1, "btn-primary");
    add_class(ab2, "btn-primary");
    add_class(ab3, "btn-primary");
    gtk_box_pack_start(GTK_BOX(admin_btnrow), ab1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(admin_btnrow), ab2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(admin_btnrow), ab3, TRUE, TRUE, 0);
    g_signal_connect(ab1, "clicked", G_CALLBACK(on_admin_categories_btn), NULL);
    g_signal_connect(ab2, "clicked", G_CALLBACK(on_admin_gifts_btn), NULL);
    g_signal_connect(ab3, "clicked", G_CALLBACK(on_admin_priority_btn), NULL);

    /* Guest card */
    GtkWidget *guest_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    add_class(guest_card, "card");
    gtk_grid_attach(GTK_GRID(grid), guest_card, 1, 0, 1, 1);

    GtkWidget *guest_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(guest_card), guest_hdr, FALSE, FALSE, 0);
    GtkWidget *guest_pill = gtk_button_new_with_label("👤  Guest");
    add_class(guest_pill, "pill");
    gtk_box_pack_start(GTK_BOX(guest_hdr), guest_pill, FALSE, FALSE, 0);

    GtkWidget *guest_title = gtk_label_new("Guest Access");
    gtk_label_set_xalign(GTK_LABEL(guest_title), 0.0);
    add_class(guest_title, "card-title");
    gtk_box_pack_start(GTK_BOX(guest_card), guest_title, FALSE, FALSE, 0);

    GtkWidget *guest_banner = gtk_label_new("Guests can register, view their seating assignment, locate their parking zone, and check the event schedule. Simple, visual interfaces for easy navigation.");
    gtk_label_set_xalign(GTK_LABEL(guest_banner), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(guest_banner), TRUE);
    gtk_box_pack_start(GTK_BOX(guest_card), guest_banner, FALSE, FALSE, 0);

    GtkWidget *guest_btnrow1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *guest_btnrow2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(guest_card), guest_btnrow1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(guest_card), guest_btnrow2, FALSE, FALSE, 0);
    GtkWidget *gb1 = gtk_button_new_with_label("GUEST REGISTRING NOW");
    GtkWidget *gb2 = gtk_button_new_with_label("VIEW SEATING");
    GtkWidget *gb3 = gtk_button_new_with_label("FIND PARKING POSITION");
    GtkWidget *gb4 = gtk_button_new_with_label("SEE SCHEDULE");
    add_class(gb1, "btn-primary");
    add_class(gb2, "btn-primary");
    add_class(gb3, "btn-primary");
    add_class(gb4, "btn-primary");
    gtk_box_pack_start(GTK_BOX(guest_btnrow1), gb1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(guest_btnrow1), gb2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(guest_btnrow2), gb3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(guest_btnrow2), gb4, TRUE, TRUE, 0);
    g_signal_connect(gb1, "clicked", G_CALLBACK(on_guest_register_btn), NULL);
    g_signal_connect(gb2, "clicked", G_CALLBACK(on_guest_seating_btn), NULL);
    g_signal_connect(gb3, "clicked", G_CALLBACK(on_guest_parking_btn), NULL);
    g_signal_connect(gb4, "clicked", G_CALLBACK(on_guest_schedule_btn), NULL);

    return ch.shell;
}

/* =========================================================
   Forward declarations for callbacks
   ========================================================= */
static void cat_refresh_tree(CatCtx *ctx);
static void person_refresh_tree(PersonCtx *ctx);
static void prio_refresh_tree(PrioCtx *ctx);
static void gift_refresh(GiftCtx *ctx);
static void gift_refresh_stats(GiftCtx *ctx);
static void person_refresh_stats(PersonCtx *ctx);

/* =========================================================
   CATEGORY TAB
   ========================================================= */
static void cat_refresh_tree(CatCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
}

static void on_cat_select(GtkTreeSelection *sel, gpointer d) {
    CatCtx *ctx = (CatCtx *)d;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        ctx->selected_id = id;

        Category *head = NULL;
        category_load(&head, CATEGORIES_FILE);
        Category *c = category_find_by_id(head, id);
        if (c) {
            gtk_entry_set_text(ctx->entry_code, c->code);
            gtk_spin_button_set_value(ctx->spin_guests, c->guest_count);
        }
        category_free(head);
    }
}

static void on_cat_add(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx *)d;
    const char *code = gtk_entry_get_text(ctx->entry_code);
    int guests = gtk_spin_button_get_value_as_int(ctx->spin_guests);

    if (!code || strlen(code) == 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Code required");
        return;
    }

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    Category *new_cat = g_malloc0(sizeof(Category));
    new_cat->id = category_get_next_id(head);
    strncpy(new_cat->code, code, sizeof(new_cat->code) - 1);
    new_cat->guest_count = guests;
    new_cat->next = head;

    category_save(new_cat, CATEGORIES_FILE);
    category_free(new_cat);

    cat_refresh_tree(ctx);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Category added");
}

static void on_cat_update(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx *)d;

    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "No category selected");
        return;
    }

    const char *code = gtk_entry_get_text(ctx->entry_code);
    int guests = gtk_spin_button_get_value_as_int(ctx->spin_guests);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    for (Category *c = head; c; c = c->next) {
        if (c->id == ctx->selected_id) {
            strncpy(c->code, code, sizeof(c->code) - 1);
            c->guest_count = guests;
            break;
        }
    }
    category_save(head, CATEGORIES_FILE);
    category_free(head);

    cat_refresh_tree(ctx);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Category updated");
}

static void on_cat_delete(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx *)d;

    if (ctx->selected_id < 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "No category selected");
        return;
    }

    if (!ui_confirm_dialog(GTK_WINDOW(ctx->main_win), "Delete this category?")) return;

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    category_delete(&head, ctx->selected_id);
    category_save(head, CATEGORIES_FILE);
    category_free(head);

    ctx->selected_id = -1;
    cat_refresh_tree(ctx);
}

static void on_cat_sort(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx *)d;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    head = category_sort_alpha(head);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
}

static void on_cat_search(GtkWidget *w, gpointer d) {
    (void)w;
    CatCtx *ctx = (CatCtx *)d;
    const char *term = gtk_entry_get_text(ctx->search_entry);

    gtk_list_store_clear(ctx->store);
    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    for (Category *c = head; c; c = c->next) {
        if (strstr(c->code, term) || !term || strlen(term) == 0) {
            GtkTreeIter iter;
            gtk_list_store_append(ctx->store, &iter);
            gtk_list_store_set(ctx->store, &iter,
                0, c->id,
                1, c->code,
                2, c->guest_count,
                -1);
        }
    }
    category_free(head);
}

static GtkWidget *build_category_tab(void) {
    CatCtx *ctx = g_malloc0(sizeof(CatCtx));
    g_cat_ctx = ctx;
    ctx->selected_id = -1;

    PageChrome ch = build_chrome(
        "Category Management",
        "Manage guest categories and their associated guests. Each category can contain up to 4 guests with details like name, age, social class, and side assignment.",
        1);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 16);
    gtk_box_pack_start(GTK_BOX(ch.content), grid, TRUE, TRUE, 0);

    /* Left card: list */
    GtkWidget *left_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    add_class(left_card, "card");
    gtk_grid_attach(GTK_GRID(grid), left_card, 0, 0, 1, 1);

    GtkWidget *left_title = gtk_label_new("Categories List");
    gtk_label_set_xalign(GTK_LABEL(left_title), 0.0);
    add_class(left_title, "card-title");
    gtk_box_pack_start(GTK_BOX(left_card), left_title, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 280);
    gtk_box_pack_start(GTK_BOX(left_card), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Code", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Guests", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_cat_select), ctx);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(left_card), btn_box, FALSE, FALSE, 0);

    GtkWidget *sort_btn = gtk_button_new_with_label("SORT BY GUESTS");
    GtkWidget *delete_btn = gtk_button_new_with_label("DELETE SELECTED");
    add_class(sort_btn, "btn-primary");
    add_class(delete_btn, "btn-danger");
    gtk_box_pack_start(GTK_BOX(btn_box), sort_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);

    g_signal_connect(sort_btn, "clicked", G_CALLBACK(on_cat_sort), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_cat_delete), ctx);

    /* Right card: form */
    GtkWidget *right_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    add_class(right_card, "card");
    gtk_grid_attach(GTK_GRID(grid), right_card, 1, 0, 1, 1);

    GtkWidget *right_title = gtk_label_new("Add / Edit Category");
    gtk_label_set_xalign(GTK_LABEL(right_title), 0.0);
    add_class(right_title, "card-title");
    gtk_box_pack_start(GTK_BOX(right_card), right_title, FALSE, FALSE, 0);

    GtkWidget *form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form), 8);
    gtk_box_pack_start(GTK_BOX(right_card), form, FALSE, FALSE, 0);

    int row = 0;
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Category Code"), 0, row, 1, 1);
    ctx->entry_code = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_code), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Number of Guests (0-4)"), 0, row, 1, 1);
    ctx->spin_guests = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 4, 1));
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->spin_guests), 1, row, 1, 1);

    GtkWidget *form_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(right_card), form_btns, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("ADD CATEGORY");
    GtkWidget *update_btn = gtk_button_new_with_label("UPDATE SELECTED");
    add_class(add_btn, "btn-primary");
    add_class(update_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(form_btns), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(form_btns), update_btn, TRUE, TRUE, 0);
    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_cat_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_cat_update), ctx);

    cat_refresh_tree(ctx);
    return ch.shell;
}

/* =========================================================
   PERSON TAB
   ========================================================= */
static void person_refresh_tree(PersonCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
    person_refresh_stats(ctx);
}

static void person_refresh_stats(PersonCtx *ctx) {
    if (!ctx) return;
    int total = 0;
    int le_count = 0, la_count = 0, vip_count = 0;

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    for (Category *c = head; c; c = c->next) {
        total += c->guest_count;
        if (strstr(c->code, "LE") || strstr(c->code, "Le")) le_count += c->guest_count;
        if (strstr(c->code, "LA") || strstr(c->code, "La")) la_count += c->guest_count;
        if (strstr(c->code, "VIP") || strstr(c->code, "Vip")) vip_count += c->guest_count;
    }
    category_free(head);

    char buf[64];
    g_snprintf(buf, sizeof(buf), "%d", total);
    gtk_label_set_text(ctx->lbl_total, buf);
    g_snprintf(buf, sizeof(buf), "%d", le_count);
    gtk_label_set_text(ctx->lbl_le, buf);
    g_snprintf(buf, sizeof(buf), "%d", la_count);
    gtk_label_set_text(ctx->lbl_la, buf);
    g_snprintf(buf, sizeof(buf), "%d", vip_count);
    gtk_label_set_text(ctx->lbl_vip, buf);
}

static void on_person_select(GtkTreeSelection *sel, gpointer d) {
    PersonCtx *ctx = (PersonCtx *)d;
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        ctx->selected_cat_id = id;
    }
}

static void on_person_add(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    const char *name = gtk_entry_get_text(ctx->entry_name);
    int age = gtk_spin_button_get_value_as_int(ctx->spin_age);
    const char *cls = gtk_entry_get_text(ctx->entry_class);
    const char *side = gtk_combo_box_text_get_active_text(ctx->combo_side);

    if (!name || strlen(name) == 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Name required");
        return;
    }

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    Category *cat = category_find_by_id(head, ctx->selected_cat_id);
    if (!cat) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Select a category first");
        category_free(head);
        return;
    }

    /* Add person to category (simplified - would need person linked list) */
    cat->guest_count++;
    category_save(head, CATEGORIES_FILE);
    category_free(head);

    person_refresh_tree(ctx);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Person added");
}

static void on_person_update(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Update person - not fully implemented");
}

static void on_person_delete(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete person - not fully implemented");
}

static void on_person_search(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Search person - not fully implemented");
}

static GtkWidget *build_person_tab(void) {
    PersonCtx *ctx = g_malloc0(sizeof(PersonCtx));
    g_person_ctx = ctx;
    ctx->selected_cat_id = -1;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Search bar */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), search_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search:"), FALSE, FALSE, 0);
    GtkWidget *search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(search_box), search_entry, TRUE, TRUE, 0);
    GtkWidget *search_btn = gtk_button_new_with_label("Find");
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_person_search), ctx);
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_person_search), ctx);

    /* Split pane */
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), split, TRUE, TRUE, 0);

    /* Tree view */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_paned_pack1(GTK_PANED(split), left, TRUE, FALSE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_box_pack_start(GTK_BOX(left), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Category", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Guests", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_person_select), ctx);

    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(left), btn_box, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("Add");
    GtkWidget *update_btn = gtk_button_new_with_label("Update");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), update_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), refresh_btn, TRUE, TRUE, 0);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_person_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_person_update), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_person_delete), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(person_refresh_tree), ctx);

    /* Right panel: form + stats */
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_paned_pack2(GTK_PANED(split), right, FALSE, FALSE);

    GtkWidget *form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form), 8);
    gtk_box_pack_start(GTK_BOX(right), form, FALSE, FALSE, 0);

    int row = 0;
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Name:"), 0, row, 1, 1);
    ctx->entry_name = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_name), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Age:"), 0, row, 1, 1);
    ctx->spin_age = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 150, 1));
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->spin_age), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Class:"), 0, row, 1, 1);
    ctx->entry_class = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_class), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Side:"), 0, row, 1, 1);
    ctx->combo_side = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_side, "Bride");
    gtk_combo_box_text_append_text(ctx->combo_side, "Groom");
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_side), 1, row, 1, 1);

    /* Stats section */
    GtkWidget *stats_frame = gtk_frame_new("Statistics");
    gtk_box_pack_start(GTK_BOX(right), stats_frame, FALSE, FALSE, 8);

    GtkWidget *stats_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(stats_grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(stats_grid), 8);
    gtk_container_add(GTK_CONTAINER(stats_frame), stats_grid);

    int srow = 0;
    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("Total Guests:"), 0, srow, 1, 1);
    ctx->lbl_total = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_total), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("LE Count:"), 0, srow, 1, 1);
    ctx->lbl_le = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_le), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("LA Count:"), 0, srow, 1, 1);
    ctx->lbl_la = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_la), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("VIP Count:"), 0, srow, 1, 1);
    ctx->lbl_vip = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_vip), 1, srow, 1, 1);

    person_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   PRIORITY TAB
   ========================================================= */
static void prio_refresh_tree(PrioCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
}

static void on_prio_sort_name(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx *)d;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    head = category_sort_alpha(head);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
}

static void on_prio_sort_priority(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx *)d;
    gtk_list_store_clear(ctx->store);

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    head = category_sort_by_guests(head);

    for (Category *c = head; c; c = c->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, c->id,
            1, c->code,
            2, c->guest_count,
            -1);
    }
    category_free(head);
}

static GtkWidget *build_priority_tab(void) {
    PrioCtx *ctx = g_malloc0(sizeof(PrioCtx));
    g_prio_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Sort buttons */
    GtkWidget *sort_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), sort_box, FALSE, FALSE, 0);

    GtkWidget *sort_name_btn = gtk_button_new_with_label("Sort by Name");
    GtkWidget *sort_priority_btn = gtk_button_new_with_label("Sort by Priority");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    gtk_box_pack_start(GTK_BOX(sort_box), sort_name_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(sort_box), sort_priority_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(sort_box), refresh_btn, TRUE, TRUE, 0);

    g_signal_connect(sort_name_btn, "clicked", G_CALLBACK(on_prio_sort_name), ctx);
    g_signal_connect(sort_priority_btn, "clicked", G_CALLBACK(on_prio_sort_priority), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(prio_refresh_tree), ctx);

    /* Tree view */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Code", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Guests", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    /* Info cards */
    GtkWidget *card_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(vbox), card_box, FALSE, FALSE, 8);

    GtkWidget *card1 = gtk_frame_new("Total Categories");
    GtkWidget *card1_lbl = gtk_label_new("0");
    gtk_container_add(GTK_CONTAINER(card1), card1_lbl);
    gtk_box_pack_start(GTK_BOX(card_box), card1, TRUE, TRUE, 0);

    GtkWidget *card2 = gtk_frame_new("Total Guests");
    GtkWidget *card2_lbl = gtk_label_new("0");
    gtk_container_add(GTK_CONTAINER(card2), card2_lbl);
    gtk_box_pack_start(GTK_BOX(card_box), card2, TRUE, TRUE, 0);

    prio_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   GIFT TAB
   ========================================================= */
static void gift_refresh(GiftCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    /* Gift loading would go here - simplified */
    gift_refresh_stats(ctx);
}

static void gift_refresh_stats(GiftCtx *ctx) {
    if (!ctx) return;
    gtk_label_set_text(ctx->lbl_count, "0");
    gtk_label_set_text(ctx->lbl_total, "$0.00");
    gtk_label_set_text(ctx->lbl_avg, "$0.00");
    gtk_label_set_text(ctx->lbl_highest, "$0.00");
}

static void on_gift_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_gift_add(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Add gift - not fully implemented");
}

static void on_gift_update(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Update gift - not fully implemented");
}

static void on_gift_delete(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete gift - not fully implemented");
}

static void on_gift_search(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Search gift - not fully implemented");
}

static GtkWidget *build_gift_tab(void) {
    GiftCtx *ctx = g_malloc0(sizeof(GiftCtx));
    g_gift_ctx = ctx;
    ctx->selected_id = -1;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Search bar */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), search_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search:"), FALSE, FALSE, 0);
    GtkWidget *search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(search_box), search_entry, TRUE, TRUE, 0);
    GtkWidget *search_btn = gtk_button_new_with_label("Find");
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_gift_search), ctx);
    g_signal_connect(search_entry, "activate", G_CALLBACK(on_gift_search), ctx);

    /* Split pane */
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), split, TRUE, TRUE, 0);

    /* Tree view */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_paned_pack1(GTK_PANED(split), left, TRUE, FALSE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_box_pack_start(GTK_BOX(left), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Name", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Value", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Guest ID", r, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_gift_select), ctx);

    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(left), btn_box, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("Add");
    GtkWidget *update_btn = gtk_button_new_with_label("Update");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), update_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), refresh_btn, TRUE, TRUE, 0);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_gift_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_gift_update), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_gift_delete), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(gift_refresh), ctx);

    /* Right panel: form + stats */
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_paned_pack2(GTK_PANED(split), right, FALSE, FALSE);

    GtkWidget *form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form), 8);
    gtk_box_pack_start(GTK_BOX(right), form, FALSE, FALSE, 0);

    int row = 0;
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Name:"), 0, row, 1, 1);
    ctx->entry_name = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_name), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Value:"), 0, row, 1, 1);
    ctx->spin_value = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 100000, 0.01));
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->spin_value), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Guest ID:"), 0, row, 1, 1);
    ctx->spin_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 9999, 1));
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->spin_guest), 1, row, 1, 1);

    /* Stats */
    GtkWidget *stats_frame = gtk_frame_new("Gift Statistics");
    gtk_box_pack_start(GTK_BOX(right), stats_frame, FALSE, FALSE, 8);

    GtkWidget *stats_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(stats_grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(stats_grid), 8);
    gtk_container_add(GTK_CONTAINER(stats_frame), stats_grid);

    int srow = 0;
    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("Count:"), 0, srow, 1, 1);
    ctx->lbl_count = GTK_LABEL(gtk_label_new("0"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_count), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("Total:"), 0, srow, 1, 1);
    ctx->lbl_total = GTK_LABEL(gtk_label_new("$0.00"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_total), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("Average:"), 0, srow, 1, 1);
    ctx->lbl_avg = GTK_LABEL(gtk_label_new("$0.00"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_avg), 1, srow, 1, 1);
    srow++;

    gtk_grid_attach(GTK_GRID(stats_grid), gtk_label_new("Highest:"), 0, srow, 1, 1);
    ctx->lbl_highest = GTK_LABEL(gtk_label_new("$0.00"));
    gtk_grid_attach(GTK_GRID(stats_grid), GTK_WIDGET(ctx->lbl_highest), 1, srow, 1, 1);

    gift_refresh(ctx);
    return vbox;
}

/* =========================================================
   PARKING TAB
   ========================================================= */
static void park_refresh_tree(ParkCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    /* Parking spot loading would go here */
}

static void on_park_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_park_add(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Add parking spot - not fully implemented");
}

static void on_park_update(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Update parking - not fully implemented");
}

static void on_park_delete(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete parking - not fully implemented");
}

static void on_park_search(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Search parking - not fully implemented");
}

static void on_park_yard_close(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *win = GTK_WIDGET(d);
    gtk_widget_destroy(win);
}

static void on_park_yard_btn(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Parking Yard View");
    gtk_window_set_default_size(GTK_WINDOW(win), 600, 400);
    gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(ctx->main_win));
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 550, 300);
    gtk_box_pack_start(GTK_BOX(vbox), draw_area, TRUE, TRUE, 0);

    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(vbox), close_btn, FALSE, FALSE, 0);
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_park_yard_close), win);
    g_signal_connect(win, "destroy", G_CALLBACK(on_park_yard_close), win);

    gtk_widget_show_all(win);
}

static GtkWidget *build_parking_tab(void) {
    ParkCtx *ctx = g_malloc0(sizeof(ParkCtx));
    g_park_ctx = ctx;
    ctx->selected_spot_id = -1;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Top-view button (matches HTML simulation) */
    GtkWidget *yard_top_btn = gtk_button_new_with_label("🅿  VIEW PARKING YARD LAYOUT");
    add_class(yard_top_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(vbox), yard_top_btn, FALSE, FALSE, 0);
    g_signal_connect(yard_top_btn, "clicked", G_CALLBACK(on_park_yard_btn), ctx);

    /* Search bar */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), search_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search:"), FALSE, FALSE, 0);
    ctx->parking_search_guest = GTK_ENTRY(gtk_entry_new());
    gtk_box_pack_start(GTK_BOX(search_box), GTK_WIDGET(ctx->parking_search_guest), TRUE, TRUE, 0);
    GtkWidget *search_btn = gtk_button_new_with_label("Find");
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_park_search), ctx);

    /* Split pane */
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), split, TRUE, TRUE, 0);

    /* Tree view */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_paned_pack1(GTK_PANED(split), left, TRUE, FALSE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_box_pack_start(GTK_BOX(left), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("Spot ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Plate", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Vehicle", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Zone", r, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_park_select), ctx);

    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(left), btn_box, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("Add");
    GtkWidget *update_btn = gtk_button_new_with_label("Update");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    GtkWidget *yard_btn = gtk_button_new_with_label("View Yard");
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), update_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), refresh_btn, TRUE, TRUE, 0);
    /* yard layout moved to top button */

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_park_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_park_update), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_park_delete), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(park_refresh_tree), ctx);
    (void)yard_btn;

    /* Right panel: form */
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_paned_pack2(GTK_PANED(split), right, FALSE, FALSE);

    GtkWidget *form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form), 8);
    gtk_box_pack_start(GTK_BOX(right), form, FALSE, FALSE, 0);

    int row = 0;
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Guest ID:"), 0, row, 1, 1);
    ctx->spin_guest = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 9999, 1));
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->spin_guest), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Plate:"), 0, row, 1, 1);
    ctx->entry_plate = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_plate), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Vehicle:"), 0, row, 1, 1);
    ctx->combo_vehicle = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Car");
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Motorcycle");
    gtk_combo_box_text_append_text(ctx->combo_vehicle, "Van");
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_vehicle), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Preferred Zone:"), 0, row, 1, 1);
    ctx->combo_preferred_zone = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "A");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "B");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "C");
    gtk_combo_box_text_append_text(ctx->combo_preferred_zone, "D");
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_preferred_zone), 1, row, 1, 1);

    park_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   SEATING TAB
   ========================================================= */
static void seat_refresh_tree(SeatCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    /* Seating loading would go here */
}

static void on_seat_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_seat_add(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Add seating - not fully implemented");
}

static void on_seat_update(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Update seating - not fully implemented");
}

static void on_seat_delete(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete seating - not fully implemented");
}

static void on_seat_search(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Search seating - not fully implemented");
}

static void on_hall_close(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *win = GTK_WIDGET(d);
    gtk_widget_destroy(win);
}

static gboolean hall_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)widget; (void)data;

    /* Background */
    cairo_set_source_rgb(cr, 0.93, 0.95, 0.98);
    cairo_paint(cr);

    /* Stage */
    cairo_set_source_rgb(cr, 0.86, 0.26, 0.20); /* red */
    cairo_rectangle(cr, 230, 30, 220, 48);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_move_to(cr, 320, 60);
    cairo_show_text(cr, "STAGE");

    /* VIP tables label */
    cairo_set_source_rgb(cr, 0.55, 0.40, 0.10);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 310, 120);
    cairo_show_text(cr, "VIP Tables");

    /* Table T1 (yellow) */
    cairo_set_source_rgb(cr, 0.98, 0.79, 0.13);
    cairo_rectangle(cr, 250, 145, 120, 70);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 305, 185);
    cairo_show_text(cr, "T1");

    /* Table T2 (brown) */
    cairo_set_source_rgb(cr, 0.72, 0.52, 0.30);
    cairo_rectangle(cr, 390, 145, 120, 70);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 445, 185);
    cairo_show_text(cr, "T2");

    /* Family tables label */
    cairo_set_source_rgb(cr, 0.10, 0.55, 0.45);
    cairo_move_to(cr, 300, 265);
    cairo_show_text(cr, "Family Tables");

    /* Table T3 */
    cairo_set_source_rgb(cr, 0.72, 0.52, 0.30);
    cairo_rectangle(cr, 250, 290, 120, 70);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 305, 330);
    cairo_show_text(cr, "T3");

    /* Table T4 */
    cairo_set_source_rgb(cr, 0.72, 0.52, 0.30);
    cairo_rectangle(cr, 390, 290, 120, 70);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
    cairo_move_to(cr, 445, 330);
    cairo_show_text(cr, "T4");

    return FALSE;
}

static void on_hall_btn(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Ceremony Hall Layout");
    gtk_window_set_default_size(GTK_WINDOW(win), 760, 520);
    gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(ctx->main_win));
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("Guest ID"), FALSE, FALSE, 0);
    GtkWidget *guest = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(guest), "e.g. 2");
    gtk_box_pack_start(GTK_BOX(top), guest, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("Category/Table"), FALSE, FALSE, 0);
    GtkWidget *table = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(table), "e.g. T1");
    gtk_box_pack_start(GTK_BOX(top), table, FALSE, FALSE, 0);
    GtkWidget *find_btn = gtk_button_new_with_label("🔎  Find My Seat");
    add_class(find_btn, "btn-primary");
    gtk_box_pack_end(GTK_BOX(top), find_btn, FALSE, FALSE, 0);

    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

    GtkWidget *draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 720, 440);
    gtk_container_add(GTK_CONTAINER(frame), draw_area);
    g_signal_connect(draw_area, "draw", G_CALLBACK(hall_draw_cb), NULL);

    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(vbox), close_btn, FALSE, FALSE, 0);
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_hall_close), win);
    g_signal_connect(win, "destroy", G_CALLBACK(on_hall_close), win);

    gtk_widget_show_all(win);
}

static GtkWidget *build_seating_tab(void) {
    SeatCtx *ctx = g_malloc0(sizeof(SeatCtx));
    g_seat_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Top-view button (matches HTML simulation) */
    GtkWidget *hall_top_btn = gtk_button_new_with_label("🏛  VIEW CEREMONY HALL LAYOUT");
    add_class(hall_top_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(vbox), hall_top_btn, FALSE, FALSE, 0);
    g_signal_connect(hall_top_btn, "clicked", G_CALLBACK(on_hall_btn), ctx);

    /* Search bar */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), search_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search:"), FALSE, FALSE, 0);
    ctx->seating_search_guest = GTK_ENTRY(gtk_entry_new());
    gtk_box_pack_start(GTK_BOX(search_box), GTK_WIDGET(ctx->seating_search_guest), TRUE, TRUE, 0);
    GtkWidget *search_btn = gtk_button_new_with_label("Find");
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_seat_search), ctx);

    /* Split pane */
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), split, TRUE, TRUE, 0);

    /* Tree view */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_paned_pack1(GTK_PANED(split), left, TRUE, FALSE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_box_pack_start(GTK_BOX(left), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("Table ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Table Name", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Capacity", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_seat_select), ctx);

    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(left), btn_box, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("Add");
    GtkWidget *update_btn = gtk_button_new_with_label("Update");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), update_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), refresh_btn, TRUE, TRUE, 0);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_seat_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_seat_update), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_seat_delete), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(seat_refresh_tree), ctx);

    /* Right panel: forms */
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_paned_pack2(GTK_PANED(split), right, FALSE, FALSE);

    /* Table form */
    GtkWidget *table_frame = gtk_frame_new("Table Management");
    gtk_box_pack_start(GTK_BOX(right), table_frame, FALSE, FALSE, 8);

    GtkWidget *table_form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table_form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(table_form), 8);
    gtk_container_add(GTK_CONTAINER(table_frame), table_form);

    int row = 0;
    gtk_grid_attach(GTK_GRID(table_form), gtk_label_new("Table Name:"), 0, row, 1, 1);
    ctx->entry_table_name = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(table_form), GTK_WIDGET(ctx->entry_table_name), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(table_form), gtk_label_new("Capacity:"), 0, row, 1, 1);
    ctx->spin_table_capacity = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(1, 20, 1));
    gtk_grid_attach(GTK_GRID(table_form), GTK_WIDGET(ctx->spin_table_capacity), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(table_form), gtk_label_new("Type:"), 0, row, 1, 1);
    ctx->combo_table_type = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Round");
    gtk_combo_box_text_append_text(ctx->combo_table_type, "Rectangular");
    gtk_grid_attach(GTK_GRID(table_form), GTK_WIDGET(ctx->combo_table_type), 1, row, 1, 1);

    seat_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   SCHEDULE TAB
   ========================================================= */
static void sched_refresh_tree(SchedCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    /* Schedule loading would go here */
}

static void on_sched_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_sched_add(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Add event - not fully implemented");
}

static void on_sched_update(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Update event - not fully implemented");
}

static void on_sched_delete(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete event - not fully implemented");
}

static GtkWidget *build_schedule_tab(void) {
    SchedCtx *ctx = g_malloc0(sizeof(SchedCtx));
    g_sched_ctx = ctx;
    ctx->selected_event_id = -1;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Tree view */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 250);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Title", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Time", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(ctx->tree));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed", G_CALLBACK(on_sched_select), ctx);

    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), btn_box, FALSE, FALSE, 0);
    GtkWidget *add_btn = gtk_button_new_with_label("Add Event");
    GtkWidget *update_btn = gtk_button_new_with_label("Update Event");
    GtkWidget *delete_btn = gtk_button_new_with_label("Delete Event");
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh");
    gtk_box_pack_start(GTK_BOX(btn_box), add_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), update_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), delete_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), refresh_btn, TRUE, TRUE, 0);

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_sched_add), ctx);
    g_signal_connect(update_btn, "clicked", G_CALLBACK(on_sched_update), ctx);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_sched_delete), ctx);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(sched_refresh_tree), ctx);

    /* Form */
    GtkWidget *form = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form), 6);
    gtk_grid_set_column_spacing(GTK_GRID(form), 8);
    gtk_box_pack_start(GTK_BOX(vbox), form, FALSE, FALSE, 0);

    int row = 0;
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Title:"), 0, row, 1, 1);
    ctx->entry_title = GTK_ENTRY(gtk_entry_new());
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_title), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Category:"), 0, row, 1, 1);
    ctx->combo_category = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_category, "Ceremony");
    gtk_combo_box_text_append_text(ctx->combo_category, "Cocktail");
    gtk_combo_box_text_append_text(ctx->combo_category, "Reception");
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_category), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Start Time:"), 0, row, 1, 1);
    GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    ctx->spin_start_h = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 23, 1));
    ctx->spin_start_m = GTK_SPIN_BUTTON(gtk_spin_button_new_with_range(0, 59, 1));
    gtk_box_pack_start(GTK_BOX(time_box), GTK_WIDGET(ctx->spin_start_h), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), gtk_label_new(":"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), GTK_WIDGET(ctx->spin_start_m), FALSE, FALSE, 0);
    gtk_grid_attach(GTK_GRID(form), time_box, 1, row, 1, 1);

    sched_refresh_tree(ctx);
    return vbox;
}

/* =========================================================
   STATS/DASHBOARD TAB
   ========================================================= */
static void stats_refresh(StatsCtx *ctx) {
    if (!ctx) return;
    gtk_label_set_text(ctx->lbl_cats, "0");
    gtk_label_set_text(ctx->lbl_guests, "0");
    gtk_label_set_text(ctx->lbl_gifts, "0");
    gtk_label_set_text(ctx->lbl_total, "$0.00");
}

static GtkWidget *build_stats_tab(void) {
    StatsCtx *ctx = g_malloc0(sizeof(StatsCtx));
    g_stats_ctx = ctx;

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    /* Title */
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Dashboard</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    /* Stat cards grid */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 0);

    /* Helper function replaced with inline code */
    int row = 0, col = 0;

    /* Card 1: Total Guests */
    GtkWidget *card1 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card1), GTK_SHADOW_OUT);
    gtk_widget_set_size_request(card1, 180, 100);
    GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 12);
    gtk_container_add(GTK_CONTAINER(card1), vbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), gtk_label_new("Total Guests"), FALSE, FALSE, 0);
    ctx->lbl_guests = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_guests), "stat-value");
    gtk_label_set_xalign(ctx->lbl_guests, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox1), GTK_WIDGET(ctx->lbl_guests), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card1, col++, row, 1, 1);

    /* Card 2: Categories */
    GtkWidget *card2 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card2), GTK_SHADOW_OUT);
    gtk_widget_set_size_request(card2, 180, 100);
    GtkWidget *vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), 12);
    gtk_container_add(GTK_CONTAINER(card2), vbox2);
    gtk_box_pack_start(GTK_BOX(vbox2), gtk_label_new("Categories"), FALSE, FALSE, 0);
    ctx->lbl_cats = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_cats), "stat-value");
    gtk_label_set_xalign(ctx->lbl_cats, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(ctx->lbl_cats), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card2, col++, row, 1, 1);

    /* Card 3: Gifts */
    GtkWidget *card3 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card3), GTK_SHADOW_OUT);
    gtk_widget_set_size_request(card3, 180, 100);
    GtkWidget *vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox3), 12);
    gtk_container_add(GTK_CONTAINER(card3), vbox3);
    gtk_box_pack_start(GTK_BOX(vbox3), gtk_label_new("Gifts"), FALSE, FALSE, 0);
    ctx->lbl_gifts = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_gifts), "stat-value");
    gtk_label_set_xalign(ctx->lbl_gifts, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox3), GTK_WIDGET(ctx->lbl_gifts), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card3, col++, row, 1, 1);

    row++; col = 0;

    /* Card 4: Total Gift Value */
    GtkWidget *card4 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card4), GTK_SHADOW_OUT);
    gtk_widget_set_size_request(card4, 180, 100);
    GtkWidget *vbox4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox4), 12);
    gtk_container_add(GTK_CONTAINER(card4), vbox4);
    gtk_box_pack_start(GTK_BOX(vbox4), gtk_label_new("Gift Value"), FALSE, FALSE, 0);
    ctx->lbl_total = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_total), "stat-value");
    gtk_label_set_xalign(ctx->lbl_total, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox4), GTK_WIDGET(ctx->lbl_total), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card4, col++, row, 1, 1);

    /* Refresh button */
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh Stats");
    gtk_box_pack_start(GTK_BOX(vbox), refresh_btn, FALSE, FALSE, 0);
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(stats_refresh), ctx);

    stats_refresh(ctx);
    return vbox;
}

/* =========================================================
   Main UI Runner
   ========================================================= */
static void destroy_cb(GtkWidget *w, gpointer d) {
    (void)w; (void)d;
    gtk_main_quit();
}

void ui_gtk_run(int *argc, char ***argv, AppState *state) {
    gtk_init(argc, argv);

    /* Apply CSS */
    ui_apply_css();

    /* Main window */
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Wedding Guest Management System");
    gtk_window_set_default_size(GTK_WINDOW(win), 1000, 700);
    g_signal_connect(win, "destroy", G_CALLBACK(destroy_cb), NULL);

    /* Store main_win in contexts */
    if (g_home_ctx) g_home_ctx->main_win = win;
    if (g_cat_ctx) g_cat_ctx->main_win = win;
    if (g_person_ctx) g_person_ctx->main_win = win;
    if (g_prio_ctx) g_prio_ctx->main_win = win;
    if (g_gift_ctx) g_gift_ctx->main_win = win;
    if (g_park_ctx) g_park_ctx->main_win = win;
    if (g_seat_ctx) g_seat_ctx->main_win = win;
    if (g_sched_ctx) g_sched_ctx->main_win = win;

    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), main_vbox);

    /* Notebook (tabs) */
    g_notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_vbox), g_notebook, TRUE, TRUE, 0);
    /* We render our own HTML-like nav inside each page. */
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(g_notebook), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(g_notebook), FALSE);

    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_home_tab(), gtk_label_new("Home"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_category_tab(), gtk_label_new("Categories"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_person_tab(), gtk_label_new("Guests"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_priority_tab(), gtk_label_new("Priority"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_gift_tab(), gtk_label_new("Gifts"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_parking_tab(), gtk_label_new("Parking"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_seating_tab(), gtk_label_new("Seating"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_schedule_tab(), gtk_label_new("Schedule"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_stats_tab(), gtk_label_new("Dashboard"));

    gtk_widget_show_all(win);
    gtk_main();
}
