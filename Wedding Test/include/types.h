#ifndef TYPES_H
#define TYPES_H

/* =========================================================
   WIGMS - Shared Data Types
   All structs and enums used across the entire application
   ========================================================= */

/* Which side of the family the guest belongs to:
   LE = Le marie (groom's side)
   LA = La mariee (bride's side) */
typedef enum {
    LE,
    LA
} Side;

/* A single wedding guest / person */
typedef struct {
    int  id;
    char name[100];
    int  age;
    char social_class[50];   /* e.g. "VIP", "Family", "Friend" */
    Side side;
} Person;

/* A guest category - linked list node.
   Each node holds up to 4 guests. */
typedef struct Category {
    int  id;
    char code[50];           /* e.g. "FAM_LE", "AMI_LA" */
    int  guest_count;       /* how many of the 4 slots are filled */
    Person guests[4];
    struct Category *next;
} Category;

/* A gift registered for the wedding */
typedef struct {
    int   gift_id;
    char  name[100];
    float value;
    int   guest_id;         /* which guest gave this gift */
} Gift;

#define MAX_GIFTS 200

#endif /* TYPES_H */

