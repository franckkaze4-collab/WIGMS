/* ============================================================
 *  WIGMS - Wedding Invitation and Gift Management System
 *  MODULE 6: Parking Management
 *  File: parking.c
 *  Description: Full implementation of all Parking functions
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "parking.h"

/* ============================================================
 *  Internal (static) helpers
 * ============================================================ */

static void clear_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void read_string(const char *prompt, char *dest, int max_len) {
    char buffer[256];
    while (true) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
                buffer[len - 1] = '\0';
            if (strlen(buffer) == 0) {
                printf("  [!] Input cannot be empty. Try again.\n");
                continue;
            }
            strncpy(dest, buffer, max_len - 1);
            dest[max_len - 1] = '\0';
            return;
        }
        printf("  [!] Input error. Try again.\n");
        clearerr(stdin);
    }
}

static int read_positive_int(const char *prompt) {
    int value;
    while (true) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1 && value >= 1) {
            clear_buffer();
            return value;
        }
        printf("  [!] Please enter a valid positive integer.\n");
        clear_buffer();
    }
}

/**
 * read_vehicle_type()
 * Presents a menu and returns the chosen VehicleType.
 */
/* Used by create_spot() interactive assignment flow (Module 5 UI calls assign_spot) */
static VehicleType read_vehicle_type(void) {
    int choice;
    while (true) {
        printf("  Vehicle type:\n");
        printf("    0 - Car\n");
        printf("    1 - Motorcycle\n");
        printf("  Your choice: ");
        if (scanf("%d", &choice) == 1 &&
            choice == 0 || choice == 1) {
            clear_buffer();
            return (VehicleType)choice;
        }
        printf("  [!] Invalid choice. Enter 0 or 1.\n");
        clear_buffer();
    }
}

/**
 * spot_id_exists()
 * Returns true if a spot with the given id already exists in the list.
 */
static bool spot_id_exists(ParkingSpot *head, int spot_id) {
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->spot_id == spot_id) return true;
        cur = cur->next;
    }
    return false;
}

/* ============================================================
 *  Helper string converters
 * ============================================================ */

const char *vehicle_type_to_string(VehicleType t) {
    switch (t) {
        case CAR:        return "Car";
        case MOTORCYCLE: return "Motorcycle";
        default:         return "Unknown";
    }
}

const char *spot_status_to_string(SpotStatus s) {
    switch (s) {
        case AVAILABLE: return "Available";
        case OCCUPIED:  return "Occupied";
        default:        return "Unknown";
    }
}

/* ============================================================
 *  Creation
 * ============================================================ */

/**
 * create_spot()
 * Allocates and initialises a new ParkingSpot interactively.
 * The spot is set to AVAILABLE — no guest or vehicle assigned yet.
 * Returns a pointer to the new spot, or NULL on allocation failure.
 */
ParkingSpot *create_spot(void) {
    ParkingSpot *spot = (ParkingSpot *)malloc(sizeof(ParkingSpot));
    if (!spot) {
        printf("  [!] Memory allocation failed for ParkingSpot.\n");
        return NULL;
    }

    printf("\n========================================\n");
    printf("         CREATE NEW PARKING SPOT        \n");
    printf("========================================\n");

    spot->spot_id = read_positive_int("  Spot ID (number)  : ");
    clear_buffer();
    read_string("  Spot code (e.g. A-01): ", spot->spot_code, SPOT_CODE_LEN);

    spot->status = AVAILABLE;
    memset(&spot->vehicle, 0, sizeof(Vehicle));
    memset(&spot->guest,   0, sizeof(person));
    spot->next = NULL;

    printf("  [OK] Spot \"%s\" created (status: Available).\n",
           spot->spot_code);
    return spot;
}

/* ============================================================
 *  Linked List Operations
 * ============================================================ */

/**
 * insert_spot()
 * Creates a new spot interactively and inserts it at the
 * head of the linked list.
 */
void insert_spot(ParkingSpot **head) {
    ParkingSpot *new_spot = create_spot();
    if (!new_spot) return;

    /* Reject duplicate IDs */
    if (spot_id_exists(*head, new_spot->spot_id)) {
        printf("  [!] A spot with ID %d already exists. Aborting.\n",
               new_spot->spot_id);
        free(new_spot);
        return;
    }

    new_spot->next = *head;
    *head = new_spot;
    printf("  [OK] Spot inserted into parking list.\n");
}

/**
 * delete_spot()
 * Removes the spot with the given spot_id from the list.
 * A spot that is OCCUPIED cannot be deleted.
 */
