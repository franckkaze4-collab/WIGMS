#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "seating.h"

int seating_table_id_counter = 1;
int seating_seat_id_counter  = 1;

static const char *TABLE_TYPE_NAMES[] = {
    "VIP / Head Table",
    "Family",
    "Friends",
    "Children",
    "Staff"
};
static const char *MEAL_NAMES[] = {
    "Meat", "Fish", "Vegetarian", "Kids Meal", "Not Set"
};
static const char *DIET_NAMES[] = {
    "None", "Halal", "Kosher", "Allergy", "Diabetic"
};
static const char *RSVP_NAMES[] = {
    "Pending", "Confirmed", "Declined", "No-Show"
};

const char* seating_table_type_name(TableType t) {
    if (t < 0 || t >= TABLE_TYPE_COUNT) return "Unknown";
    return TABLE_TYPE_NAMES[t];
}
const char* seating_meal_name(MealPreference m) {
    if (m < 0 || m > MEAL_NOT_SET) return "Unknown";
    return MEAL_NAMES[m];
}
const char* seating_diet_name(DietaryRestriction d) {
    if (d < 0 || d > DIET_DIABETIC) return "Unknown";
    return DIET_NAMES[d];
}
const char* seating_rsvp_name(RsvpStatus r) {
    if (r < 0 || r > RSVP_NO_SHOW) return "Unknown";
    return RSVP_NAMES[r];
}

