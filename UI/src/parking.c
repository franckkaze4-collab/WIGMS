#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parking.h"

int parking_spot_id_counter = 1;

static const char *ZONE_NAMES[] = {
    "Zone A - VIP Reserved",
    "Zone B - Standard Cars",
    "Zone C - Motorcycles",
    "Zone D - Buses & Vans"
};
static const char *ZONE_CODES[] = {"A","B","C","D"};

static const char *STATUS_NAMES[] = {
    "AVAILABLE","OCCUPIED","RESERVED","BLOCKED"
};
static const char *VEHICLE_NAMES[] = {
    "Car","Motorcycle","Bus/Van","VIP"
};

const char* parking_zone_name(ParkingZone z) {
    if (z < 0 || z >= ZONE_COUNT) return "Unknown";
    return ZONE_NAMES[z];
}
const char* parking_status_name(SpotStatus s) {
    if (s < 0 || s > SPOT_BLOCKED) return "Unknown";
    return STATUS_NAMES[s];
}
const char* parking_vehicle_name(VehicleType v) {
    if (v < 0 || v > VEHICLE_VIP) return "Unknown";
    return VEHICLE_NAMES[v];
}

char* parking_format_time(time_t t, char *buf, int len) {
    if (!buf || len <= 0) return buf;
    if (t == 0) {
        strncpy(buf, "-", (size_t)len);
        buf[len - 1] = '\0';
        return buf;
    }
    struct tm *tm_info = localtime(&t);
    if (!tm_info) {
        strncpy(buf, "?", (size_t)len);
        buf[len - 1] = '\0';
        return buf;
    }
    strftime(buf, (size_t)len, "%H:%M %d/%m", tm_info);
    return buf;
}

static void spot_clear(ParkingSpot *sp) {
    sp->status = SPOT_AVAILABLE;
    sp->guest_id = -1;
    sp->plate[0] = '\0';
    sp->vehicle_type = VEHICLE_CAR;
    sp->entry_time = 0;
    sp->exit_time = 0;
    sp->notes[0] = '\0';
}

void parking_init(ParkingLot *lot,
                  int vip_cap, int std_cap,
                  int moto_cap, int bus_cap) {
    if (!lot) return;
    memset(lot, 0, sizeof(*lot));

    int caps[ZONE_COUNT] = {vip_cap, std_cap, moto_cap, bus_cap};
    int cursor = 0;

    for (int z = 0; z < ZONE_COUNT; z++) {
        lot->zones[z].zone = (ParkingZone)z;
        strncpy(lot->zones[z].name, ZONE_NAMES[z], sizeof(lot->zones[z].name) - 1);
        strncpy(lot->zones[z].code, ZONE_CODES[z], sizeof(lot->zones[z].code) - 1);
        lot->zones[z].capacity = caps[z];
        lot->zones[z].spot_start_id = parking_spot_id_counter + cursor;

        for (int s = 0; s < caps[z] && cursor < MAX_SPOTS; s++, cursor++) {
            ParkingSpot *sp = &lot->spots[cursor];
            memset(sp, 0, sizeof(*sp));
            sp->spot_id = parking_spot_id_counter++;
            sp->zone = (ParkingZone)z;
            spot_clear(sp);
        }
    }

    lot->total_spots = cursor;
}

static int spot_matches_zone(ParkingSpot *s, ParkingZone z) {
    return s && s->zone == z;
}

int parking_assign(ParkingLot *lot,
                   int guest_id, const char *plate,
                   VehicleType vtype, ParkingZone preferred_zone) {
    if (!lot || !plate || !plate[0]) return -1;

    ParkingSpot *target = NULL;

    /* Pass 1: preferred zone only */
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (spot_matches_zone(s, preferred_zone) && s->status == SPOT_AVAILABLE) {
            target = s;
            break;
        }
    }

    /* Pass 2: overflow (any zone) */
    if (!target) {
        for (int i = 0; i < lot->total_spots; i++) {
            ParkingSpot *s = &lot->spots[i];
            if (s->status == SPOT_AVAILABLE) {
                target = s;
                break;
            }
        }
    }

    if (!target) return -1;

    target->status = SPOT_OCCUPIED;
    target->guest_id = guest_id;
    target->vehicle_type = vtype;
    target->entry_time = time(NULL);
    target->exit_time = 0;
    strncpy(target->plate, plate, sizeof(target->plate) - 1);
    target->plate[sizeof(target->plate) - 1] = '\0';
    target->notes[0] = '\0';

    return target->spot_id;
}

