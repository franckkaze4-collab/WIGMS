#include "ui_gtk.h"
#include "category.h"
#include "gift.h"
#include "parking.h"
#include "seating.h"
#include "schedule.h"
#include <string.h>

int main(int argc, char *argv[]) {
    AppState state;
    state.cat_head = NULL;
    state.gift_count = 0;

    /* Module 6: parking (capacities can be adjusted) */
    parking_init(&state.parking, 20, 60, 30, 10);
    parking_load(&state.parking, "data/parking.dat");

    /* Module 7: seating */
    memset(&state.dining, 0, sizeof(state.dining));
    seating_load(&state.dining, "data/seating.dat");

    /* Module 8: schedule */
    schedule_init(&state.schedule, "Wedding Day", "Venue");
    schedule_load(&state.schedule, "data/schedule.dat");

    ui_gtk_run(&argc, &argv, &state);

    /* Safety cleanup (UI also saves on exit) */
    schedule_free(&state.schedule);
    return 0;
}