static void safe_copy(char *dst, size_t dst_sz, const char *src) {
    if (!dst || dst_sz == 0) return;
    if (!src) src = "";
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

static void seat_init(SeatAssignment *s, int table_id, int seat_number) {
    memset(s, 0, sizeof(*s));
    s->seat_id = seating_seat_id_counter++;
    s->table_id = table_id;
    s->seat_number = seat_number;
    s->guest_id = -1;
    s->guest_name[0] = '\0';
    s->meal = MEAL_NOT_SET;
    s->diet = DIET_NONE;
    s->rsvp = RSVP_PENDING;
    s->notes[0] = '\0';
}

int seating_add_table(DiningHall *hall, TableType type,
                      int capacity, const char *name) {
    if (!hall) return -1;
    if (hall->table_count >= MAX_TABLES) return -1;
    if (capacity < 1) capacity = 1;
    if (capacity > MAX_SEATS_PER_TABLE) capacity = MAX_SEATS_PER_TABLE;

    DiningTable *t = &hall->tables[hall->table_count];
    memset(t, 0, sizeof(*t));
    t->table_id = seating_table_id_counter++;
    t->table_number = hall->table_count + 1;
    t->type = type;
    t->capacity = capacity;
    t->assigned_count = 0;
    safe_copy(t->name, sizeof(t->name), name && name[0] ? name : "Table");

    for (int i = 0; i < capacity; i++) {
        seat_init(&t->seats[i], t->table_id, i + 1);
    }

    hall->table_count++;
    hall->total_capacity += capacity;

    return t->table_id;
}

DiningTable* seating_find_table(DiningHall *hall, int table_id) {
    if (!hall) return NULL;
    for (int i = 0; i < hall->table_count; i++) {
        if (hall->tables[i].table_id == table_id) return &hall->tables[i];
    }
    return NULL;
}

SeatAssignment* seating_find_guest(DiningHall *hall, int guest_id) {
    if (!hall) return NULL;
    for (int t = 0; t < hall->table_count; t++) {
        DiningTable *tbl = &hall->tables[t];
        for (int s = 0; s < tbl->capacity; s++) {
            if (tbl->seats[s].guest_id == guest_id) return &tbl->seats[s];
        }
    }
    return NULL;
}

int seating_guest_table_id(DiningHall *hall, int guest_id) {
    SeatAssignment *s = seating_find_guest(hall, guest_id);
    return s ? s->table_id : -1;
}

int seating_assign_guest(DiningHall *hall,
                         int table_id, int guest_id,
                         const char *guest_name,
                         MealPreference meal,
                         DietaryRestriction diet) {
    if (!hall) return -1;
    if (seating_find_guest(hall, guest_id)) return -1;

    DiningTable *t = seating_find_table(hall, table_id);
    if (!t) return -1;
    if (t->assigned_count >= t->capacity) return -1;

    for (int i = 0; i < t->capacity; i++) {
        SeatAssignment *s = &t->seats[i];
        if (s->guest_id == -1) {
            s->guest_id = guest_id;
            safe_copy(s->guest_name, sizeof(s->guest_name), guest_name ? guest_name : "Guest");
            s->meal = meal;
            s->diet = diet;
            s->rsvp = RSVP_PENDING;
            t->assigned_count++;
            return s->seat_id;
        }
    }
    return -1;
}

int seating_remove_guest(DiningHall *hall, int guest_id) {
    if (!hall) return 0;
    for (int t = 0; t < hall->table_count; t++) {
        DiningTable *tbl = &hall->tables[t];
        for (int i = 0; i < tbl->capacity; i++) {
            SeatAssignment *s = &tbl->seats[i];
            if (s->guest_id == guest_id) {
                s->guest_id = -1;
                s->guest_name[0] = '\0';
                s->meal = MEAL_NOT_SET;
                s->diet = DIET_NONE;
                s->rsvp = RSVP_PENDING;
                s->notes[0] = '\0';
                if (tbl->assigned_count > 0) tbl->assigned_count--;
                return 1;
            }
        }
    }
    return 0;
}

int seating_update_rsvp(DiningHall *hall, int guest_id, RsvpStatus rsvp) {
    SeatAssignment *s = seating_find_guest(hall, guest_id);
    if (!s) return 0;
    s->rsvp = rsvp;
    return 1;
}

int seating_move_guest(DiningHall *hall, int guest_id, int new_table_id) {
    if (!hall) return 0;
    SeatAssignment *old = seating_find_guest(hall, guest_id);
    if (!old) return 0;

    char name[100];
    safe_copy(name, sizeof(name), old->guest_name);
    MealPreference meal = old->meal;
    DietaryRestriction diet = old->diet;
    RsvpStatus rsvp = old->rsvp;

    int old_table_id = old->table_id;
    (void)old_table_id;

    if (!seating_remove_guest(hall, guest_id)) return 0;

    int seat_id = seating_assign_guest(hall, new_table_id, guest_id, name, meal, diet);
    if (seat_id < 0) {
        /* best-effort rollback: try put back to original table */
        seating_assign_guest(hall, old_table_id, guest_id, name, meal, diet);
        SeatAssignment *rb = seating_find_guest(hall, guest_id);
        if (rb) rb->rsvp = rsvp;
        return 0;
    }

    SeatAssignment *ns = seating_find_guest(hall, guest_id);
    if (ns) ns->rsvp = rsvp;
    return 1;
}

MealSummary seating_meal_summary(DiningHall *hall) {
    MealSummary ms;
    memset(&ms, 0, sizeof(ms));
    if (!hall) return ms;

    for (int t = 0; t < hall->table_count; t++) {
        DiningTable *tbl = &hall->tables[t];
        for (int i = 0; i < tbl->capacity; i++) {
            SeatAssignment *s = &tbl->seats[i];
            if (s->guest_id == -1) continue;

            switch (s->meal) {
                case MEAL_MEAT:       ms.meat_count++; break;
                case MEAL_FISH:       ms.fish_count++; break;
                case MEAL_VEGETARIAN: ms.vegetarian_count++; break;
                case MEAL_KIDS:       ms.kids_count++; break;
                default: break;
            }
            switch (s->diet) {
                case DIET_HALAL: ms.halal_count++; break;
                case DIET_ALLERGY: ms.allergy_count++; break;
                default: break;
            }
        }
    }

    return ms;
}

void seating_save(DiningHall *hall, const char *path) {
    if (!hall || !path) return;
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(hall, sizeof(*hall), 1, f);
    fclose(f);
}

void seating_load(DiningHall *hall, const char *path) {
    if (!hall || !path) return;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fread(hall, sizeof(*hall), 1, f);
    fclose(f);

    int max_tid = 0;
    int max_sid = 0;
    for (int i = 0; i < hall->table_count; i++) {
        if (hall->tables[i].table_id > max_tid) max_tid = hall->tables[i].table_id;
        for (int s = 0; s < hall->tables[i].capacity; s++) {
            if (hall->tables[i].seats[s].seat_id > max_sid) max_sid = hall->tables[i].seats[s].seat_id;
        }
    }
    if (max_tid >= seating_table_id_counter) seating_table_id_counter = max_tid + 1;
    if (max_sid >= seating_seat_id_counter) seating_seat_id_counter = max_sid + 1;
}

