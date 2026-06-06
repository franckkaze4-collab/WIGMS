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
#include <stdio.h>

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
#define GUEST_DB_FILE "data/guest_access.csv"
#define GUEST_DB_FILE_FALLBACK "data/guests_access.csv"
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
    int  id;
    char name[64];
    int  age;
    char category[32];
    char social_class[32];
    char side[16];
    char seat_code[16];
    char parking_zone[16];
    char email[96];
} GuestRecord;

typedef struct {
    GtkWidget     *main_win;
    GtkListStore  *store;
    GtkTreeView   *tree;
    GtkEntry      *search_entry;
    GtkEntry      *entry_name;
    GtkSpinButton *spin_age;
    GtkEntry      *entry_class;
    GtkComboBoxText *combo_category;
    GtkComboBoxText *combo_side;
    GtkEntry      *entry_email;
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

typedef struct {
    GtkWidget     *main_win;
    GtkListStore  *guest_store;
    GtkTreeView   *guest_tree;
    gboolean       selected_ids[64];
    GtkWidget     *draw_area;
    GtkLabel      *lbl_guest_name;
    GtkLabel      *lbl_guest_table;
    GtkLabel      *lbl_guest_parking;
    GtkLabel      *lbl_guest_category;
} InviteCtx;

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
static InviteCtx *g_invite_ctx = NULL;
static gboolean   g_admin_mode = TRUE;
static GtkWidget *g_notebook   = NULL;
static GtkWidget *g_all_nav[10][10]; /* per-page nav button references */
static GuestRecord g_guest_db[64];
static int g_guest_db_count = 0;
static int g_current_guest_id = -1;
static int g_highlight_seat_guest_id = -1;
static int g_highlight_park_guest_id = -1;
static Gift g_gift_db[MAX_GIFTS];
static int g_gift_db_count = 0;
static int g_registered_ids[64];
static int g_registered_count = 0;
#define REGISTRATIONS_FILE "data/registrations.dat"

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
static void on_nav_invitation(GtkWidget *w, gpointer d);
static void on_park_yard_btn(GtkWidget *w, gpointer d);
static void on_hall_btn(GtkWidget *w, gpointer d);

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

static const GuestRecord* guest_find_by_name(const char *name) {
    if (!name || !name[0]) return NULL;
    for (int i = 0; i < g_guest_db_count; i++) {
        if (g_ascii_strcasecmp(g_guest_db[i].name, name) == 0) return &g_guest_db[i];
    }
    return NULL;
}

static const GuestRecord* guest_find_by_id(int id) {
    for (int i = 0; i < g_guest_db_count; i++) {
        if (g_guest_db[i].id == id) return &g_guest_db[i];
    }
    return NULL;
}

static void guest_db_ensure_seed_file(void) {
    FILE *fp = fopen(GUEST_DB_FILE, "r");
    if (fp) {
        fclose(fp);
        return;
    }
    fp = fopen(GUEST_DB_FILE_FALLBACK, "r");
    if (fp) {
        fclose(fp);
        return;
    }
    fp = fopen(GUEST_DB_FILE, "w");
    if (!fp) return;
    fputs("id,name,age,category,social_class,side,seat_code,parking_zone,email\n", fp);
    fputs("1,Pierre Dubois,35,FAM_LE,VIP,Le marie,T1-A,A1,\n", fp);
    fputs("2,Marie Laurent,28,VIP_LA,Family,La mariee,T1-B,A2,\n", fp);
    fputs("3,Jean Martin,30,FRIEND_LE,Friend,Le marie,T2-A,A3,\n", fp);
    fputs("4,Sophie Bernard,27,VIP_LA,VIP,La mariee,T2-B,A4,\n", fp);
    fputs("5,Luc Petit,42,STAFF,Staff,Le marie,T3-A,B1,\n", fp);
    fputs("6,Anne Moreau,31,FRIEND_LA,Friend,La mariee,T3-B,B2,\n", fp);
    fputs("7,Marc Lefebvre,45,VIP_LE,VIP,Le marie,T4-A,B3,\n", fp);
    fputs("8,Julie Caron,33,FAM_LA,Family,La mariee,T4-B,B4,\n", fp);
    fputs("9,Paul Girard,38,FAM_LE,Family,Le marie,T5-A,C1,\n", fp);
    fputs("10,Claire Martin,29,FRIEND_LA,Friend,La mariee,T5-B,C2,\n", fp);
    fputs("11,Thomas Leroy,36,FAM_LE,Family,Le marie,T6-A,C3,\n", fp);
    fputs("12,Emma Petit,26,VIP_LA,VIP,La mariee,T6-B,C4,\n", fp);
    fputs("13,Lucas Robert,34,FRIEND_LE,Friend,Le marie,T7-A,D1,\n", fp);
    fputs("14,Chloe Simon,32,FAM_LA,Family,La mariee,T7-B,D2,\n", fp);
    fputs("15,Antoine Henry,41,STAFF,Staff,Le marie,T8-A,D3,\n", fp);
    fputs("16,Lea Garcia,25,FRIEND_LA,Friend,La mariee,T8-B,D4,\n", fp);
    fputs("17,David Roux,37,VIP_LE,VIP,Le marie,T9-A,E1,\n", fp);
    fputs("18,Nina Blanc,27,VIP_LA,VIP,La mariee,T9-B,E2,\n", fp);
    fputs("19,Hugo Faure,39,FAM_LE,Family,Le marie,T10-A,E3,\n", fp);
    fputs("20,Manon Noel,30,FAM_LA,Family,La mariee,T10-B,E4,\n", fp);
    fclose(fp);
}

static void registrations_save(void) {
    FILE *f = fopen(REGISTRATIONS_FILE, "wb");
    if (!f) return;
    fwrite(&g_registered_count, sizeof(int), 1, f);
    fwrite(g_registered_ids, sizeof(int), g_registered_count, f);
    fclose(f);
}

static void registrations_load(void) {
    FILE *f = fopen(REGISTRATIONS_FILE, "rb");
    if (!f) { g_registered_count = 0; return; }
    if (fread(&g_registered_count, sizeof(int), 1, f) != 1) { g_registered_count = 0; fclose(f); return; }
    if (g_registered_count > 64) g_registered_count = 64;
    fread(g_registered_ids, sizeof(int), g_registered_count, f);
    fclose(f);
}

static gboolean is_guest_registered(int id) {
    for (int i = 0; i < g_registered_count; i++)
        if (g_registered_ids[i] == id) return TRUE;
    return FALSE;
}

static void guest_db_load(void) {
    g_guest_db_count = 0;
    guest_db_ensure_seed_file();
    FILE *fp = fopen(GUEST_DB_FILE, "r");
    if (!fp) fp = fopen(GUEST_DB_FILE_FALLBACK, "r");
    if (!fp) return;

    char line[512];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return;
    }
    while (fgets(line, sizeof(line), fp) && g_guest_db_count < (int)(sizeof(g_guest_db) / sizeof(g_guest_db[0]))) {
        GuestRecord rec;
        memset(&rec, 0, sizeof(rec));
        char *tok = strtok(line, ",");
        if (!tok) continue;
        rec.id = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; g_strlcpy(rec.name, tok, sizeof(rec.name));
        tok = strtok(NULL, ","); if (!tok) continue; rec.age = atoi(tok);
        tok = strtok(NULL, ","); if (!tok) continue; g_strlcpy(rec.category, tok, sizeof(rec.category));
        tok = strtok(NULL, ","); if (!tok) continue; g_strlcpy(rec.social_class, tok, sizeof(rec.social_class));
        tok = strtok(NULL, ","); if (!tok) continue; g_strlcpy(rec.side, tok, sizeof(rec.side));
        tok = strtok(NULL, ","); if (!tok) continue; g_strlcpy(rec.seat_code, tok, sizeof(rec.seat_code));
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; g_strlcpy(rec.parking_zone, tok, sizeof(rec.parking_zone));
        tok = strtok(NULL, ",\r\n"); if (tok) g_strlcpy(rec.email, tok, sizeof(rec.email)); else rec.email[0] = '\0';
        g_guest_db[g_guest_db_count++] = rec;
    }
    fclose(fp);
}

typedef struct {
    GtkWidget *shell;
    GtkWidget *content;
    GtkWidget *admin_btn;
    GtkWidget *guest_btn;
    GtkWidget *nav_buttons[10];
} PageChrome;

