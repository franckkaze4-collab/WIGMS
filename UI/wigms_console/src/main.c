#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "ui_console.h"
#include "category.h"
#include "gift.h"
#include "parking.h"
#include "seating.h"
#include "schedule.h"

#define ADMIN_PASSWORD "Roddysenpai"

int main(void) {
    AppState state;
    memset(&state, 0, sizeof(state));

    category_load(&state.cat_head, "data/categories.dat");

    parking_init(&state.parking, 60, 0, 0, 0);
    parking_load(&state.parking, "data/parking.dat");

    memset(&state.dining, 0, sizeof(state.dining));
    seating_load(&state.dining, "data/seating.dat");

    schedule_init(&state.schedule, "Wedding Day", "Venue");
    schedule_load(&state.schedule, "data/schedule.dat");

    gift_load(state.gifts, &state.gift_count, "data/gifts.dat");

    guest_db_load(&state);

    printf("\n");
    printf("  ============================================================\n");
    printf("     WIGMS - Wedding Invitation & Gift Management System\n");
    printf("                   Console Edition\n");
    printf("  ============================================================\n");
    printf("  This app manages your entire wedding event:\n");
    printf("  - Register guests with automatic seating & parking assignment\n");
    printf("  - Browse 20 gift items with models & purchase tracking (max 3)\n");
    printf("  - View/manage parking zones and seating tables\n");
    printf("  - Plan and follow the event schedule timeline\n");
    printf("  ============================================================\n");
    printf("  Two access modes:\n");
    printf("    ADMIN  - Full control over all modules (password protected)\n");
    printf("    GUEST  - Register yourself, select gifts, confirm your seat\n");
    printf("             and parking spot, and view the event schedule\n");
    printf("  ============================================================\n");

    int is_admin = 0;
    while (1) {
        printf("\n");
        printf("  Are you an Admin or a Guest?\n");
        printf("  [1] Admin (requires password)\n");
        printf("  [2] Guest (no password needed)\n");
        printf("  [0] Exit\n");
        printf("  Choose: ");

        int choice;
        scanf("%d", &choice);
        while (getchar() != '\n');

        if (choice == 0) {
            printf("  Goodbye!\n");
            category_free_all(&state.cat_head);
            schedule_free(&state.schedule);
            return 0;
        }

        if (choice == 1) {
            printf("  Enter admin password: ");
            char pass[100];
            int i = 0;
            while (i < 99) {
                char ch = _getch();
                if (ch == '\r') break;
                if (ch == '\b' && i > 0) { printf("\b \b"); i--; continue; }
                pass[i++] = ch;
                putchar('*');
            }
            pass[i] = '\0';
            printf("\n");
            if (strcmp(pass, ADMIN_PASSWORD) == 0) {
                is_admin = 1;
                printf("  Access granted. Welcome Admin!\n");
                break;
            } else {
                printf("  Incorrect password. Access denied.\n");
            }
        } else if (choice == 2) {
            is_admin = 0;
            printf("  Welcome Guest! You have limited access.\n");
            break;
        } else {
            printf("  Invalid option.\n");
        }
    }

    ui_console_run(&state, is_admin);

    category_free_all(&state.cat_head);
    schedule_free(&state.schedule);

    return 0;
}
