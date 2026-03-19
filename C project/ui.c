#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "ui.h"
#include "category.h"
#include "person.h"
#include "priority.h"
#include "gift.h"
 '
 '
/* =========================================================
   MODULE 5 - User Interface Management
 
   ARCHITECTURE RULE:
   This module contains ZERO business logic.
   It only:
     1. Displays menus and messages
     2. Reads user input
     3. Calls the appropriate module function
     4. Shows the result
 
   The MVC pattern here:
     Model      = Modules 1-4 (data + logic)
     Controller = this file (routes user actions to model)
     View       = printf() calls here + terminal
   ========================================================= */
 
#define GIFTS_FILE "data/gifts.dat"
 
/* ----------------------------------------------------------
   print_banner() 1
   Prints the application title header using box-drawing
   characters for a clean terminal look.
   ---------------------------------------------------------- */
void print_banner(void) {
    printf("\n");
    printf("  =================================================\n");
    printf("  ::   WIGMS(Wedding Invitation & Gift Mgmt Sys) ::\n");
    printf("  ::   PKFIE  |  C Programming  |  2025-2026     ::\n");
    printf("  =================================================\n");
    printf("\n");
}
 
/* ----------------------------------------------------------
   print_separator()
   Prints a horizontal rule for visual separation.
   ---------------------------------------------------------- */
void print_separator(void) {
    printf("  ------------------------------------------------\n");
}
 
/* ----------------------------------------------------------
   display_error_message()
   Prints a clearly marked error to stderr.
   ---------------------------------------------------------- */
void display_error_message(const char *msg) {
    fprintf(stderr, "\n  [ERROR] %s\n", msg);
}
 
/* ----------------------------------------------------------
   display_success_message()
   Prints a success confirmation to stdout.
   ---------------------------------------------------------- */
void display_success_message(const char *msg) {
    printf("\n  [OK] %s\n", msg);
}
 
/* ----------------------------------------------------------
   confirm_action()
   Asks the user "Are you sure? (y/n)" before destructive
   operations like delete.
   Returns 1 if confirmed, 0 if cancelled.
   ---------------------------------------------------------- */
int confirm_action(const char *prompt) {
    char buf[8];
    printf("  %s (y/n): ", prompt);
    fgets(buf, sizeof(buf), stdin);
    return (tolower(buf[0]) == 'y') ? 1 : 0;
}
 
/* ----------------------------------------------------------
   read_int()
   Internal helper - safely reads an integer from stdin.
   Loops until the user enters a valid integer.
   Prevents the infinite loop that happens when scanf()
   fails on non-numeric input (it leaves bad chars in buffer).
   ---------------------------------------------------------- */
static int read_int(const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clear_input_buffer();
            return value;
        }
        clear_input_buffer();
        printf("  Invalid input. Please enter a number.\n");
    }
}
 
/* =========================================================
   CATEGORY MENU
   ========================================================= */
void category_menu(Category **head) {
    int choice;
    do {
        system("clear");
        print_banner();
        printf("  --- CATEGORY MANAGEMENT ---\n\n");
        printf("  1. Add new category\n");
        printf("  2. Display all categories\n");
        printf("  3. Display all guests\n");
        printf("  4. Update a category\n");
        printf("  5. Delete a category\n");
        printf("  6. Count total guests\n");
        printf("  7. Sort categories by guest count\n");
        printf("  0. Back to main menu\n\n");
 
        choice = read_int("  Your choice: ");
 
        switch (choice) {
            case 1:
                printf("\n  === Add Category ===\n");
                insert_category(head);
                break;
            case 2:
                printf("\n  === All Categories ===\n");
                display_all_categories(*head);
                break;
            case 3:
                printf("\n  === All Guests ===\n");
                display_all_guests(*head);
                break;
            case 4: {
                display_all_categories(*head);
                int id = read_int("\n  Enter category ID to update: ");
                update_category(*head, id);
                break;
            }
            case 5: {
                display_all_categories(*head);
                int id = read_int("\n  Enter category ID to delete: ");
                if (confirm_action("Delete this category?")) {
                    delete_category(head, id);
                } else {
                    printf("  Cancelled.\n");
                }
                break;
            }
            case 6:
                printf("\n  Total guests: %d\n", count_guest(*head));
                break;
            case 7:
                sort_categories_desc(head);
                break;
            case 0:
                break;
            default:
                display_error_message("Invalid choice.");
        }
 
        if (choice != 0) {
            printf("\n  Press Enter to continue...");
            getchar();
        }
 
    } while (choice != 0);
}
 
/* =========================================================
   PERSON MENU
   ========================================================= */