static void chrome_set_active(PageChrome *ch, int active_tab) {
    if (!ch) return;
    for (int i = 0; i < 10; i++) {
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

    const char *labels[10] = {"Home","Category","Person","Priority","Gift","Parking","Schedule","Seating","Invitation","Dashboard"};
    void (*handlers[10])(GtkWidget*,gpointer) = {
        on_nav_home, on_nav_categories, on_nav_persons, on_nav_priority, on_nav_gifts,
        on_nav_parking, on_nav_schedule, on_nav_seating, on_nav_invitation, on_nav_dashboard
    };
    for (int i = 0; i < 10; i++) {
        GtkWidget *b = gtk_button_new_with_label(labels[i]);
        add_class(b, "nav-pill");
        gtk_box_pack_start(GTK_BOX(nav), b, FALSE, FALSE, 0);
        g_signal_connect(b, "clicked", G_CALLBACK(handlers[i]), NULL);
        ch.nav_buttons[i] = b;
    }

    /* Store nav buttons globally for mode toggle sensitivity updates */
    if (active_tab >= 0 && active_tab < 10)
        for (int i = 0; i < 10; i++) g_all_nav[active_tab][i] = ch.nav_buttons[i];

    /* Match HTML behavior: admin-only nav items are disabled in Guest mode */
    if (!g_admin_mode) {
        int admin_only[] = {1, 3, 8, 9};
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

static gboolean show_admin_password_dialog(void) {
    while (1) {
        GtkWidget *dlg = gtk_dialog_new_with_buttons(
            "Admin Login",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Login", GTK_RESPONSE_OK,
            NULL);
        GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
        GtkWidget *grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
        gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
        gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

        gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Enter admin password:"), 0, 0, 1, 1);
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Password");
        gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

        gtk_widget_show_all(dlg);
        gint resp = gtk_dialog_run(GTK_DIALOG(dlg));
        char pw[128];
        g_strlcpy(pw, gtk_entry_get_text(GTK_ENTRY(entry)), sizeof(pw));
        gtk_widget_destroy(dlg);

        if (resp != GTK_RESPONSE_OK) return FALSE;
        if (strcmp(pw, "Roddysenpai") == 0) return TRUE;
        ui_show_error_dialog(NULL, "Login Failed", "Incorrect password. Try again.");
    }
}

static void on_welcome_choice_clicked(GtkWidget *button, gpointer dialog) {
    int resp = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "resp"));
    gtk_dialog_response(GTK_DIALOG(dialog), resp);
}

static gboolean show_welcome_role_dialog(void) {
    GtkWidget *dlg = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dlg), "Welcome");
    gtk_window_set_modal(GTK_WINDOW(dlg), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 1000, 700);
    gtk_window_set_resizable(GTK_WINDOW(dlg), FALSE);

    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(box), 0);

    GtkWidget *overlay = gtk_overlay_new();
    gtk_box_pack_start(GTK_BOX(box), overlay, TRUE, TRUE, 0);

    GError *err = NULL;
    GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale("web/1 (32).png", 1000, 700, FALSE, &err);
    GtkWidget *bg = NULL;
    if (pix) {
        bg = gtk_image_new_from_pixbuf(pix);
        g_object_unref(pix);
    } else {
        if (err) g_error_free(err);
        bg = gtk_image_new();
    }
    gtk_container_add(GTK_CONTAINER(overlay), bg);
    gtk_widget_set_halign(bg, GTK_ALIGN_FILL);
    gtk_widget_set_valign(bg, GTK_ALIGN_FILL);

    GtkWidget *panel_frame = gtk_frame_new(NULL);
    gtk_widget_set_halign(panel_frame, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(panel_frame, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), panel_frame);

    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 28);
    gtk_container_set_border_width(GTK_CONTAINER(panel), 24);
    gtk_container_add(GTK_CONTAINER(panel_frame), panel);
    gtk_widget_set_halign(panel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(panel, GTK_ALIGN_CENTER);
    add_class(panel_frame, "card");
    gtk_widget_set_size_request(panel_frame, 860, 300);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span size='xx-large' weight='bold' foreground='#ffffff'>WELCOME TO THE WEDDING OF BRYAN AND DANIELLE</span>");
    gtk_label_set_line_wrap(GTK_LABEL(title), TRUE);
    gtk_label_set_xalign(GTK_LABEL(title), 0.5f);
    gtk_box_pack_start(GTK_BOX(panel), title, FALSE, FALSE, 0);

    GtkWidget *subtitle = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(subtitle),
        "<span size='large' foreground='#e9e5ff'>Please choose your access mode</span>");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.5f);
    gtk_box_pack_start(GTK_BOX(panel), subtitle, FALSE, FALSE, 0);

    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 24);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(panel), btn_row, FALSE, FALSE, 8);
    GtkWidget *admin_btn = gtk_button_new_with_label("ADMIN");
    GtkWidget *guest_btn = gtk_button_new_with_label("GUEST");
    gtk_widget_set_size_request(admin_btn, 160, 44);
    gtk_widget_set_size_request(guest_btn, 160, 44);
    add_class(admin_btn, "btn-primary");
    add_class(guest_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(btn_row), admin_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_row), guest_btn, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(admin_btn), "resp", GINT_TO_POINTER(1001));
    g_signal_connect(admin_btn, "clicked", G_CALLBACK(on_welcome_choice_clicked), dlg);
    g_object_set_data(G_OBJECT(guest_btn), "resp", GINT_TO_POINTER(1002));
    g_signal_connect(guest_btn, "clicked", G_CALLBACK(on_welcome_choice_clicked), dlg);

    GtkWidget *quit_btn = gtk_button_new_with_label("Quit");
    gtk_widget_set_halign(quit_btn, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(quit_btn, 344, 44);
    gtk_box_pack_start(GTK_BOX(panel), quit_btn, FALSE, FALSE, 8);
    g_object_set_data(G_OBJECT(quit_btn), "resp", GINT_TO_POINTER(GTK_RESPONSE_CANCEL));
    g_signal_connect(quit_btn, "clicked", G_CALLBACK(on_welcome_choice_clicked), dlg);

    gtk_widget_show_all(dlg);
    gint resp = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);

    if (resp == 1001) {
        g_admin_mode = TRUE;
        return show_admin_password_dialog();
    }
    if (resp == 1002) {
        g_admin_mode = FALSE;
        return TRUE;
    }
    return FALSE;
}

/* =========================================================
   Navigation helpers
   ========================================================= */
static void switch_to_tab(int tab_index) {
    if (g_notebook) {
        /* Guest mode: block admin-only modules like the HTML simulation */
        if (!g_admin_mode && (tab_index == 1 || tab_index == 3 || tab_index == 8 || tab_index == 9)) {
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
    switch_to_tab(9);
}

static void on_nav_invitation(GtkWidget *w, gpointer d) {
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

    /* Show admin tabs (Categories=1, Priority=3, Invitation=8, Dashboard=9) */
    if (g_notebook) {
        GtkWidget *cat_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 1);
        GtkWidget *prio_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 3);
        GtkWidget *inv_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 8);
        GtkWidget *dash_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 9);
        if (cat_tab) gtk_widget_show(cat_tab);
        if (prio_tab) gtk_widget_show(prio_tab);
        if (inv_tab) gtk_widget_show(inv_tab);
        if (dash_tab) gtk_widget_show(dash_tab);
    }
    /* Re-enable admin-only nav buttons on all pages */
    if (g_notebook) {
        int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(g_notebook));
        for (int p = 0; p < n; p++)
            for (int i = 0; i < 10 && g_all_nav[p][i]; i++)
                gtk_widget_set_sensitive(g_all_nav[p][i], TRUE);
    }
}

static void on_toggle_guest(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *admin_btn = GTK_WIDGET(d);
    g_admin_mode = FALSE;
    gtk_style_context_add_class(gtk_widget_get_style_context(w), "suggested-action");
    gtk_style_context_remove_class(gtk_widget_get_style_context(admin_btn), "suggested-action");

    /* Hide admin tabs (Categories=1, Priority=3, Invitation=8, Dashboard=9) */
    if (g_notebook) {
        GtkWidget *cat_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 1);
        GtkWidget *prio_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 3);
        GtkWidget *inv_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 8);
        GtkWidget *dash_tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_notebook), 9);
        if (cat_tab) gtk_widget_hide(cat_tab);
        if (prio_tab) gtk_widget_hide(prio_tab);
        if (inv_tab) gtk_widget_hide(inv_tab);
        if (dash_tab) gtk_widget_hide(dash_tab);
    }
    /* Disable admin-only nav buttons on all pages */
    if (g_notebook) {
        int n = gtk_notebook_get_n_pages(GTK_NOTEBOOK(g_notebook));
        int admin_only[] = {1, 3, 8, 9};
        for (int p = 0; p < n; p++) {
            for (size_t k = 0; k < sizeof(admin_only) / sizeof(admin_only[0]); k++) {
                int idx = admin_only[k];
                if (g_all_nav[p][idx])
                    gtk_widget_set_sensitive(g_all_nav[p][idx], FALSE);
            }
        }
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
    for (Category *c = head; c; c = c->next) c->guest_count = 0;

    /* Ensure categories from guest_access.csv are present and counted */
    for (int i = 0; i < g_guest_db_count; i++) {
        const char *code = g_guest_db[i].category;
        Category *found = NULL;
        for (Category *c = head; c; c = c->next) {
            if (g_ascii_strcasecmp(c->code, code) == 0) {
                found = c;
                break;
            }
        }
        if (!found) {
            Category *new_cat = g_malloc0(sizeof(Category));
            new_cat->id = category_get_next_id(head);
            g_strlcpy(new_cat->code, code, sizeof(new_cat->code));
            new_cat->guest_count = 0;
            new_cat->next = head;
            head = new_cat;
            found = new_cat;
        }
        found->guest_count += 1;
    }

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

static gboolean combo_contains_text(GtkComboBoxText *combo, const char *text) {
    if (!combo || !text || !*text) return FALSE;
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gchar *value = NULL;
        gtk_tree_model_get(model, &iter, 0, &value, -1);
        if (value && g_ascii_strcasecmp(value, text) == 0) {
            g_free(value);
            return TRUE;
        }
        g_free(value);
        valid = gtk_tree_model_iter_next(model, &iter);
    }
    return FALSE;
}

