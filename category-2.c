/*
 * FILE: src/category.c
 * MODULE 2: Category Management (Linked List)
 * OWNER: Student 2
 * DESCRIPTION: Full linked-list operations on Category nodes.
 *              Also provides category statistics for the dashboard.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/types.h"
#include "../include/category.h"
#include "../include/ui.h"

/* ============================================================
 *  CORE LINKED-LIST FUNCTIONS
 * ============================================================ */

/*
 * create_category()
 * Allocates and returns a new Category node filled by user input.
 */
Category *create_category(void) {
    Category *c = (Category *)malloc(sizeof(Category));
    if (!c) { perror("malloc failed"); exit(1); }

    printf("\n  -- New Category --\n");
    printf("  ID   : "); scanf("%d", &c->id); getchar();
    printf("  Code : "); fgets(c->code, 50, stdin);
    c->code[strcspn(c->code, "\n")] = '\0';

    c->guest_count = 0;
    c->next        = NULL;
    return c;
}

/*
 * insert_category()
 * Inserts a new category at the HEAD of the linked list.
 */
void insert_category(Category **head) {
    Category *new_cat  = create_category();
    new_cat->next      = *head;
    *head              = new_cat;
    display_success_message("Category inserted successfully!");
}

/*
 * delete_category()
 * Removes the category with the given id from the list.
 */
void delete_category(Category **head, int id) {
    Category *cur  = *head;
    Category *prev = NULL;

    while (cur && cur->id != id) {
        prev = cur;
        cur  = cur->next;
    }

    if (!cur) { display_error_message("Category ID not found."); return; }

    if (prev) prev->next = cur->next;
    else      *head      = cur->next;

    free(cur);
    display_success_message("Category deleted.");
}

/*
 * update_category()
 * Finds a category by id and lets user change its code.
 */
void update_category(Category *head, int id) {
    while (head) {
        if (head->id == id) {
            printf("  New code (current: %s): ", head->code);
            fgets(head->code, 50, stdin);
            head->code[strcspn(head->code, "\n")] = '\0';
            display_success_message("Category updated.");
            return;
        }
        head = head->next;
    }
    display_error_message("Category not found.");
}

/*
 * count_guest()
 * Returns the total number of guests across all categories.
 */
int count_guest(Category *head) {
    int total = 0;
    while (head) { total += head->guest_count; head = head->next; }
    return total;
}

/*
 * sort_categories_desc()
 * Bubble-sorts the linked list by guest_count (descending).
 */
void sort_categories_desc(Category **head) {
    if (!*head) return;
    int swapped;
    do {
        swapped = 0;
        Category *cur = *head;
        while (cur->next) {
            if (cur->guest_count < cur->next->guest_count) {
                /* swap data only (easier than relinking) */
                int tmp_id           = cur->id;
                char tmp_code[50];
                Person tmp_guests[4];
                int tmp_count        = cur->guest_count;

                cur->id          = cur->next->id;
                strncpy(tmp_code, cur->code, 50);
                strncpy(cur->code, cur->next->code, 50);
                strncpy(cur->next->code, tmp_code, 50);
                memcpy(tmp_guests, cur->guests, sizeof(tmp_guests));
                memcpy(cur->guests, cur->next->guests, sizeof(tmp_guests));
                memcpy(cur->next->guests, tmp_guests, sizeof(tmp_guests));
                cur->guest_count      = cur->next->guest_count;
                cur->next->id         = tmp_id;
                cur->next->guest_count = tmp_count;

                swapped = 1;
            }
            cur = cur->next;
        }
    } while (swapped);
}

/*
 * display_all_guests()
 * Prints every guest in every category.
 */
void display_all_guests(Category *head) {
    if (!head) { display_error_message("No categories found."); return; }
    while (head) {
        printf("\n  === Category [%d] %s ===\n", head->id, head->code);
        if (head->guest_count == 0) {
            printf("  (no guests)\n");
        } else {
            for (int i = 0; i < head->guest_count; i++) {
                Person *p = &head->guests[i];
                printf("    %d. %-20s | Age: %d | %s | %s\n",
                       i + 1, p->name, p->age, p->social_class,
                       p->side == LE ? "Groom" : "Bride");
            }
        }
        head = head->next;
    }
}

/* ============================================================
 *  DASHBOARD CONTRIBUTIONS  (Student 2 owns these)
 * ============================================================ */

int stats_total_categories(Category *head) {
    int count = 0;
    while (head) { count++; head = head->next; }
    return count;
}

char *stats_most_populated_category(Category *head) {
    if (!head) return "N/A";
    Category *best = head;
    while (head) {
        if (head->guest_count > best->guest_count) best = head;
        head = head->next;
    }
    return best->code;
}

/*
 * display_category_panel()
 * Renders the Category Summary box on the dashboard.
 */
void display_category_panel(Category *head) {
    print_line('-', 60);
    print_centered("[ CATEGORY SUMMARY ]", 60);
    print_line('-', 60);
    printf("  %-30s : %d\n", "Total Categories",
           stats_total_categories(head));
    printf("  %-30s : %d\n", "Total Guests",
           count_guest(head));
    printf("  %-30s : %s\n", "Most Populated Category",
           stats_most_populated_category(head));

    /* List each category with a mini bar */
    Category *cur = head;
    while (cur) {
        printf("  %-15s [", cur->code);
        for (int i = 0; i < cur->guest_count; i++) putchar('#');
        for (int i = cur->guest_count; i < 4; i++) putchar('.');
        printf("] %d/4\n", cur->guest_count);
        cur = cur->next;
    }
    printf("\n");
}

/* ============================================================
 *  CATEGORY SUB-MENU
 * ============================================================ */

void category_menu(Category **head) {
    char buf[16];
    int  choice, id;

    do {
        clear_screen();
        print_line('=', 60);
        print_centered("CATEGORY MANAGEMENT", 60);
        print_line('=', 60);
        printf("  1. Add category\n");
        printf("  2. Delete category\n");
        printf("  3. Update category\n");
        printf("  4. Display all guests\n");
        printf("  5. Sort categories (desc)\n");
        printf("  6. Count all guests\n");
        printf("  0. Back\n");
        printf("  Choice: ");
        fgets(buf, sizeof(buf), stdin);
        choice = atoi(buf);

        switch (choice) {
            case 1: insert_category(head);                          break;
            case 2:
                printf("  Delete ID: "); scanf("%d", &id); getchar();
                delete_category(head, id);
                printf("  Press ENTER..."); getchar();             break;
            case 3:
                printf("  Update ID: "); scanf("%d", &id); getchar();
                update_category(*head, id);
                printf("  Press ENTER..."); getchar();             break;
            case 4:
                clear_screen();
                display_all_guests(*head);
                printf("\n  Press ENTER..."); getchar();           break;
            case 5:
                sort_categories_desc(head);
                display_success_message("Sorted by guest count (desc).");
                printf("  Press ENTER..."); getchar();             break;
            case 6:
                printf("\n  Total guests: %d\n", count_guest(*head));
                printf("  Press ENTER..."); getchar();             break;
            case 0: break;
            default: display_error_message("Invalid option.");
        }
    } while (choice != 0);
}