void delete_spot(ParkingSpot **head, int spot_id) {
    if (!*head) {
        printf("  [!] Parking list is empty.\n");
        return;
    }

    ParkingSpot *cur  = *head;
    ParkingSpot *prev = NULL;

    while (cur && cur->spot_id != spot_id) {
        prev = cur;
        cur  = cur->next;
    }

    if (!cur) {
        printf("  [!] Spot ID %d not found.\n", spot_id);
        return;
    }

    if (cur->status == OCCUPIED) {
        printf("  [!] Spot %s is currently OCCUPIED by guest \"%s\".\n",
               cur->spot_code, cur->guest.name);
        printf("      Free the spot first before deleting it.\n");
        return;
    }

    if (prev)
        prev->next = cur->next;
    else
        *head = cur->next;

    printf("  [OK] Spot \"%s\" (ID %d) deleted.\n",
           cur->spot_code, cur->spot_id);
    free(cur);
}

/* ============================================================
 *  Assignment
 * ============================================================ */

/**
 * assign_spot()
 * Assigns a guest and their vehicle to the spot with spot_id.
 * Fails if the spot is already occupied or not found.
 */
void assign_spot(ParkingSpot *head, int spot_id,
                 person guest, Vehicle vehicle) {
    ParkingSpot *spot = find_spot_by_id(head, spot_id);

    if (!spot) {
        printf("  [!] Spot ID %d not found.\n", spot_id);
        return;
    }
    if (spot->status == OCCUPIED) {
        printf("  [!] Spot \"%s\" is already occupied by \"%s\".\n",
               spot->spot_code, spot->guest.name);
        return;
    }

    spot->status  = OCCUPIED;
    spot->guest   = guest;
    spot->vehicle = vehicle;

    printf("  [OK] Spot \"%s\" assigned to guest \"%s\" "
           "(plate: %s, vehicle: %s).\n",
           spot->spot_code,
           guest.name,
           vehicle.plate,
           vehicle_type_to_string(vehicle.type));
}

/**
 * free_spot()
 * Releases the spot with spot_id — sets it back to AVAILABLE
 * and clears guest / vehicle data.
 */
void free_spot(ParkingSpot *head, int spot_id) {
    ParkingSpot *spot = find_spot_by_id(head, spot_id);

    if (!spot) {
        printf("  [!] Spot ID %d not found.\n", spot_id);
        return;
    }
    if (spot->status == AVAILABLE) {
        printf("  [!] Spot \"%s\" is already free.\n", spot->spot_code);
        return;
    }

    printf("  [OK] Spot \"%s\" freed (was assigned to \"%s\").\n",
           spot->spot_code, spot->guest.name);

    spot->status = AVAILABLE;
    memset(&spot->vehicle, 0, sizeof(Vehicle));
    memset(&spot->guest,   0, sizeof(person));
}

/* ============================================================
 *  Search
 * ============================================================ */

/**
 * find_spot_by_id()
 * Returns pointer to the spot with matching spot_id, or NULL.
 */
ParkingSpot *find_spot_by_id(ParkingSpot *head, int spot_id) {
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->spot_id == spot_id) return cur;
        cur = cur->next;
    }
    return NULL;
}

/**
 * find_spot_by_guest()
 * Returns pointer to the OCCUPIED spot assigned to guest_id, or NULL.
 */
ParkingSpot *find_spot_by_guest(ParkingSpot *head, int guest_id) {
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == OCCUPIED && cur->guest.id == guest_id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/**
 * find_spot_by_plate()
 * Returns pointer to the spot whose vehicle plate matches, or NULL.
 * Comparison is case-insensitive.
 */
ParkingSpot *find_spot_by_plate(ParkingSpot *head, const char *plate) {
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == OCCUPIED &&
            strcasecmp(cur->vehicle.plate, plate) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* ============================================================
 *  Display
 * ============================================================ */

static void print_spot_header(void) {
    printf("\n");
    printf("+------+----------+-----------+--------------------+------------------+-----------+\n");
    printf("| %-4s | %-8s | %-9s | %-18s | %-16s | %-9s |\n",
           "ID", "Code", "Status", "Guest Name", "Plate", "Vehicle");
    printf("+------+----------+-----------+--------------------+------------------+-----------+\n");
}

static void print_spot_row(ParkingSpot *s) {
    if (s->status == OCCUPIED) {
        printf("| %-4d | %-8s | %-9s | %-18s | %-16s | %-9s |\n",
               s->spot_id,
               s->spot_code,
               spot_status_to_string(s->status),
               s->guest.name,
               s->vehicle.plate,
               vehicle_type_to_string(s->vehicle.type));
    } else {
        printf("| %-4d | %-8s | %-9s | %-18s | %-16s | %-9s |\n",
               s->spot_id,
               s->spot_code,
               spot_status_to_string(s->status),
               "---", "---", "---");
    }
    printf("+------+----------+-----------+--------------------+------------------+-----------+\n");
}

/**
 * display_spot()
 * Prints a single parking spot row with header.
 */
void display_spot(ParkingSpot *spot) {
    if (!spot) {
        printf("  [!] Spot not found.\n");
        return;
    }
    print_spot_header();
    print_spot_row(spot);
}

/**
 * display_all_spots()
 * Prints every spot in the list.
 */
void display_all_spots(ParkingSpot *head) {
    if (!head) {
        printf("  [!] No parking spots registered.\n");
        return;
    }
    printf("\n========== ALL PARKING SPOTS ==========\n");
    print_spot_header();
    ParkingSpot *cur = head;
    while (cur) {
        print_spot_row(cur);
        cur = cur->next;
    }
    printf("  Total: %d spot(s) | Available: %d | Occupied: %d\n",
           count_all_spots(head),
           count_available(head),
           count_occupied(head));
}

/**
 * display_available_spots()
 * Prints only free spots.
 */
void display_available_spots(ParkingSpot *head) {
    printf("\n========== AVAILABLE SPOTS ==========\n");
    print_spot_header();
    int found = 0;
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == AVAILABLE) {
            print_spot_row(cur);
            found++;
        }
        cur = cur->next;
    }
    if (found == 0)
        printf("  No available spots at the moment.\n");
    else
        printf("  %d available spot(s).\n", found);
}