static void populate_person_category_combo(GtkComboBoxText *combo) {
    if (!combo) return;

    gtk_combo_box_text_append_text(combo, "-- Select Category --");

    Category *head = NULL;
    category_load(&head, CATEGORIES_FILE);
    for (Category *c = head; c; c = c->next) {
        if (c->code[0] != '\0' && !combo_contains_text(combo, c->code)) {
            gtk_combo_box_text_append_text(combo, c->code);
        }
    }

    for (int i = 0; i < g_guest_db_count; i++) {
        const char *csv_cat = g_guest_db[i].category;
        if (csv_cat && csv_cat[0] != '\0' && !combo_contains_text(combo, csv_cat)) {
            gtk_combo_box_text_append_text(combo, csv_cat);
        }
    }

    category_free(head);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
}

static int combo_get_index_of_text(GtkComboBoxText *combo, const char *text) {
    if (!combo || !text || !*text) return -1;
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
    int index = 0;
    while (valid) {
        gchar *value = NULL;
        gtk_tree_model_get(model, &iter, 0, &value, -1);
        if (value && g_ascii_strcasecmp(value, text) == 0) {
            g_free(value);
            return index;
        }
        g_free(value);
        valid = gtk_tree_model_iter_next(model, &iter);
        index++;
    }
    return -1;
}

static void set_combo_active_text(GtkComboBoxText *combo, const char *text) {
    int idx = combo_get_index_of_text(combo, text);
    if (idx >= 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), idx);
    }
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

static void guest_db_save_all(void) {
    const char *path = GUEST_DB_FILE;
    FILE *fp = fopen(path, "w");
    if (!fp) { fp = fopen(GUEST_DB_FILE_FALLBACK, "w"); path = GUEST_DB_FILE_FALLBACK; }
    if (!fp) return;
    fputs("id,name,age,category,social_class,side,seat_code,parking_zone,email\n", fp);
    for (int i = 0; i < g_guest_db_count; i++) {
        GuestRecord *r = &g_guest_db[i];
        fprintf(fp, "%d,%s,%d,%s,%s,%s,%s,%s,%s\n",
                r->id, r->name, r->age, r->category, r->social_class,
                r->side, r->seat_code, r->parking_zone, r->email);
    }
    fclose(fp);
}

static void add_guest_to_csv(const char *name, int age, const char *cat, const char *cls, const char *side, const char *email) {
    int new_id = 1;
    for (int i = 0; i < g_guest_db_count; i++)
        if (g_guest_db[i].id >= new_id) new_id = g_guest_db[i].id + 1;

    FILE *fp = fopen(GUEST_DB_FILE, "a");
    if (!fp) { fp = fopen(GUEST_DB_FILE_FALLBACK, "a"); }
    if (!fp) return;
    fprintf(fp, "%d,%s,%d,%s,%s,%s,T%d-A,A%d,%s\n", new_id, name, age, cat ? cat : "FRIEND_LE", cls ? cls : "Friend", side ? side : "Le marie", new_id, new_id, email ? email : "");
    fclose(fp);
    guest_db_load();
}

static void on_person_add(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    const char *name = gtk_entry_get_text(ctx->entry_name);

    if (!name || strlen(name) == 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Name required");
        return;
    }

    const GuestRecord *gr = guest_find_by_name(name);
    if (!gr) {
        if (g_admin_mode) {
            char buf[512];
            g_snprintf(buf, sizeof(buf), "Guest '%s' is not on the guest list.\n\nDo you want to add this guest to the guest access list?", name);
            if (ui_confirm_dialog(GTK_WINDOW(ctx->main_win), buf)) {
                add_guest_to_csv(name,
                    gtk_spin_button_get_value_as_int(ctx->spin_age),
                    gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_category)) > 0 ? gtk_combo_box_text_get_active_text(ctx->combo_category) : "FRIEND_LE",
                    gtk_entry_get_text(ctx->entry_class),
                    gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_side)) == 0 ? "La mariee" : "Le marie",
                    gtk_entry_get_text(ctx->entry_email));
                ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Added", "Guest added to the access list.");
            }
        } else {
            ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "Guest name is not on the list. As a guest you cannot add names.");
        }
        return;
    }

    if (is_guest_registered(gr->id)) {
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Already Registered", "Guest Already Registered");
        return;
    }

    g_current_guest_id = gr->id;
    if (g_registered_count < 64) {
        g_registered_ids[g_registered_count++] = gr->id;
        registrations_save();
    }
    gtk_spin_button_set_value(ctx->spin_age, gr->age);
    gtk_entry_set_text(ctx->entry_class, gr->social_class);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_side), (g_ascii_strncasecmp(gr->side, "La", 2) == 0) ? 0 : 1);
    set_combo_active_text(ctx->combo_category, gr->category);

    char msg[256];
    g_snprintf(msg, sizeof(msg), "Guest verified.\nPrivate ID: %d\nCategory: %s\nSeat: %s\nParking: %s",
               gr->id, gr->category, gr->seat_code, gr->parking_zone);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Registration Successful", msg);
}

static void on_person_update(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    const char *name = gtk_entry_get_text(ctx->entry_name);
    if (!name || strlen(name) == 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Name required");
        return;
    }
    const GuestRecord *gr = guest_find_by_name(name);
    if (!gr) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "No guest with that name in the access list.");
        return;
    }
    for (int i = 0; i < g_guest_db_count; i++) {
        if (g_guest_db[i].id == gr->id) {
            g_strlcpy(g_guest_db[i].name, gtk_entry_get_text(ctx->entry_name), sizeof(g_guest_db[i].name));
            g_guest_db[i].age = gtk_spin_button_get_value_as_int(ctx->spin_age);
            g_strlcpy(g_guest_db[i].social_class, gtk_entry_get_text(ctx->entry_class), sizeof(g_guest_db[i].social_class));
            const char *cat = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_category)) > 0 ? gtk_combo_box_text_get_active_text(ctx->combo_category) : "";
            if (cat && cat[0]) g_strlcpy(g_guest_db[i].category, cat, sizeof(g_guest_db[i].category));
            const char *side_str = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx->combo_side)) == 0 ? "La mariee" : "Le marie";
            g_strlcpy(g_guest_db[i].side, side_str, sizeof(g_guest_db[i].side));
            g_strlcpy(g_guest_db[i].email, gtk_entry_get_text(ctx->entry_email), sizeof(g_guest_db[i].email));
            guest_db_save_all();
            ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Updated", "Guest information updated successfully.");
            return;
        }
    }
}

static void on_person_delete(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Delete person - not fully implemented");
}

static void on_person_search(GtkWidget *w, gpointer d) {
    (void)w;
    PersonCtx *ctx = (PersonCtx *)d;
    const char *name = gtk_entry_get_text(ctx->search_entry);
    const GuestRecord *gr = guest_find_by_name(name);
    if (!gr) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "Guest not found in access table.");
        return;
    }
    g_current_guest_id = gr->id;
    gtk_entry_set_text(ctx->entry_name, gr->name);
    gtk_spin_button_set_value(ctx->spin_age, gr->age);
    gtk_entry_set_text(ctx->entry_class, gr->social_class);
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_side), (g_ascii_strncasecmp(gr->side, "La", 2) == 0) ? 0 : 1);
    set_combo_active_text(ctx->combo_category, gr->category);
    gtk_entry_set_text(ctx->entry_email, gr->email);
    char msg[256];
    g_snprintf(msg, sizeof(msg), "Guest: %s\nPrivate ID: %d\nCategory: %s\nEmail: %s", gr->name, gr->id, gr->category, gr->email);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Search Result", msg);
}

static GtkWidget *build_person_tab(void) {
    PersonCtx *ctx = g_malloc0(sizeof(PersonCtx));
    g_person_ctx = ctx;
    ctx->selected_cat_id = -1;

    PageChrome ch = build_chrome(
        "Person Registration",
        "View and edit all registered guests. Select a guest from the list to modify their details including name, age, social class, and side assignment.",
        2);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    /* Search bar */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), search_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(search_box), gtk_label_new("Search:"), FALSE, FALSE, 0);
    ctx->search_entry = GTK_ENTRY(gtk_entry_new());
    gtk_box_pack_start(GTK_BOX(search_box), GTK_WIDGET(ctx->search_entry), TRUE, TRUE, 0);
    GtkWidget *search_btn = gtk_button_new_with_label("Find");
    gtk_box_pack_start(GTK_BOX(search_box), search_btn, FALSE, FALSE, 0);
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_person_search), ctx);
    g_signal_connect(ctx->search_entry, "activate", G_CALLBACK(on_person_search), ctx);

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

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Category:"), 0, row, 1, 1);
    ctx->combo_category = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    populate_person_category_combo(ctx->combo_category);
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_category), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Side:"), 0, row, 1, 1);
    ctx->combo_side = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(ctx->combo_side, "La mariee");
    gtk_combo_box_text_append_text(ctx->combo_side, "Le marie");
    gtk_combo_box_set_active(GTK_COMBO_BOX(ctx->combo_side), 0);
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->combo_side), 1, row, 1, 1);
    row++;

    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Email:"), 0, row, 1, 1);
    ctx->entry_email = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(ctx->entry_email, "guest@example.com");
    gtk_grid_attach(GTK_GRID(form), GTK_WIDGET(ctx->entry_email), 1, row, 1, 1);
    row++;

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
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   PRIORITY TAB
   ========================================================= */
