
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "category.h"

Category* create_category() {
    Category *c = (Category*)malloc(sizeof(Category));
    printf("Enter Category ID: ");
    scanf("%d", &c->id);
    printf("Enter Category Code: ");
    scanf("%s", c->code);
    c->next = NULL;
    return c;
}

void insert_category(Category **tete) {
    Category *new_cat = create_category();
    new_cat->next = *tete;
    *tete = new_cat;
    printf("Category inserted successfully.\n");
}

void delete_category(Category **tete, int id) {
    Category *current = *tete;
    Category *previous = NULL;
    while (current != NULL) {
        if (current->id == id) {
            if (previous == NULL) {
                *tete = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            printf("Category deleted successfully.\n");
            return;
        }
        previous = current;
        current = current->next;
    }
    printf("Category not found.\n");
}

void update_category(Category *tete, int id) {
    while (tete != NULL) {
        if (tete->id == id) {
            printf("Enter new code: ");
            scanf("%s", tete->code);
            printf("Category updated successfully.\n");
            return;
        }
        tete = tete->next;
    }
    printf("Category not found.\n");
}

int count_guest(Category *tete) {
    int count = 0;
    while (tete != NULL) {
        count += 4;
        tete = tete->next;
    }
    return count;
}

void sort_categories_desc(Category **tete) {
    if (tete == NULL || *tete==NULL || (*tete)->next == NULL) return;
    int swapped;
    Category *current;
    do {
        swapped = 0;
        current = *tete;
        while (current->next != NULL && current->next !=NULL) {
            if (current->id < current->next->id) {
            	/* swap the full node content, not just id */
                int temp_id = current->id;
                char temp_code[50];
                person temp_guests[4];
                
                temp_id = current->id;
                strncpy(temp_code, current->code,50);
                memcpy(temp_guests, current->guests, sizeof(person)*4);
                
                current->id=current->next->id;
                strncpy(current->code,current->next->code,50);
                memcpy(current->guests,current->next->guests,sizeof(person)*4);
                
                current->next->id = temp_id;
                strncpy(current->next->code, temp_code, 50);
                memcpy(current->next->guests, temp_guests, sizeof(person)*4);
               
                swapped = 1;
            }
            current = current->next;
        }
    } while (swapped);
}

void display_all_guests(Category *tete) {
    while (tete != NULL) {
        printf("Category ID: %d Code: %s\n", tete->id, tete->code);
        tete = tete->next;
    }
}

