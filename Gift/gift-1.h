#ifndef GIFT_H
#define GIFT_H

/* ============================================================
 *  MODULE 4 — Gift Management  (WIGMS)
 *  File   : gift.h
 *  Author : <your name>
 *  Date   : 2025-2026
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_GIFTS        200
#define GIFT_NAME_LEN    100
#define GIFT_FILE        "data/gifts.dat"   /* binary persistence  */
#define GIFT_FILE_CSV    "data/gifts.csv"   /* CSV  export         */


typedef struct {
    int   gift_id;              /* unique identifier            */
    char  name[GIFT_NAME_LEN];  /* gift description / label     */
    float value;                /* monetary value (FCFA or any) */
    int   guest_id;             /* foreign key → Person.id      */
} Gift;


typedef struct {
    Gift gifts[MAX_GIFTS];
    int  count;
} GiftStore;



/**
 * init_gift_store
 * Initialises an empty GiftStore (sets count to 0).
 * Must be called once before any other gift function.
 */
void init_gift_store(GiftStore *store);

/**
 * register_gift
 * Prompts the user for gift data and appends the new gift
 * to the store.  Auto-generates a unique gift_id.
 * Returns  1 on success, 0 if the store is full.
 */
int register_gift(GiftStore *store);

/**
 * display_gifts
 * Prints all gifts currently held in the store as a
 * formatted table to stdout.
 */
void display_gifts(const GiftStore *store);

/**
 * display_gift_by_guest
 * Prints every gift associated with the given guest_id.
 */
void display_gift_by_guest(const GiftStore *store, int guest_id);

/**
 * update_gift
 * Finds the gift with the matching gift_id and lets the
 * user update its fields interactively.
 * Returns  1 if the gift was found and updated, 0 otherwise.
 */
int update_gift(GiftStore *store, int gift_id);

/**
 * delete_gift
 * Removes the gift with the given gift_id from the store
 * (shifts the array to fill the gap).
 * Returns  1 on success, 0 if not found.
 */
int delete_gift(GiftStore *store, int gift_id);

/**
 * find_gift_by_id
 * Returns a pointer to the gift with the given gift_id,
 * or NULL if no match is found.
 */
Gift *find_gift_by_id(GiftStore *store, int gift_id);

/**
 * total_gift_value
 * Returns the sum of the values of all gifts in the store.
 */
float total_gift_value(const GiftStore *store);

/**
 * total_gift_value_by_guest
 * Returns the total gift value contributed by a specific guest.
 */
float total_gift_value_by_guest(const GiftStore *store, int guest_id);

/**
 * sort_gifts_by_value_desc
 * Sorts the gift array in descending order of value (in place).
 */
void sort_gifts_by_value_desc(GiftStore *store);

/**
 * save_gifts
 * Persists the entire GiftStore to a binary file (GIFT_FILE).
 * Returns  1 on success, 0 on I/O error.
 */
int save_gifts(const GiftStore *store);

/**
 * load_gifts
 * Loads gifts from the binary file (GIFT_FILE) into the store.
 * Returns  1 on success, 0 if the file does not exist or on error.
 */
int load_gifts(GiftStore *store);

/**
 * export_gifts_csv
 * Exports all gifts to a human-readable CSV file (GIFT_FILE_CSV).
 * Returns  1 on success, 0 on I/O error.
 */
int export_gifts_csv(const GiftStore *store);

#endif /* GIFT_H */
