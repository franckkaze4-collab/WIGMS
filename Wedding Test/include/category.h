#ifndef CATEGORY_H
#define CATEGORY_H

#include "types.h"

/* =========================================================
   MODULE 2 - Category Management (Linked List)
   ========================================================= */

/* Console-interactive versions */
Category* create_category(int id);
void      insert_category(Category **tete);
void      delete_category(Category **tete, int id);
void      update_category(Category *tete, int id);
int       count_guest(Category *tete);
void      sort_categories_desc(Category **tete);
void      display_all_categories(Category *tete);
void      display_all_guests(Category *tete);
void      free_list(Category **tete);

/* Shared counters (used by console and GUI) */
extern int category_id_counter;
extern int person_id_counter;

/* =========================================================
   GUI-friendly wrappers (data-driven)
   ========================================================= */

/* Create a category node using provided code (no stdin). */
Category* category_create(int id, const char *code);

/* Insert an existing category node at the head of the list. */
void      category_insert(Category **tete, Category *node);

/* Find a category node by id (read-only traversal). */
Category* category_find_by_id(Category *head, int id);

/* GUI wrappers around the console functions (same semantics). */
void      category_delete(Category **tete, int id);
void      category_sort_desc(Category **tete);
void      category_free_all(Category **tete);

/* Stats helpers for dashboard */
int       category_count_nodes(Category *head);
int       category_count_all_guests(Category *head);

#endif /* CATEGORY_H */