static void prio_refresh_tree(PrioCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);

    // Display all guests from CSV
    for (int i = 0; i < g_guest_db_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, g_guest_db[i].id,
            1, g_guest_db[i].name,
            2, g_guest_db[i].category,
            -1);
    }
}

static void on_prio_sort_name(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx *)d;
    gtk_list_store_clear(ctx->store);

    // Sort guests by name (simple bubble sort for demonstration)
    for (int i = 0; i < g_guest_db_count - 1; i++) {
        for (int j = 0; j < g_guest_db_count - i - 1; j++) {
            if (g_ascii_strcasecmp(g_guest_db[j].name, g_guest_db[j+1].name) > 0) {
                GuestRecord temp = g_guest_db[j];
                g_guest_db[j] = g_guest_db[j+1];
                g_guest_db[j+1] = temp;
            }
        }
    }

    for (int i = 0; i < g_guest_db_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, g_guest_db[i].id,
            1, g_guest_db[i].name,
            2, g_guest_db[i].category,
            -1);
    }
}

static void on_prio_sort_priority(GtkWidget *w, gpointer d) {
    (void)w;
    PrioCtx *ctx = (PrioCtx *)d;
    gtk_list_store_clear(ctx->store);

    // Sort guests by priority (age descending, then VIP status, then social class)
    for (int i = 0; i < g_guest_db_count - 1; i++) {
        for (int j = 0; j < g_guest_db_count - i - 1; j++) {
            int priority_j = 0, priority_j1 = 0;
            
            // Age priority (older = higher priority)
            priority_j += g_guest_db[j].age;
            priority_j1 += g_guest_db[j+1].age;
            
            // VIP priority
            if (strstr(g_guest_db[j].category, "VIP")) priority_j += 100;
            if (strstr(g_guest_db[j+1].category, "VIP")) priority_j1 += 100;
            
            // Social class priority (Family > Friend > Staff)
            if (strstr(g_guest_db[j].social_class, "Family")) priority_j += 50;
            else if (strstr(g_guest_db[j].social_class, "Friend")) priority_j += 25;
            
            if (strstr(g_guest_db[j+1].social_class, "Family")) priority_j1 += 50;
            else if (strstr(g_guest_db[j+1].social_class, "Friend")) priority_j1 += 25;
            
            if (priority_j < priority_j1) {
                GuestRecord temp = g_guest_db[j];
                g_guest_db[j] = g_guest_db[j+1];
                g_guest_db[j+1] = temp;
            }
        }
    }

    for (int i = 0; i < g_guest_db_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, g_guest_db[i].id,
            1, g_guest_db[i].name,
            2, g_guest_db[i].category,
            -1);
    }
}

static GtkWidget *build_priority_tab(void) {
    PrioCtx *ctx = g_malloc0(sizeof(PrioCtx));
    g_prio_ctx = ctx;

    PageChrome ch = build_chrome(
        "Priority Sorting",
        "View and sort all registered guests from the CSV file by name or priority criteria.",
        3);

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

    ctx->store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
    ctx->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->store)));
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->tree));

    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c = gtk_tree_view_column_new_with_attributes("ID", r, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Name", r, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);
    c = gtk_tree_view_column_new_with_attributes("Category", r, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(ctx->tree), c);

    /* Info cards */
    GtkWidget *card_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(vbox), card_box, FALSE, FALSE, 8);

    GtkWidget *card1 = gtk_frame_new("Total Guests");
    char guest_count_buf[32];
    g_snprintf(guest_count_buf, sizeof(guest_count_buf), "%d", g_guest_db_count);
    GtkWidget *card1_lbl = gtk_label_new(guest_count_buf);
    gtk_container_add(GTK_CONTAINER(card1), card1_lbl);
    gtk_box_pack_start(GTK_BOX(card_box), card1, TRUE, TRUE, 0);

    // Count unique categories
    int unique_cats = 0;
    for (int i = 0; i < g_guest_db_count; i++) {
        gboolean seen = FALSE;
        for (int j = 0; j < i; j++) {
            if (g_ascii_strcasecmp(g_guest_db[i].category, g_guest_db[j].category) == 0) {
                seen = TRUE;
                break;
            }
        }
        if (!seen) unique_cats++;
    }

    GtkWidget *card2 = gtk_frame_new("Categories");
    char cat_count_buf[32];
    g_snprintf(cat_count_buf, sizeof(cat_count_buf), "%d", unique_cats);
    GtkWidget *card2_lbl = gtk_label_new(cat_count_buf);
    gtk_container_add(GTK_CONTAINER(card2), card2_lbl);
    gtk_box_pack_start(GTK_BOX(card_box), card2, TRUE, TRUE, 0);

    prio_refresh_tree(ctx);
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   GIFT TAB
   ========================================================= */
static void gift_refresh(GiftCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    for (int i = 0; i < g_gift_db_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, g_gift_db[i].gift_id,
            1, g_gift_db[i].name,
            2, (double)g_gift_db[i].value,
            3, g_gift_db[i].guest_id,
            -1);
    }
    gift_refresh_stats(ctx);
}

static void gift_refresh_stats(GiftCtx *ctx) {
    if (!ctx) return;
    double total = 0.0, highest = 0.0;
    for (int i = 0; i < g_gift_db_count; i++) {
        total += g_gift_db[i].value;
        if (g_gift_db[i].value > highest) highest = g_gift_db[i].value;
    }
    double avg = (g_gift_db_count > 0) ? total / g_gift_db_count : 0.0;
    char buf[64];
    g_snprintf(buf, sizeof(buf), "%d", g_gift_db_count);
    gtk_label_set_text(ctx->lbl_count, buf);
    g_snprintf(buf, sizeof(buf), "%.2f FCFA", total);
    gtk_label_set_text(ctx->lbl_total, buf);
    g_snprintf(buf, sizeof(buf), "%.2f FCFA", avg);
    gtk_label_set_text(ctx->lbl_avg, buf);
    g_snprintf(buf, sizeof(buf), "%.2f FCFA", highest);
    gtk_label_set_text(ctx->lbl_highest, buf);
}

static void on_gift_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_gift_add(GtkWidget *w, gpointer d) {
    (void)w;
    GiftCtx *ctx = (GiftCtx *)d;
    const char *name = gtk_entry_get_text(ctx->entry_name);
    int guest_id = gtk_spin_button_get_value_as_int(ctx->spin_guest);
    double value = gtk_spin_button_get_value(ctx->spin_value);

    if (!name || !name[0]) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Gift name is required.");
        return;
    }
    if (guest_id <= 0 && g_current_guest_id > 0) {
        guest_id = g_current_guest_id;
        gtk_spin_button_set_value(ctx->spin_guest, guest_id);
    }
    if (!guest_find_by_id(guest_id)) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Guest ID not found in sample access table.");
        return;
    }
    if (g_gift_db_count >= MAX_GIFTS) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Gift storage full.");
        return;
    }

    Gift *g = &g_gift_db[g_gift_db_count++];
    g->gift_id = gift_id_counter++;
    g_strlcpy(g->name, name, sizeof(g->name));
    g->value = (float)value;
    g->guest_id = guest_id;

    gift_save(g_gift_db, g_gift_db_count, GIFTS_FILE);
    gift_refresh(ctx);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Gift Saved", "Gift added and linked to guest.");
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

    PageChrome ch = build_chrome(
        "Gift Registry",
        "Track gifts, values, and guest references. Add, update, and save gift records.",
        4);

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
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   PARKING TAB
   ========================================================= */
static void park_refresh_tree(ParkCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    ParkingLot lot;
    parking_init(&lot, 10, 20, 12, 8);
    parking_load(&lot, PARKING_FILE);
    for (int i = 0; i < lot.total_spots && i < 50; i++) {
        ParkingSpot *s = &lot.spots[i];
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        char zcode[8];
        g_snprintf(zcode, sizeof(zcode), "%s%d", lot.zones[s->zone].code, (i % 10) + 1);
        gtk_list_store_set(ctx->store, &iter,
            0, s->spot_id,
            1, s->plate[0] ? s->plate : "---",
            2, parking_vehicle_name(s->vehicle_type),
            3, zcode,
            -1);
    }
}

static void on_park_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_park_add(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    if (!g_admin_mode) {
        if (g_current_guest_id > 0) {
            g_highlight_park_guest_id = g_current_guest_id;
            const GuestRecord *gr = guest_find_by_id(g_current_guest_id);
            if (gr) {
                char msg[256];
                g_snprintf(msg, sizeof(msg), "%s -> Parking Zone: %s", gr->name, gr->parking_zone);
                ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Your Parking", msg);
                on_park_yard_btn(NULL, ctx);
            }
            return;
        }
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Use 'Find' to locate your parking spot.");
        return;
    }
    int guest_id = gtk_spin_button_get_value_as_int(ctx->spin_guest);
    if (guest_id <= 0) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Enter a valid Guest ID"); return; }
    const char *plate = gtk_entry_get_text(ctx->entry_plate);
    if (!plate || !plate[0]) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Plate required"); return; }
    ParkingLot lot;
    parking_init(&lot, 10, 20, 12, 8);
    parking_load(&lot, PARKING_FILE);
    int spot = parking_assign(&lot, guest_id, plate, VEHICLE_CAR, ZONE_STANDARD);
    if (spot > 0) {
        parking_save(&lot, PARKING_FILE);
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Parking spot assigned.");
    } else {
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Full", "No available spots.");
    }
}

