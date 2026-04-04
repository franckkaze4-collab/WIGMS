#ifndef UI_GTK_H
#define UI_GTK_H

#include <gtk/gtk.h>
#include "types.h"
#include "parking.h"
#include "seating.h"
#include "schedule.h"

/* ============================================================
   MODULE 5 - GTK Graphical User Interface
   ui_gtk.h : public declarations
   ============================================================ */

typedef struct {
    Category *cat_head;          /* head of category linked list */
    Gift      gifts[MAX_GIFTS];  /* flat gift array */
    int       gift_count;        /* number of active gifts */

    /* Modules 6-8 */
    ParkingLot      parking;
    DiningHall      dining;
    WeddingSchedule schedule;
} AppState;

/* Entry point for the GTK GUI */
void ui_gtk_run(int *argc, char ***argv, AppState *state);

/* Shared dialog helpers */
void     ui_apply_css(void);
void     ui_show_info_dialog(GtkWindow *parent, const char *title, const char *msg);
void     ui_show_error_dialog(GtkWindow *parent, const char *title, const char *msg);
gboolean ui_confirm_dialog(GtkWindow *parent, const char *question);

#endif /* UI_GTK_H */

