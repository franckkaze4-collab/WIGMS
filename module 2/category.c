
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
    Category *new = create_category();
    new->next = *tete;
    *tete = new;
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
    if (*tete == NULL) return;
    int swapped;
    Category *current;
    do {
        swapped = 0;
        current = *tete;
        while (current->next != NULL) {
            if (current->id < current->next->id) {
                int temp = current->id;
                current->id = current->next->id;
                current->next->id = temp;
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

