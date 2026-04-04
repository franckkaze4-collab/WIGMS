#ifndef SEATING_H
#define SEATING_H

/*
 * ============================================================
 *  MODULE 7 - Meal & Table Seating Management
 * ============================================================
 */

#include "types.h"

typedef enum {
    TABLE_VIP = 0,
    TABLE_FAMILY,
    TABLE_FRIENDS,
    TABLE_CHILDREN,
    TABLE_STAFF,
    TABLE_TYPE_COUNT
} TableType;

typedef enum {
    MEAL_MEAT = 0,
    MEAL_FISH,
    MEAL_VEGETARIAN,
    MEAL_KIDS,
    MEAL_NOT_SET
} MealPreference;

typedef enum {
    DIET_NONE = 0,
    DIET_HALAL,
    DIET_KOSHER,
    DIET_ALLERGY,
    DIET_DIABETIC
} DietaryRestriction;

typedef enum {
    RSVP_PENDING = 0,
    RSVP_CONFIRMED,
    RSVP_DECLINED,
    RSVP_NO_SHOW
} RsvpStatus;

typedef struct {
    int                seat_id;
    int                table_id;
    int                seat_number;  /* 1..capacity */
    int                guest_id;     /* -1 if empty */
    char               guest_name[100];
    MealPreference     meal;
    DietaryRestriction diet;
    RsvpStatus         rsvp;
    char               notes[100];
} SeatAssignment;

#define MAX_SEATS_PER_TABLE 12
#define MAX_TABLES 30

typedef struct {
    int            table_id;
    int            table_number;
    TableType      type;
    char           name[50];
    int            capacity;
    int            assigned_count;
    SeatAssignment seats[MAX_SEATS_PER_TABLE];
} DiningTable;

typedef struct {
    DiningTable tables[MAX_TABLES];
    int         table_count;
    int         total_capacity;
} DiningHall;

typedef struct {
    int meat_count;
    int fish_count;
    int vegetarian_count;
    int kids_count;
    int halal_count;
    int allergy_count;
} MealSummary;

int seating_add_table(DiningHall *hall, TableType type,
                      int capacity, const char *name);

int seating_assign_guest(DiningHall *hall,
                         int table_id, int guest_id,
                         const char *guest_name,
                         MealPreference meal,
                         DietaryRestriction diet);

int seating_move_guest(DiningHall *hall, int guest_id, int new_table_id);
int seating_remove_guest(DiningHall *hall, int guest_id);
int seating_update_rsvp(DiningHall *hall, int guest_id, RsvpStatus rsvp);

DiningTable*    seating_find_table(DiningHall *hall, int table_id);
SeatAssignment* seating_find_guest(DiningHall *hall, int guest_id);
int             seating_guest_table_id(DiningHall *hall, int guest_id);
MealSummary     seating_meal_summary(DiningHall *hall);

void seating_save(DiningHall *hall, const char *path);
void seating_load(DiningHall *hall, const char *path);

const char* seating_table_type_name(TableType t);
const char* seating_meal_name(MealPreference m);
const char* seating_diet_name(DietaryRestriction d);
const char* seating_rsvp_name(RsvpStatus r);

extern int seating_table_id_counter;
extern int seating_seat_id_counter;

#endif /* SEATING_H */

