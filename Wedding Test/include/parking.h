#ifndef PARKING_H
#define PARKING_H

/*
 * ============================================================
 *  MODULE 6 - Parking Management
 * ============================================================
 */

#include <time.h>
#include "types.h"

typedef enum {
    SPOT_AVAILABLE = 0,
    SPOT_OCCUPIED,
    SPOT_RESERVED,
    SPOT_BLOCKED
} SpotStatus;

typedef enum {
    VEHICLE_CAR = 0,
    VEHICLE_MOTORCYCLE,
    VEHICLE_BUS,
    VEHICLE_VIP
} VehicleType;

typedef enum {
    ZONE_VIP = 0,
    ZONE_STANDARD,
    ZONE_MOTO,
    ZONE_BUS,
    ZONE_COUNT
} ParkingZone;

typedef struct {
    int         spot_id;
    ParkingZone zone;
    SpotStatus  status;
    int         guest_id;      /* -1 if none */
    char        plate[20];
    VehicleType vehicle_type;
    time_t      entry_time;    /* 0 if none */
    time_t      exit_time;     /* 0 if none */
    char        notes[100];
} ParkingSpot;

typedef struct {
    ParkingZone zone;
    char        name[30];
    char        code[5];
    int         capacity;
    int         spot_start_id;
} ZoneConfig;

#define MAX_SPOTS 200

typedef struct {
    ParkingSpot spots[MAX_SPOTS];
    int         total_spots;
    ZoneConfig  zones[ZONE_COUNT];
} ParkingLot;

typedef struct {
    int total_spots;
    int occupied;
    int available;
    int reserved;
    int blocked;
    int vip_occupied;
    int vip_available;
} ParkingStats;

void parking_init(ParkingLot *lot,
                  int vip_cap, int std_cap,
                  int moto_cap, int bus_cap);

int  parking_assign(ParkingLot *lot,
                    int guest_id, const char *plate,
                    VehicleType vtype, ParkingZone preferred_zone);

int  parking_release(ParkingLot *lot, int spot_id);

int  parking_reserve(ParkingLot *lot,
                     int spot_id, int guest_id,
                     const char *note);

int  parking_block(ParkingLot *lot, int spot_id, const char *reason);
int  parking_unblock(ParkingLot *lot, int spot_id);

ParkingSpot* parking_find_by_plate(ParkingLot *lot, const char *plate);
ParkingSpot* parking_find_by_guest(ParkingLot *lot, int guest_id);
int          parking_count_available(ParkingLot *lot, ParkingZone zone);

ParkingStats parking_get_stats(ParkingLot *lot);

void parking_save(ParkingLot *lot, const char *path);
void parking_load(ParkingLot *lot, const char *path);

const char* parking_zone_name(ParkingZone z);
const char* parking_status_name(SpotStatus s);
const char* parking_vehicle_name(VehicleType v);
char*       parking_format_time(time_t t, char *buf, int len);

extern int parking_spot_id_counter;

#endif /* PARKING_H */