static void on_park_update(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Select a spot and use Search.");
}

static void on_park_delete(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    int id = gtk_spin_button_get_value_as_int(ctx->spin_guest);
    if (id <= 0) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Enter Guest ID to release"); return; }
    ParkingLot lot;
    parking_init(&lot, 10, 20, 12, 8);
    parking_load(&lot, PARKING_FILE);
    ParkingSpot *sp = parking_find_by_guest(&lot, id);
    if (sp) {
        parking_release(&lot, sp->spot_id);
        parking_save(&lot, PARKING_FILE);
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Released", "Parking spot released.");
    } else {
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "No parking found for this guest.");
    }
}

static void on_park_search(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;
    int id = atoi(gtk_entry_get_text(ctx->parking_search_guest));
    const GuestRecord *gr = guest_find_by_id(id);
    if (!gr) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "Guest ID not found.");
        return;
    }
    g_highlight_park_guest_id = id;
    char msg[256];
    g_snprintf(msg, sizeof(msg), "%s -> Parking Zone %s", gr->name, gr->parking_zone);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Parking Position", msg);
    on_park_yard_btn(NULL, ctx);
}

static void on_park_yard_close(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *win = GTK_WIDGET(d);
    gtk_widget_destroy(win);
}

static gboolean park_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)widget; (void)data;
    cairo_set_source_rgb(cr, 0.18, 0.18, 0.20);
    cairo_paint(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 14);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 16, 24);
    cairo_show_text(cr, "Parking Yard (50 spots)");

    const GuestRecord *highlight = guest_find_by_id(g_highlight_park_guest_id);

    /* Zone layout: A=10(2x5), B=15(3x5), C=15(3x5), D=10(2x5) = 50 */
    int zone_rows[] = {2, 3, 3, 2};
    int zone_cols[] = {5, 5, 5, 5};
    char zone_letters[] = {'A','B','C','D'};
    double colors[4][3] = {
        {0.85, 0.65, 0.10},
        {0.20, 0.55, 0.85},
        {0.30, 0.70, 0.30},
        {0.60, 0.40, 0.75}
    };

    int x0 = 16, y0 = 48, w = 62, h = 32, gap = 6;
    int spot_num = 1;

    for (int z = 0; z < 4; z++) {
        for (int r = 0; r < zone_rows[z]; r++) {
            for (int c = 0; c < zone_cols[z]; c++) {
                char zone[8];
                g_snprintf(zone, sizeof(zone), "%c%d", zone_letters[z], spot_num);
                int x = x0 + c * (w + gap) + z * 8;
                int y = y0 + (r + (z == 0 ? 0 : (z == 1 ? 2 : (z == 2 ? 5 : 8)))) * (h + gap) + 10;

                gboolean occupied = (spot_num <= g_guest_db_count);
                gboolean mine = (highlight && g_ascii_strcasecmp(highlight->parking_zone, zone) == 0);

                if (mine) {
                    cairo_set_source_rgb(cr, 0.10, 0.80, 0.20);
                } else if (occupied) {
                    cairo_set_source_rgb(cr, colors[z][0]*0.7, colors[z][1]*0.7, colors[z][2]*0.7);
                } else {
                    cairo_set_source_rgb(cr, colors[z][0], colors[z][1], colors[z][2]);
                }
                cairo_rectangle(cr, x, y, w, h);
                cairo_fill_preserve(cr);
                cairo_set_source_rgb(cr, 0.08, 0.08, 0.08);
                cairo_set_line_width(cr, 1);
                cairo_stroke(cr);

                cairo_set_source_rgb(cr, 1, 1, 1);
                cairo_set_font_size(cr, 10);
                cairo_move_to(cr, x + 6, y + 14);
                cairo_show_text(cr, zone);

                if (occupied) {
                    cairo_set_font_size(cr, 14);
                    cairo_move_to(cr, x + w - 22, y + 8);
                    cairo_show_text(cr, "tick");
                }

                if (mine) {
                    cairo_set_font_size(cr, 10);
                    cairo_move_to(cr, x + 6, y + h - 6);
                    cairo_show_text(cr, "YOU");
                }

                spot_num++;
            }
        }
    }

    /* Legend */
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 16, 450);
    cairo_show_text(cr, "Zones: A(10-VIP)  B(15-Standard)  C(15-Moto)  D(10-Bus)");
    return FALSE;
}

static void on_park_yard_find(GtkWidget *btn, gpointer d) {
    (void)btn;
    GtkWidget **widgets = (GtkWidget **)d;
    GtkEntry *entry = GTK_ENTRY(widgets[0]);
    GtkWidget *draw_area = widgets[1];

    const char *text = gtk_entry_get_text(entry);
    if (text && text[0]) {
        int id = atoi(text);
        const GuestRecord *gr = guest_find_by_id(id);
        if (gr) {
            g_highlight_park_guest_id = id;
        } else {
            const GuestRecord *gr2 = guest_find_by_name(text);
            if (gr2) g_highlight_park_guest_id = gr2->id;
            else g_highlight_park_guest_id = -1;
        }
    }
    gtk_widget_queue_draw(draw_area);
}

static void on_park_yard_btn(GtkWidget *w, gpointer d) {
    (void)w;
    ParkCtx *ctx = (ParkCtx *)d;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Parking Yard View");
    gtk_window_set_default_size(GTK_WINDOW(win), 600, 520);
    gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(ctx->main_win));
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("Guest ID/Name:"), FALSE, FALSE, 0);
    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "e.g. 3 or name");
    gtk_box_pack_start(GTK_BOX(top), search_entry, TRUE, TRUE, 0);
    GtkWidget *find_btn = gtk_button_new_with_label("Find");
    add_class(find_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(top), find_btn, FALSE, FALSE, 0);

    GtkWidget *draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(draw_area, 560, 420);
    gtk_box_pack_start(GTK_BOX(vbox), draw_area, TRUE, TRUE, 0);
    g_signal_connect(draw_area, "draw", G_CALLBACK(park_draw_cb), NULL);

    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(vbox), close_btn, FALSE, FALSE, 0);
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_park_yard_close), win);

    GtkWidget **widgets = g_malloc0(2 * sizeof(GtkWidget *));
    widgets[0] = search_entry;
    widgets[1] = draw_area;
    g_signal_connect(find_btn, "clicked", G_CALLBACK(on_park_yard_find), widgets);
    g_signal_connect(win, "destroy", G_CALLBACK(g_free), widgets);

    gtk_widget_show_all(win);
}

static GtkWidget *build_parking_tab(void) {
    ParkCtx *ctx = g_malloc0(sizeof(ParkCtx));
    g_park_ctx = ctx;
    ctx->selected_spot_id = -1;

    PageChrome ch = build_chrome(
        "Parking Management",
        "Manage parking spots, assign vehicles, and open the yard top-view map.",
        5);

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
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   SEATING TAB
   ========================================================= */
static void seat_refresh_tree(SeatCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    DiningHall hall;
    memset(&hall, 0, sizeof(hall));
    seating_load(&hall, SEATING_FILE);
    if (hall.table_count == 0) {
        for (int i = 1; i <= 10; i++) {
            TableType ttype = (i <= 2) ? TABLE_VIP : (i <= 6) ? TABLE_FAMILY : (i <= 9) ? TABLE_FRIENDS : TABLE_STAFF;
            char tname[16];
            g_snprintf(tname, sizeof(tname), "Table %d", i);
            seating_add_table(&hall, ttype, 8, tname);
        }
        seating_save(&hall, SEATING_FILE);
    }
    for (int i = 0; i < hall.table_count; i++) {
        DiningTable *t = &hall.tables[i];
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        gtk_list_store_set(ctx->store, &iter,
            0, t->table_id,
            1, t->name,
            2, t->capacity,
            -1);
    }
}

static void on_seat_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_seat_add(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    if (!g_admin_mode) {
        if (g_current_guest_id > 0) {
            g_highlight_seat_guest_id = g_current_guest_id;
            const GuestRecord *gr = guest_find_by_id(g_current_guest_id);
            if (gr) {
                char msg[256];
                g_snprintf(msg, sizeof(msg), "%s -> Table: %s", gr->name, gr->seat_code);
                ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Your Seat", msg);
                on_hall_btn(NULL, ctx);
            }
            return;
        }
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Use 'Find' to locate your seat.");
        return;
    }
    const char *tname = gtk_entry_get_text(ctx->entry_table_name);
    int cap = gtk_spin_button_get_value_as_int(ctx->spin_table_capacity);
    if (!tname || !tname[0]) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Table name required"); return; }
    DiningHall hall;
    memset(&hall, 0, sizeof(hall));
    seating_load(&hall, SEATING_FILE);
    int tid = seating_add_table(&hall, TABLE_FAMILY, cap, tname);
    if (tid > 0) {
        seating_save(&hall, SEATING_FILE);
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Table added.");
        seat_refresh_tree(ctx);
    } else {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Could not add table.");
    }
}

static void on_seat_update(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Select a table and use the Hall view.");
}

static void on_seat_delete(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Use the Hall view to manage tables.");
}

static void on_seat_search(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;
    int id = atoi(gtk_entry_get_text(ctx->seating_search_guest));
    const GuestRecord *gr = guest_find_by_id(id);
    if (!gr) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Not Found", "Guest ID not found.");
        return;
    }
    g_highlight_seat_guest_id = id;
    char msg[256];
    g_snprintf(msg, sizeof(msg), "%s -> Seat %s", gr->name, gr->seat_code);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Seating Position", msg);
    on_hall_btn(NULL, ctx);
}

