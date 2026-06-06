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

/* Guest registration (phone, email for the extended guest module) */
#define MAX_REGISTERED 200
#define MAX_PHONE 20
#define MAX_EMAIL 100
#define MAX_GIFT_CATALOG 20
#define MAX_SUB_OPTIONS 5
#define MAX_GIFT_PURCHASES 100
#define MAX_CATEGORY_CODE 50

/* A gift catalog item (one of the 20 available gifts) */
typedef struct {
    int    id;
    char   name[100];
    float  price;
    char   sub_options[MAX_SUB_OPTIONS][50];
    int    sub_count;
    int    purchase_count;   /* how many times bought (max 3) */
} GiftCatalogItem;

/* A guest's purchase record */
typedef struct {
    int   guest_id;
    int   catalog_item_id;
    int   sub_index;
    int   quantity;
    float total_amount;
} GiftPurchase;

/* A registered guest (extended from GuestRecord) */
typedef struct {
    int    id;
    char   name[100];
    char   phone[MAX_PHONE];
    char   email[MAX_EMAIL];
    int    age;
    char   category[MAX_CATEGORY_CODE];
    char   social_class[50];
    int    seat_zone;       /* auto-assigned seat zone */
    int    parking_spot;    /* auto-assigned parking spot */
    int    seat_confirmed;  /* 1 if guest confirmed seat */
    int    parking_confirmed;/* 1 if guest confirmed parking */
} RegisteredGuest;

#endif /* TYPES_H */