/**
 * display_occupied_spots()
 * Prints only occupied spots.
 */
void display_occupied_spots(ParkingSpot *head) {
    printf("\n========== OCCUPIED SPOTS ==========\n");
    print_spot_header();
    int found = 0;
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == OCCUPIED) {
            print_spot_row(cur);
            found++;
        }
        cur = cur->next;
    }
    if (found == 0)
        printf("  No occupied spots at the moment.\n");
    else
        printf("  %d occupied spot(s).\n", found);
}

/* ============================================================
 *  Statistics
 * ============================================================ */

int count_available(ParkingSpot *head) {
    int count = 0;
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == AVAILABLE) count++;
        cur = cur->next;
    }
    return count;
}

int count_occupied(ParkingSpot *head) {
    int count = 0;
    ParkingSpot *cur = head;
    while (cur) {
        if (cur->status == OCCUPIED) count++;
        cur = cur->next;
    }
    return count;
}

int count_all_spots(ParkingSpot *head) {
    int count = 0;
    ParkingSpot *cur = head;
    while (cur) { count++; cur = cur->next; }
    return count;
}

/* ============================================================
 *  Persistence
 * ============================================================ */

/**
 * save_parking()
 * Saves all parking spots to a binary file.
 * Node pointers (next) are NOT saved — only data fields.
 */
void save_parking(ParkingSpot *head, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("  [!] Cannot open file \"%s\" for writing.\n", filename);
        return;
    }

    int count = 0;
    ParkingSpot *cur = head;
    while (cur) { count++; cur = cur->next; }

    fwrite(&count, sizeof(int), 1, f);

    cur = head;
    while (cur) {
        fwrite(&cur->spot_id,   sizeof(int),        1, f);
        fwrite(cur->spot_code,  sizeof(char), SPOT_CODE_LEN, f);
        fwrite(&cur->status,    sizeof(SpotStatus),  1, f);
        fwrite(&cur->vehicle,   sizeof(Vehicle),     1, f);
        fwrite(&cur->guest,     sizeof(person),      1, f);
        cur = cur->next;
    }

    fclose(f);
    printf("  [OK] %d parking spot(s) saved to \"%s\".\n", count, filename);
}

/**
 * load_parking()
 * Loads parking spots from a binary file and rebuilds the list.
 * Existing list is NOT cleared — new spots are appended at head.
 */
void load_parking(ParkingSpot **head, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("  [!] Cannot open file \"%s\" for reading.\n", filename);
        return;
    }

    int count = 0;
    fread(&count, sizeof(int), 1, f);

    int loaded = 0;
    int i;
    
    	for( i = 0; i < count; i++) {
        ParkingSpot *spot = (ParkingSpot *)malloc(sizeof(ParkingSpot));
        if (!spot) {
            printf("  [!] Memory allocation failed during load.\n");
            break;
        }

        fread(&spot->spot_id,  sizeof(int),        1, f);
        fread(spot->spot_code, sizeof(char), SPOT_CODE_LEN, f);
        fread(&spot->status,   sizeof(SpotStatus),  1, f);
        fread(&spot->vehicle,  sizeof(Vehicle),     1, f);
        fread(&spot->guest,    sizeof(person),      1, f);

        spot->next = *head;
        *head      = spot;
        loaded++;
    }

    fclose(f);
    printf("  [OK] %d parking spot(s) loaded from \"%s\".\n",
           loaded, filename);
}
