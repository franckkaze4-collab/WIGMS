#ifndef UI_CONSOLE_H
#define UI_CONSOLE_H

#include "types.h"
#include "person.h"
#include "category.h"
#include "priority.h"
#include "gift.h"
#include "parking.h"
#include "seating.h"
#include "schedule.h"

#define MAX_GUEST_DB 200

typedef struct {
    int  id;
    char name[100];
    int  age;
    char category[50];
    char social_class[50];
    char side[20];
    char seat_code[20];
    char parking_zone[10];
} GuestRecord;

typedef struct {
    Category       *cat_head;
    Gift            gifts[MAX_GIFTS];
    int             gift_count;
    ParkingLot      parking;
    DiningHall      dining;
    WeddingSchedule schedule;
    GuestRecord     guest_db[MAX_GUEST_DB];
    int             guest_db_count;

    /* New fields for extended guest management */
    RegisteredGuest reg_guests[MAX_REGISTERED];
    int             reg_guest_count;
    GiftCatalogItem gift_catalog[MAX_GIFT_CATALOG];
    int             gift_catalog_count;
    GiftPurchase    gift_purchases[MAX_GIFT_PURCHASES];
    int             gift_purchase_count;
} AppState;

void ui_console_run(AppState *state, int is_admin);
void guest_db_load(AppState *state);
void init_gift_catalog(AppState *state);
void save_registered_guests(AppState *state);
void load_registered_guests(AppState *state);
void save_gift_purchases(AppState *state);
void load_gift_purchases(AppState *state);

#endif