static void on_hall_close(GtkWidget *w, gpointer d) {
    (void)w;
    GtkWidget *win = GTK_WIDGET(d);
    gtk_widget_destroy(win);
}

typedef struct {
    const char *label;
    const char *tables[10];
    int count;
    double r, g, b;
} TableGroup;

static void draw_rounded_rect(cairo_t *cr, double x, double y, double w, double h, double r) {
    cairo_move_to(cr, x + r, y);
    cairo_line_to(cr, x + w - r, y);
    cairo_curve_to(cr, x + w, y, x + w, y, x + w, y + r);
    cairo_line_to(cr, x + w, y + h - r);
    cairo_curve_to(cr, x + w, y + h, x + w, y + h, x + w - r, y + h);
    cairo_line_to(cr, x + r, y + h);
    cairo_curve_to(cr, x, y + h, x, y + h, x, y + h - r);
    cairo_line_to(cr, x, y + r);
    cairo_curve_to(cr, x, y, x, y, x + r, y);
    cairo_close_path(cr);
}

static gboolean hall_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)widget; (void)data;
    const GuestRecord *highlight = guest_find_by_id(g_highlight_seat_guest_id);

    cairo_set_source_rgb(cr, 0.93, 0.95, 0.98);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0.86, 0.26, 0.20);
    draw_rounded_rect(cr, 260, 20, 200, 45, 8);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, 340, 48);
    cairo_show_text(cr, "STAGE");

    TableGroup groups[] = {
        {"VIP Tables", {"T1","T2"}, 2, 0.98, 0.79, 0.13},
        {"Family Tables", {"T3","T4","T5","T6"}, 4, 0.20, 0.70, 0.30},
        {"Friends Tables", {"T7","T8","T9"}, 3, 0.30, 0.50, 0.90},
        {"Staff Table", {"T10"}, 1, 0.60, 0.60, 0.62},
    };

    double tw = 110, th = 60, gap = 12;
    double start_y = 85;
    int table_idx = 1;

    for (int g = 0; g < 4; g++) {
        double y = start_y + g * (th + gap + 18);
        cairo_set_source_rgb(cr, 0.30, 0.30, 0.30);
        cairo_set_font_size(cr, 11);
        cairo_move_to(cr, 16, y + 14);
        cairo_show_text(cr, groups[g].label);

        int n = groups[g].count;
        double total_w = n * tw + (n - 1) * gap;
        double start_x = (680 - total_w) / 2.0;

        for (int t = 0; t < n; t++, table_idx++) {
            double x = start_x + t * (tw + gap);
            char tcode[8];
            g_snprintf(tcode, sizeof(tcode), "T%d", table_idx);

            gboolean is_highlight = (highlight && g_str_has_prefix(highlight->seat_code, tcode));

            if (is_highlight) {
                cairo_set_source_rgb(cr, 0.20, 0.70, 0.30);
            } else {
                cairo_set_source_rgb(cr, groups[g].r, groups[g].g, groups[g].b);
            }
            draw_rounded_rect(cr, x, y, tw, th, 6);
            cairo_fill_preserve(cr);
            cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
            cairo_set_line_width(cr, 1.5);
            cairo_stroke(cr);

            cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
            cairo_set_font_size(cr, 13);
            cairo_move_to(cr, x + tw/2 - 10, y + th/2 + 5);
            cairo_show_text(cr, tcode);

            if (is_highlight) {
                cairo_set_source_rgb(cr, 1, 1, 1);
                cairo_set_font_size(cr, 10);
                cairo_move_to(cr, x + 8, y + th - 8);
                cairo_show_text(cr, "HERE");
            }
        }
    }

    return FALSE;
}

static void on_hall_find_clicked(GtkWidget *btn, gpointer d) {
    (void)btn;
    GtkWidget **widgets = (GtkWidget **)d;
    GtkEntry *guest_entry = GTK_ENTRY(widgets[0]);
    GtkComboBoxText *table_combo = GTK_COMBO_BOX_TEXT(widgets[1]);
    GtkWidget *draw_area = widgets[2];

    const char *guest_text = gtk_entry_get_text(guest_entry);
    const char *table_text = gtk_combo_box_text_get_active_text(table_combo);

    if (guest_text && guest_text[0]) {
        int id = atoi(guest_text);
        const GuestRecord *gr = guest_find_by_id(id);
        if (gr) {
            g_highlight_seat_guest_id = id;
        } else {
            g_highlight_seat_guest_id = -1;
        }
    } else if (table_text && table_text[0]) {
        for (int i = 0; i < g_guest_db_count; i++) {
            if (g_ascii_strcasecmp(g_guest_db[i].seat_code, table_text) == 0) {
                g_highlight_seat_guest_id = g_guest_db[i].id;
                break;
            }
            g_highlight_seat_guest_id = -1;
        }
    }
    gtk_widget_queue_draw(draw_area);
}

static void on_hall_btn(GtkWidget *w, gpointer d) {
    (void)w;
    SeatCtx *ctx = (SeatCtx *)d;

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Ceremony Hall Layout");
    gtk_window_set_default_size(GTK_WINDOW(win), 760, 580);
    gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(ctx->main_win));
    gtk_window_set_modal(GTK_WINDOW(win), TRUE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("Guest ID:"), FALSE, FALSE, 0);
    GtkWidget *guest_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(guest_entry), "e.g. 2");
    gtk_widget_set_size_request(guest_entry, 80, -1);
    gtk_box_pack_start(GTK_BOX(top), guest_entry, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(top), gtk_label_new("Select Table:"), FALSE, FALSE, 0);
    GtkWidget *table_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(table_combo), "-- All Tables --");
    for (int i = 1; i <= 10; i++) {
        char label[8];
        g_snprintf(label, sizeof(label), "T%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(table_combo), label);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(table_combo), 0);
    gtk_box_pack_start(GTK_BOX(top), table_combo, FALSE, FALSE, 0);

    GtkWidget *find_btn = gtk_button_new_with_label("Find My Seat");
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

    GtkWidget **widgets = g_malloc0(3 * sizeof(GtkWidget *));
    widgets[0] = guest_entry;
    widgets[1] = table_combo;
    widgets[2] = draw_area;
    g_signal_connect(find_btn, "clicked", G_CALLBACK(on_hall_find_clicked), widgets);
    g_signal_connect(win, "destroy", G_CALLBACK(g_free), widgets);

    gtk_widget_show_all(win);
}

static GtkWidget *build_seating_tab(void) {
    SeatCtx *ctx = g_malloc0(sizeof(SeatCtx));
    g_seat_ctx = ctx;

    PageChrome ch = build_chrome(
        "Seating Management",
        "Manage table assignments, guest placement, and open the ceremony hall top-view map.",
        7);

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
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   SCHEDULE TAB
   ========================================================= */
static void sched_refresh_tree(SchedCtx *ctx) {
    if (!ctx || !ctx->store) return;
    gtk_list_store_clear(ctx->store);
    WeddingSchedule sched;
    schedule_init(&sched, "Wedding Day", "Venue");
    schedule_load(&sched, SCHEDULE_FILE);
    for (EventNode *e = sched.head; e; e = e->next) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->store, &iter);
        char time_buf[32];
        schedule_format_hhmm(e->scheduled_start, time_buf, sizeof(time_buf));
        gtk_list_store_set(ctx->store, &iter,
            0, e->event_id,
            1, e->title,
            2, time_buf,
            -1);
    }
    schedule_free(&sched);
}

static void on_sched_select(GtkTreeSelection *sel, gpointer d) {
    (void)sel; (void)d;
}

static void on_sched_add(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    if (!g_admin_mode) {
        WeddingSchedule sched;
        schedule_init(&sched, "Wedding Day", "Venue");
        schedule_load(&sched, SCHEDULE_FILE);
        char buf[1024] = "";
        for (EventNode *e = sched.head; e; e = e->next) {
            char time_buf[16], line[128];
            schedule_format_hhmm(e->scheduled_start, time_buf, sizeof(time_buf));
            g_snprintf(line, sizeof(line), "%s - %s\n", time_buf, e->title);
            g_strlcat(buf, line, sizeof(buf));
        }
        if (buf[0]) ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Event Schedule", buf);
        else ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Schedule", "No events scheduled yet.");
        schedule_free(&sched);
        return;
    }
    const char *title = gtk_entry_get_text(ctx->entry_title);
    if (!title || !title[0]) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Title required"); return; }
    WeddingSchedule sched;
    schedule_init(&sched, "Wedding Day", "Venue");
    schedule_load(&sched, SCHEDULE_FILE);
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    tm->tm_hour = gtk_spin_button_get_value_as_int(ctx->spin_start_h);
    tm->tm_min = gtk_spin_button_get_value_as_int(ctx->spin_start_m);
    time_t start = mktime(tm);
    EventNode *e = schedule_add_event(&sched, title, "", EVT_CEREMONY, PRIO_NORMAL, start, 60, "", "");
    if (e) {
        schedule_save(&sched, SCHEDULE_FILE);
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Success", "Event added.");
        sched_refresh_tree(ctx);
    } else {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Could not add event.");
    }
    schedule_free(&sched);
}