void person_menu(Category *head) {
    int choice;
    do {
        system("clear");
        print_banner();
        printf("  --- PERSON MANAGEMENT ---\n\n");
        printf("  1. Display all guests\n");
        printf("  2. Update a specific guest\n");
        printf("  0. Back to main menu\n\n");
 
        choice = read_int("  Your choice: ");
 
        switch (choice) {
            case 1:
                display_all_guests(head);
                break;
            case 2: {
                display_all_guests(head);
                int cat_id  = read_int("\n  Category ID: ");
                int guest_num = read_int("  Guest number in category (1-4): ");
                /* Find the category */
                Category *curr = head;
                while (curr != NULL && curr->id != cat_id)
                    curr = curr->next;
                if (curr == NULL) {
                    display_error_message("Category not found.");
                } else if (guest_num < 1 || guest_num > curr->guest_count) {
                    display_error_message("Invalid guest number.");
                } else {
                    update_person(&curr->guests[guest_num - 1]);
                }
                break;
            }
            case 0:
                break;
            default:
                display_error_message("Invalid choice.");
        }
 
        if (choice != 0) {
            printf("\n  Press Enter to continue...");
            getchar();
        }
 
    } while (choice != 0);
}
 
/* =========================================================
   PRIORITY MENU
   ========================================================= */
void priority_menu(Category **head) {
    int choice;
    do {
        system("clear");
        print_banner();
        printf("  --- PRIORITY MANAGEMENT (DPR) ---\n\n");
        printf("  1. Sort guests by age (within categories)\n");
        printf("  2. Sort guests by social class\n");
        printf("  3. Sort categories by guest count (Merge Sort)\n");
        printf("  0. Back to main menu\n\n");
 
        choice = read_int("  Your choice: ");
 
        switch (choice) {
            case 1:
                prioritize_by_age(*head);
                display_all_guests(*head);
                break;
            case 2:
                prioritize_by_social_class(*head);
                display_all_guests(*head);
                break;
            case 3:
                merge_sort_categories(head);
                display_all_categories(*head);
                break;
            case 0:
                break;
            default:
                display_error_message("Invalid choice.");
        }
 
        if (choice != 0) {
            printf("\n  Press Enter to continue...");
            getchar();
        }
 
    } while (choice != 0);
}
 
/* =========================================================
   GIFT MENU
   ========================================================= */
void gift_menu(Gift gifts[], int *count) {
    int choice;
    do {
        system("clear");
        print_banner();
        printf("  --- GIFT MANAGEMENT ---\n\n");
        printf("  1. Register new gift\n");
        printf("  2. Display all gifts\n");
        printf("  3. Update a gift\n");
        printf("  4. Delete a gift\n");
        printf("  5. Show total gift value\n");
        printf("  6. Save gifts to file\n");
        printf("  0. Back to main menu\n\n");
 
        choice = read_int("  Your choice: ");
 
        switch (choice) {
            case 1:
                printf("\n  === Register Gift ===\n");
                register_gift(gifts, count);
                break;
            case 2:
                display_gifts(gifts, *count);
                break;
            case 3: {
                display_gifts(gifts, *count);
                int id = read_int("\n  Enter gift ID to update: ");
                update_gifts(gifts, *count, id);
                break;
            }
            case 4: {
                display_gifts(gifts, *count);
                int id = read_int("\n  Enter gift ID to delete: ");
                if (confirm_action("Delete this gift?")) {
                    delete_gift(gifts, count, id);
                }
                break;
            }
            case 5:
                printf("\n  Total gift value: %.0f FCFA\n",
                       total_gift_value(gifts, *count));
                break;
            case 6:
                save_gifts(gifts, *count, GIFTS_FILE);
                break;
            case 0:
                break;
            default:
                display_error_message("Invalid choice.");
        }
 
        if (choice != 0) {
            printf("\n  Press Enter to continue...");
            getchar();
        }
 
    } while (choice != 0);
}
 
/* =========================================================
   MAIN MENU - Application Entry Point
   This is the top-level controller. It never returns until
   the user chooses Exit (0).
   ========================================================= */
void display_main_menu(Category **head, Gift gifts[], int *gcount) {
    int choice;
 
    /* Load saved gifts on startup */
    load_gifts(gifts, gcount, GIFTS_FILE);
 
    do {
        system("clear");
        print_banner();
        print_separator();
        printf("  Registered guests : %d\n", count_guest(*head));
        printf("  Registered gifts  : %d  (Total: %.0f FCFA)\n",
               *gcount, total_gift_value(gifts, *gcount));
        print_separator();
        printf("\n");
        printf("  1. Category Management\n");
        printf("  2. Person Management\n");
        printf("  3. Priority Management\n");
        printf("  4. Gift Management\n");
        printf("  0. Save & Exit\n\n");
 
        choice = read_int("  Your choice: ");
 
        switch (choice) {
            case 1: category_menu(head);
			        break;
            case 2: person_menu(*head);
			        break;
            case 3: priority_menu(head);
			        break;
            case 4: gift_menu(gifts, gcount);
				    break;
            case 0:
                /* Auto-save gifts on exit */
                save_gifts(gifts, *gcount, GIFTS_FILE);
                free_list(head);
                printf("\n  Goodbye! Data saved.\n\n");
                break;
            default:
                display_error_message("Please enter 0-4.");
                printf("\n  Press Enter to continue...");
                getchar();
        }
 
    } while (choice != 0);
}
 

