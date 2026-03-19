#ifndef CATEGORY_H
#define CATEGORY_H
#include "person.h"

typedef struct Category {
	int id;
	char code[50];
	person guests[4];
	struct category* next;
} Category;

Category* create_category();
void insert_category(Category **tete);
void delete_category(Category **tete, int id);
void update_category(Category *tete, int id);
void count_guest(Category *tete);
void sort_categories_desc(Category **tete);
void display_all_guests(Category *tete);

#endif