int parking_release(ParkingLot *lot, int spot_id) {
    if (!lot) return 0;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->spot_id != spot_id) continue;
        if (s->status != SPOT_OCCUPIED) return 0;

        s->exit_time = time(NULL);
        spot_clear(s);
        return 1;
    }
    return 0;
}

int parking_reserve(ParkingLot *lot,
                    int spot_id, int guest_id,
                    const char *note) {
    if (!lot) return 0;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->spot_id != spot_id) continue;
        if (s->status != SPOT_AVAILABLE) return 0;
        s->status = SPOT_RESERVED;
        s->guest_id = guest_id;
        s->notes[0] = '\0';
        if (note && note[0]) {
            strncpy(s->notes, note, sizeof(s->notes) - 1);
            s->notes[sizeof(s->notes) - 1] = '\0';
        }
        return 1;
    }
    return 0;
}

int parking_block(ParkingLot *lot, int spot_id, const char *reason) {
    if (!lot) return 0;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->spot_id != spot_id) continue;
        if (s->status == SPOT_OCCUPIED) return 0;
        s->status = SPOT_BLOCKED;
        s->notes[0] = '\0';
        if (reason && reason[0]) {
            strncpy(s->notes, reason, sizeof(s->notes) - 1);
            s->notes[sizeof(s->notes) - 1] = '\0';
        }
        return 1;
    }
    return 0;
}

int parking_unblock(ParkingLot *lot, int spot_id) {
    if (!lot) return 0;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->spot_id != spot_id) continue;
        if (s->status != SPOT_BLOCKED) return 0;
        s->status = SPOT_AVAILABLE;
        s->notes[0] = '\0';
        return 1;
    }
    return 0;
}

ParkingSpot* parking_find_by_plate(ParkingLot *lot, const char *plate) {
    if (!lot || !plate || !plate[0]) return NULL;
    for (int i = 0; i < lot->total_spots; i++) {
        if (strncmp(lot->spots[i].plate, plate, sizeof(lot->spots[i].plate)) == 0)
            return &lot->spots[i];
    }
    return NULL;
}

ParkingSpot* parking_find_by_guest(ParkingLot *lot, int guest_id) {
    if (!lot) return NULL;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->guest_id == guest_id && s->status != SPOT_AVAILABLE)
            return s;
    }
    return NULL;
}

int parking_count_available(ParkingLot *lot, ParkingZone zone) {
    if (!lot) return 0;
    int count = 0;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        if (s->zone == zone && s->status == SPOT_AVAILABLE) count++;
    }
    return count;
}

ParkingStats parking_get_stats(ParkingLot *lot) {
    ParkingStats st;
    memset(&st, 0, sizeof(st));
    if (!lot) return st;

    st.total_spots = lot->total_spots;
    for (int i = 0; i < lot->total_spots; i++) {
        ParkingSpot *s = &lot->spots[i];
        switch (s->status) {
            case SPOT_AVAILABLE: st.available++; break;
            case SPOT_OCCUPIED:  st.occupied++; break;
            case SPOT_RESERVED:  st.reserved++; break;
            case SPOT_BLOCKED:   st.blocked++; break;
        }
        if (s->zone == ZONE_VIP) {
            if (s->status == SPOT_AVAILABLE) st.vip_available++;
            if (s->status == SPOT_OCCUPIED)  st.vip_occupied++;
        }
    }
    return st;
}

void parking_save(ParkingLot *lot, const char *path) {
    if (!lot || !path) return;
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(lot, sizeof(*lot), 1, f);
    fclose(f);
}

void parking_load(ParkingLot *lot, const char *path) {
    if (!lot || !path) return;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fread(lot, sizeof(*lot), 1, f);
    fclose(f);

    /* Keep counters safe to avoid new spot-id collisions if re-init happens */
    int max_id = 0;
    for (int i = 0; i < lot->total_spots; i++) {
        if (lot->spots[i].spot_id > max_id) max_id = lot->spots[i].spot_id;
    }
    if (max_id >= parking_spot_id_counter) parking_spot_id_counter = max_id + 1;
}

