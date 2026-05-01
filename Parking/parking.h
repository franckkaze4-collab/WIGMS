#ifndef PARKING_H
#define PARKING_H

/* ============================================================
 *  WIGMS - Wedding Invitation and Gift Management System
 *  MODULE 6: Parking Management
 *  File: parking.h
 *  Description: Header — structs, enums, and function prototypes
 * ============================================================ */

#include "person.h"

/* ------- Constants ------- */
#define PLATE_MAX_LEN    20
#define SPOT_CODE_LEN    10
#define MAX_SPOT_NUMBER  999

/* ------- Enums ------- */

/**
 * VehicleType: the kind of vehicle the guest drives.
 */
typedef enum {
    CAR,         /* Standard car          */
    MOTORCYCLE  /* Motorcycle / scooter  */
} VehicleType;

/**
 * SpotStatus: whether a parking spot is free or taken.
 */
typedef enum {
    AVAILABLE,   /* No vehicle assigned   */
    OCCUPIED     /* A guest is parked here*/
} SpotStatus;

/* ------- Structs ------- */

/**
 * Vehicle: information about a guest's vehicle.
 *
 * Fields:
 *   plate        - licence plate number  (e.g., "LT 1234 A")
 *   type         - type of vehicle
 */
typedef struct {
    char        plate[PLATE_MAX_LEN];
    VehicleType type;
} Vehicle;

/**
 * ParkingSpot: one node in the parking linked list.
 *
 * Fields:
 *   spot_id      - unique numeric ID for this spot
 *   spot_code    - human label  (e.g., "A-01", "B-12")
 *   status       - AVAILABLE or OCCUPIED
 *   vehicle      - the vehicle currently parked (valid only when OCCUPIED)
 *   guest        - the Person this spot is assigned to (valid only when OCCUPIED)
 *   next         - pointer to the next spot in the list
 */
typedef struct ParkingSpot {
    int               spot_id;
    char              spot_code[SPOT_CODE_LEN];
    SpotStatus        status;
    Vehicle           vehicle;
    person            guest;
    struct ParkingSpot *next;
} ParkingSpot;

/* ------- Function Prototypes ------- */

/* -- Creation -- */
ParkingSpot *create_spot(void);

/* -- Linked list operations -- */
void         insert_spot(ParkingSpot **head);
void         delete_spot(ParkingSpot **head, int spot_id);

/* -- Assignment -- */
void         assign_spot(ParkingSpot *head, int spot_id,
                         person guest, Vehicle vehicle);
void         free_spot(ParkingSpot *head, int spot_id);

/* -- Search -- */
ParkingSpot *find_spot_by_id(ParkingSpot *head, int spot_id);
ParkingSpot *find_spot_by_guest(ParkingSpot *head, int guest_id);
ParkingSpot *find_spot_by_plate(ParkingSpot *head,
                                const char *plate);

/* -- Display -- */
void         display_spot(ParkingSpot *spot);
void         display_all_spots(ParkingSpot *head);
void         display_available_spots(ParkingSpot *head);
void         display_occupied_spots(ParkingSpot *head);

/* -- Statistics -- */
int          count_available(ParkingSpot *head);
int          count_occupied(ParkingSpot *head);
int          count_all_spots(ParkingSpot *head);

/* -- Persistence -- */
void         save_parking(ParkingSpot *head, const char *filename);
void         load_parking(ParkingSpot **head, const char *filename);

/* -- Helpers -- */
const char  *vehicle_type_to_string(VehicleType t);
const char  *spot_status_to_string(SpotStatus s);

#endif /* PARKING_H */