static void on_sched_update(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Info", "Select an event and update its details.");
}

static void on_sched_delete(GtkWidget *w, gpointer d) {
    (void)w;
    SchedCtx *ctx = (SchedCtx *)d;
    if (ctx->selected_event_id <= 0) { ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Select an event first"); return; }
    WeddingSchedule sched;
    schedule_init(&sched, "Wedding Day", "Venue");
    schedule_load(&sched, SCHEDULE_FILE);
    if (schedule_remove_event(&sched, ctx->selected_event_id)) {
        schedule_save(&sched, SCHEDULE_FILE);
        ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Deleted", "Event removed.");
        sched_refresh_tree(ctx);
    } else {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Could not delete event.");
    }
    schedule_free(&sched);
}

static GtkWidget *build_schedule_tab(void) {
    SchedCtx *ctx = g_malloc0(sizeof(SchedCtx));
    g_sched_ctx = ctx;
    ctx->selected_event_id = -1;

    PageChrome ch = build_chrome(
        "Event Schedule",
        "View and manage the wedding timeline with categories, priorities, timings, and delays.",
        6);

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
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
}

/* =========================================================
   INVITATION TAB
   ========================================================= */
static void draw_shadow_text(cairo_t *cr, double x, double y, const char *text,
                              double r, double g, double b, double font_size,
                              cairo_font_weight_t weight) {
    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, weight);
    cairo_set_font_size(cr, font_size);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_move_to(cr, x + 1, y + 1);
    cairo_show_text(cr, text);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
}

static void draw_text_italic(cairo_t *cr, double x, double y, const char *text,
                              double font_size, cairo_font_weight_t weight) {
    cairo_select_font_face(cr, "Georgia", CAIRO_FONT_SLANT_ITALIC, weight);
    cairo_set_font_size(cr, font_size);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
}

static void paint_background(cairo_t *cr, int W, int H) {
    cairo_surface_t *bg = cairo_image_surface_create_from_png("Invitation.png");
    if (cairo_surface_status(bg) == CAIRO_STATUS_SUCCESS) {
        double sx = (double)W / cairo_image_surface_get_width(bg);
        double sy = (double)H / cairo_image_surface_get_height(bg);
        cairo_save(cr);
        cairo_scale(cr, sx, sy);
        cairo_set_source_surface(cr, bg, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    } else {
        cairo_set_source_rgb(cr, 0.95, 0.92, 0.85);
        cairo_paint(cr);
    }
    cairo_surface_destroy(bg);
}

static gboolean invite_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)widget;
    InviteCtx *ctx = (InviteCtx *)data;
    if (!ctx) return FALSE;

    int W = 680, H = 460;

    paint_background(cr, W, H);

    draw_text_italic(cr, 180, 65, "Bryan & Danielle", 32, CAIRO_FONT_WEIGHT_BOLD);
    draw_text_italic(cr, 230, 95, "Wedding Invitation", 18, CAIRO_FONT_WEIGHT_NORMAL);
    draw_text_italic(cr, 180, 160, "You are cordially invited to celebrate", 16, CAIRO_FONT_WEIGHT_NORMAL);
    draw_text_italic(cr, 250, 190, "the marriage of", 20, CAIRO_FONT_WEIGHT_NORMAL);
    draw_shadow_text(cr, 160, 235, "Bryan & Danielle",
                     0.15, 0.10, 0.08, 30, CAIRO_FONT_WEIGHT_BOLD);

    int first_selected = -1;
    for (int i = 0; i < g_guest_db_count && i < 64; i++) {
        if (ctx->selected_ids[i]) { first_selected = i; break; }
    }

    char guest_label[128];

    draw_text_italic(cr, 60, 295, "Guest Information:", 20, CAIRO_FONT_WEIGHT_BOLD);

    if (first_selected >= 0) {
        g_snprintf(guest_label, sizeof(guest_label), "Name    :  %s", g_guest_db[first_selected].name);
        draw_text_italic(cr, 60, 330, guest_label, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(guest_label, sizeof(guest_label), "Table   :  %s", g_guest_db[first_selected].seat_code);
        draw_text_italic(cr, 60, 355, guest_label, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(guest_label, sizeof(guest_label), "Parking :  %s", g_guest_db[first_selected].parking_zone);
        draw_text_italic(cr, 60, 380, guest_label, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(guest_label, sizeof(guest_label), "Category:  %s", g_guest_db[first_selected].category);
        draw_text_italic(cr, 60, 405, guest_label, 17, CAIRO_FONT_WEIGHT_NORMAL);
    }

    draw_text_italic(cr, 60, 440, "Date: Wedding Day  |  Venue: To be announced", 12, CAIRO_FONT_WEIGHT_NORMAL);

    return FALSE;
}

static void on_invite_toggled(GtkCellRendererToggle *renderer, gchar *path_str, gpointer d) {
    (void)renderer;
    InviteCtx *ctx = (InviteCtx *)d;
    int row = atoi(path_str);
    if (row >= 0 && row < g_guest_db_count && row < 64) {
        ctx->selected_ids[row] = !ctx->selected_ids[row];
        GtkTreeIter iter;
        GtkTreeModel *model = GTK_TREE_MODEL(ctx->guest_store);
        if (gtk_tree_model_get_iter_from_string(model, &iter, path_str)) {
            gtk_list_store_set(ctx->guest_store, &iter, 0, ctx->selected_ids[row], -1);
        }
        gtk_widget_queue_draw(ctx->draw_area);
    }
}

static void on_invite_send(GtkWidget *w, gpointer d) {
    (void)w;
    InviteCtx *ctx = (InviteCtx *)d;

    int selected_count = 0;
    for (int i = 0; i < g_guest_db_count && i < 64; i++) {
        if (ctx->selected_ids[i]) selected_count++;
    }

    if (selected_count == 0) {
        ui_show_error_dialog(GTK_WINDOW(ctx->main_win), "Error", "Check at least one guest from the list first.");
        return;
    }

    int sent_count = 0;
    for (int i = 0; i < g_guest_db_count && i < 64; i++) {
        if (!ctx->selected_ids[i]) continue;

        int guest_id = g_guest_db[i].id;
        const char *guest_name = g_guest_db[i].name;
        const char *guest_email = g_guest_db[i].email;
        const char *table_pos = g_guest_db[i].seat_code;
        const char *park_zone = g_guest_db[i].parking_zone;

        char img_path[256];
        g_snprintf(img_path, sizeof(img_path), "invitation_%d.png", guest_id);

        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 680, 460);
        cairo_t *cr = cairo_create(surface);

        paint_background(cr, 680, 460);

        draw_text_italic(cr, 180, 65, "Bryan & Danielle", 32, CAIRO_FONT_WEIGHT_BOLD);
        draw_text_italic(cr, 230, 95, "Wedding Invitation", 18, CAIRO_FONT_WEIGHT_NORMAL);
        draw_text_italic(cr, 180, 160, "You are cordially invited to celebrate", 16, CAIRO_FONT_WEIGHT_NORMAL);
        draw_text_italic(cr, 250, 190, "the marriage of", 20, CAIRO_FONT_WEIGHT_NORMAL);
        draw_shadow_text(cr, 160, 235, "Bryan & Danielle",
                         0.15, 0.10, 0.08, 30, CAIRO_FONT_WEIGHT_BOLD);

        draw_text_italic(cr, 60, 295, "Guest Information:", 20, CAIRO_FONT_WEIGHT_BOLD);

        char line[256];
        g_snprintf(line, sizeof(line), "Name    :  %s", guest_name);
        draw_text_italic(cr, 60, 330, line, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(line, sizeof(line), "Table   :  %s", table_pos);
        draw_text_italic(cr, 60, 355, line, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(line, sizeof(line), "Parking :  %s", park_zone);
        draw_text_italic(cr, 60, 380, line, 17, CAIRO_FONT_WEIGHT_NORMAL);
        g_snprintf(line, sizeof(line), "Category:  %s", g_guest_db[i].category);
        draw_text_italic(cr, 60, 405, line, 17, CAIRO_FONT_WEIGHT_NORMAL);

        draw_text_italic(cr, 60, 440, "Date: Wedding Day  |  Venue: To be announced", 12, CAIRO_FONT_WEIGHT_NORMAL);

        cairo_surface_write_to_png(surface, img_path);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);

        if (guest_email && guest_email[0]) {
            char cmd[1024];
#ifdef _WIN32
            g_snprintf(cmd, sizeof(cmd), "start mailto:%s?subject=Wedding Invitation - Bryan & Danielle&body=Dear %s,%%0D%%0A%%0D%%0APlease find attached your wedding invitation.%%0D%%0A%%0D%%0ATable: %s%%0D%%0AParking: %s",
                       guest_email, guest_name, table_pos, park_zone);
#else
            g_snprintf(cmd, sizeof(cmd), "xdg-email \"%s\" --subject \"Wedding Invitation - Bryan & Danielle\" --attach \"%s\" --body \"Dear %s,\\n\\nPlease find attached your wedding invitation.\\n\\nTable: %s\\nParking: %s\"",
                       guest_email, img_path, guest_name, table_pos, park_zone);
#endif
            system(cmd);
        }
        sent_count++;
    }

    char result[256];
    g_snprintf(result, sizeof(result), "Processed %d invitation(s).\nImages saved as invitation_ID.png\nEmail client opened for guests with valid email.", sent_count);
    ui_show_info_dialog(GTK_WINDOW(ctx->main_win), "Invitations Sent", result);
}

static GtkWidget *build_invitation_tab(void) {
    InviteCtx *ctx = g_malloc0(sizeof(InviteCtx));
    g_invite_ctx = ctx;

    PageChrome ch = build_chrome(
        "Invitation Manager",
        "Check guests from the list and send their wedding invitations.",
        8);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 10);

    /* Left: guest checklist */
    GtkWidget *left_frame = gtk_frame_new("Guest List (check to select)");
    GtkWidget *left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(left_frame), left_vbox);

    ctx->guest_store = gtk_list_store_new(2, G_TYPE_BOOLEAN, G_TYPE_STRING);
    for (int i = 0; i < g_guest_db_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(ctx->guest_store, &iter);
        char label[128];
        g_snprintf(label, sizeof(label), "%d - %s", g_guest_db[i].id, g_guest_db[i].name);
        gtk_list_store_set(ctx->guest_store, &iter, 0, FALSE, 1, label, -1);
    }

    ctx->guest_tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(ctx->guest_store)));
    GtkCellRenderer *toggle_renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(toggle_renderer, "toggled", G_CALLBACK(on_invite_toggled), ctx);
    GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Select", toggle_renderer, "active", 0, NULL);
    gtk_tree_view_append_column(ctx->guest_tree, col);

    GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes("Guest", text_renderer, "text", 1, NULL);
    gtk_tree_view_append_column(ctx->guest_tree, col);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 300);
    gtk_container_add(GTK_CONTAINER(scroll), GTK_WIDGET(ctx->guest_tree));
    gtk_box_pack_start(GTK_BOX(left_vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *send_btn = gtk_button_new_with_label("Generate & Send Invitations");
    add_class(send_btn, "btn-primary");
    gtk_box_pack_start(GTK_BOX(left_vbox), send_btn, FALSE, FALSE, 0);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(on_invite_send), ctx);

    gtk_box_pack_start(GTK_BOX(hbox), left_frame, TRUE, TRUE, 0);

    /* Right: invitation preview */
    GtkWidget *right_frame = gtk_frame_new("Invitation Preview");
    ctx->draw_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(ctx->draw_area, 680, 460);
    gtk_container_add(GTK_CONTAINER(right_frame), ctx->draw_area);
    g_signal_connect(ctx->draw_area, "draw", G_CALLBACK(invite_draw_cb), ctx);
    gtk_box_pack_start(GTK_BOX(hbox), right_frame, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(ch.content), hbox, TRUE, TRUE, 0);
    return ch.shell;
}
/* =========================================================
   STATS/DASHBOARD TAB
   ========================================================= */
static void stats_refresh(StatsCtx *ctx) {
    if (!ctx) return;
    int guests = g_guest_db_count;
    int gifts = g_gift_db_count;
    double total = 0.0;
    for (int i = 0; i < g_gift_db_count; i++) total += g_gift_db[i].value;

    int unique_cats = 0;
    for (int i = 0; i < g_guest_db_count; i++) {
        gboolean seen = FALSE;
        for (int j = 0; j < i; j++) {
            if (g_ascii_strcasecmp(g_guest_db[i].category, g_guest_db[j].category) == 0) {
                seen = TRUE;
                break;
            }
        }
        if (!seen) unique_cats++;
    }

    char buf[64];
    g_snprintf(buf, sizeof(buf), "%d", unique_cats);
    gtk_label_set_text(ctx->lbl_cats, buf);
    g_snprintf(buf, sizeof(buf), "%d", guests);
    gtk_label_set_text(ctx->lbl_guests, buf);
    g_snprintf(buf, sizeof(buf), "%d", gifts);
    gtk_label_set_text(ctx->lbl_gifts, buf);
    g_snprintf(buf, sizeof(buf), "%.2f FCFA", total);
    gtk_label_set_text(ctx->lbl_total, buf);
}

static void on_stats_refresh_clicked(GtkWidget *w, gpointer d) {
    (void)w;
    stats_refresh((StatsCtx *)d);
}

static GtkWidget *build_stats_tab(void) {
    StatsCtx *ctx = g_malloc0(sizeof(StatsCtx));
    g_stats_ctx = ctx;

    PageChrome ch = build_chrome(
        "Dashboard",
        "Overview statistics for categories, guests, gifts, and total gift value.",
        9);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);

    /* Title */
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>Dashboard</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    /* Stat cards grid */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 18);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 18);
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 0);

    /* Card 1: Total Guests */
    GtkWidget *card1 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card1), GTK_SHADOW_OUT);
    add_class(card1, "card");
    gtk_widget_set_size_request(card1, 260, 130);
    GtkWidget *vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), 12);
    gtk_container_add(GTK_CONTAINER(card1), vbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), gtk_label_new("Total Guests"), FALSE, FALSE, 0);
    ctx->lbl_guests = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_guests), "stat-value");
    gtk_label_set_xalign(ctx->lbl_guests, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox1), GTK_WIDGET(ctx->lbl_guests), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card1, 0, 0, 1, 1);

    /* Card 2: Categories */
    GtkWidget *card2 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card2), GTK_SHADOW_OUT);
    add_class(card2, "card");
    gtk_widget_set_size_request(card2, 260, 130);
    GtkWidget *vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), 12);
    gtk_container_add(GTK_CONTAINER(card2), vbox2);
    gtk_box_pack_start(GTK_BOX(vbox2), gtk_label_new("Categories"), FALSE, FALSE, 0);
    ctx->lbl_cats = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_cats), "stat-value");
    gtk_label_set_xalign(ctx->lbl_cats, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox2), GTK_WIDGET(ctx->lbl_cats), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card2, 1, 0, 1, 1);

    /* Card 3: Gifts */
    GtkWidget *card3 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card3), GTK_SHADOW_OUT);
    add_class(card3, "card");
    gtk_widget_set_size_request(card3, 260, 130);
    GtkWidget *vbox3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox3), 12);
    gtk_container_add(GTK_CONTAINER(card3), vbox3);
    gtk_box_pack_start(GTK_BOX(vbox3), gtk_label_new("Gifts"), FALSE, FALSE, 0);
    ctx->lbl_gifts = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_gifts), "stat-value");
    gtk_label_set_xalign(ctx->lbl_gifts, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox3), GTK_WIDGET(ctx->lbl_gifts), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card3, 0, 1, 1, 1);

    /* Card 4: Total Gift Value */
    GtkWidget *card4 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(card4), GTK_SHADOW_OUT);
    add_class(card4, "card");
    gtk_widget_set_size_request(card4, 260, 130);
    GtkWidget *vbox4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox4), 12);
    gtk_container_add(GTK_CONTAINER(card4), vbox4);
    gtk_box_pack_start(GTK_BOX(vbox4), gtk_label_new("Gift Value"), FALSE, FALSE, 0);
    ctx->lbl_total = GTK_LABEL(gtk_label_new("-"));
    gtk_widget_set_name(GTK_WIDGET(ctx->lbl_total), "stat-value");
    gtk_label_set_xalign(ctx->lbl_total, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox4), GTK_WIDGET(ctx->lbl_total), TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(grid), card4, 1, 1, 1, 1);

    /* Refresh button */
    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh Stats");
    gtk_box_pack_start(GTK_BOX(vbox), refresh_btn, FALSE, FALSE, 0);
    add_class(refresh_btn, "btn-primary");
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(on_stats_refresh_clicked), ctx);

    stats_refresh(ctx);
    gtk_box_pack_start(GTK_BOX(ch.content), vbox, TRUE, TRUE, 0);
    return ch.shell;
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
    guest_db_load();
    registrations_load();
    gift_load(g_gift_db, &g_gift_db_count, GIFTS_FILE);

    if (!show_welcome_role_dialog()) {
        return;
    }

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
    if (g_invite_ctx) g_invite_ctx->main_win = win;

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
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_invitation_tab(), gtk_label_new("Invitation"));
    gtk_notebook_append_page(GTK_NOTEBOOK(g_notebook), build_stats_tab(), gtk_label_new("Dashboard"));

    /* Contexts are allocated while building tabs; bind window after creation. */
    if (g_home_ctx) g_home_ctx->main_win = win;
    if (g_cat_ctx) g_cat_ctx->main_win = win;
    if (g_person_ctx) g_person_ctx->main_win = win;
    if (g_prio_ctx) g_prio_ctx->main_win = win;
    if (g_gift_ctx) g_gift_ctx->main_win = win;
    if (g_park_ctx) g_park_ctx->main_win = win;
    if (g_seat_ctx) g_seat_ctx->main_win = win;
    if (g_sched_ctx) g_sched_ctx->main_win = win;
    if (g_invite_ctx) g_invite_ctx->main_win = win;

    gtk_widget_show_all(win);
    gtk_main();
}
