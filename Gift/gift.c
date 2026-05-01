/* ============================================================
 *  MODULE 4 — Gift Management  (WIGMS)
 *  File   : gift.c
 *  Author : <your name>
 *  Date   : 2025-2026
 * ============================================================ */

#include "GIFT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int next_gift_id(const GiftStore *store)
{
    int max_id = 0;
    for (int i = 0; i < store->count; i++) {
        if (store->gifts[i].gift_id > max_id)
            max_id = store->gifts[i].gift_id;
    }
    return max_id + 1;
}

/* Print a separator line for tables. */
static void print_separator(void)
{
    printf("+--------+------------------------------------+-------------+----------+\n");
}


void init_gift_store(GiftStore *store)
{
    if (!store) return;
    store->count = 0;
    memset(store->gifts, 0, sizeof(store->gifts));
}


 
int register_gift(GiftStore *store)
{
    if (!store) return 0;

    if (store->count >= MAX_GIFTS) {
        printf("[ERROR] Gift store is full (%d/%d).\n", store->count, MAX_GIFTS);
        return 0;
    }

    Gift g;
    memset(&g, 0, sizeof(Gift));

    g.gift_id = next_gift_id(store);

    printf("\n--- Register New Gift (ID: %d) ---\n", g.gift_id);

    printf("Gift name : ");
    if (!fgets(g.name, GIFT_NAME_LEN, stdin)) return 0;
    /* Strip trailing newline */
    g.name[strcspn(g.name, "\n")] = '\0';

    printf("Value     : ");
    if (scanf("%f", &g.value) != 1 || g.value < 0) {
        printf("[ERROR] Invalid value.\n");
        while (getchar() != '\n'); /* flush */
        return 0;
    }
    while (getchar() != '\n'); /* flush newline after scanf */

    printf("Guest ID  : ");
    if (scanf("%d", &g.guest_id) != 1 || g.guest_id <= 0) {
        printf("[ERROR] Invalid guest ID.\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');

    store->gifts[store->count] = g;
    store->count++;

    printf("[OK] Gift \"%s\" registered with ID %d.\n", g.name, g.gift_id);
    return 1;
}


void display_gifts(const GiftStore *store)
{
    if (!store || store->count == 0) {
        printf("No gifts registered yet.\n");
        return;
    }

    print_separator();
    printf("| %-6s | %-34s | %-11s | %-8s |\n",
           "ID", "Name", "Value", "Guest ID");
    print_separator();

    for (int i = 0; i < store->count; i++) {
        const Gift *g = &store->gifts[i];
        printf("| %-6d | %-34s | %11.2f | %-8d |\n",
               g->gift_id, g->name, g->value, g->guest_id);
    }

    print_separator();
    printf("Total gifts : %d  |  Total value : %.2f\n",
           store->count, total_gift_value(store));
}


void display_gift_by_guest(const GiftStore *store, int guest_id)
{
    if (!store) return;

    int found = 0;
    printf("\n--- Gifts for Guest ID %d ---\n", guest_id);
    print_separator();
    printf("| %-6s | %-34s | %-11s | %-8s |\n",
           "ID", "Name", "Value", "Guest ID");
    print_separator();

    for (int i = 0; i < store->count; i++) {
        if (store->gifts[i].guest_id == guest_id) {
            const Gift *g = &store->gifts[i];
            printf("| %-6d | %-34s | %11.2f | %-8d |\n",
                   g->gift_id, g->name, g->value, g->guest_id);
            found++;
        }
    }

    if (!found) {
        printf("| %-66s |\n", "No gifts found for this guest.");
    }
    print_separator();
    printf("Total for guest %d : %.2f\n",
           guest_id, total_gift_value_by_guest(store, guest_id));
}


int update_gift(GiftStore *store, int gift_id)
{
    Gift *g = find_gift_by_id(store, gift_id);
    if (!g) {
        printf("[ERROR] Gift ID %d not found.\n", gift_id);
        return 0;
    }

    printf("\n--- Update Gift ID %d ---\n", gift_id);
    printf("Current name  : %s\n", g->name);
    printf("New name (Enter to keep): ");

    char buffer[GIFT_NAME_LEN];
    if (fgets(buffer, GIFT_NAME_LEN, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) > 0)
            strncpy(g->name, buffer, GIFT_NAME_LEN - 1);
    }

    printf("Current value : %.2f\n", g->value);
    printf("New value (-1 to keep): ");
    float new_val;
    if (scanf("%f", &new_val) == 1 && new_val >= 0)
        g->value = new_val;
    while (getchar() != '\n');

    printf("Current guest ID : %d\n", g->guest_id);
    printf("New guest ID (-1 to keep): ");
    int new_gid;
    if (scanf("%d", &new_gid) == 1 && new_gid > 0)
        g->guest_id = new_gid;
    while (getchar() != '\n');

    printf("[OK] Gift ID %d updated.\n", gift_id);
    return 1;
}


int delete_gift(GiftStore *store, int gift_id)
{
    if (!store) return 0;

    for (int i = 0; i < store->count; i++) {
        if (store->gifts[i].gift_id == gift_id) {
            /* Shift elements left to fill the gap */
            for (int j = i; j < store->count - 1; j++)
                store->gifts[j] = store->gifts[j + 1];
            store->count--;
            printf("[OK] Gift ID %d deleted.\n", gift_id);
            return 1;
        }
    }

    printf("[ERROR] Gift ID %d not found.\n", gift_id);
    return 0;
}


Gift *find_gift_by_id(GiftStore *store, int gift_id)
{
    if (!store) return NULL;
    for (int i = 0; i < store->count; i++) {
        if (store->gifts[i].gift_id == gift_id)
            return &store->gifts[i];
    }
    return NULL;
}


float total_gift_value(const GiftStore *store)
{
    float total = 0.0f;
    if (!store) return total;
    for (int i = 0; i < store->count; i++)
        total += store->gifts[i].value;
    return total;
}


float total_gift_value_by_guest(const GiftStore *store, int guest_id)
{
    float total = 0.0f;
    if (!store) return total;
    for (int i = 0; i < store->count; i++) {
        if (store->gifts[i].guest_id == guest_id)
            total += store->gifts[i].value;
    }
    return total;
}


void sort_gifts_by_value_desc(GiftStore *store)
{
    if (!store || store->count < 2) return;

    for (int i = 0; i < store->count - 1; i++) {
        for (int j = 0; j < store->count - 1 - i; j++) {
            if (store->gifts[j].value < store->gifts[j + 1].value) {
                Gift tmp          = store->gifts[j];
                store->gifts[j]   = store->gifts[j + 1];
                store->gifts[j+1] = tmp;
            }
        }
    }
    printf("[OK] Gifts sorted by descending value.\n");
}


int save_gifts(const GiftStore *store)
{
    if (!store) return 0;

    FILE *fp = fopen(GIFT_FILE, "wb");
    if (!fp) {
        printf("[ERROR] Cannot open %s for writing.\n", GIFT_FILE);
        return 0;
    }

    /* Write count first, then all Gift structs */
    fwrite(&store->count, sizeof(int),  1,            fp);
    fwrite(store->gifts,  sizeof(Gift), store->count, fp);

    fclose(fp);
    printf("[OK] %d gift(s) saved to %s.\n", store->count, GIFT_FILE);
    return 1;
}


int load_gifts(GiftStore *store)
{
    if (!store) return 0;

    FILE *fp = fopen(GIFT_FILE, "rb");
    if (!fp) {
        printf("[INFO] No existing gift file found (%s).\n", GIFT_FILE);
        return 0;
    }

    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1 || count < 0 || count > MAX_GIFTS) {
        printf("[ERROR] Corrupted gift file.\n");
        fclose(fp);
        return 0;
    }

    if (fread(store->gifts, sizeof(Gift), count, fp) != (size_t)count) {
        printf("[ERROR] Failed to read gift records.\n");
        fclose(fp);
        return 0;
    }

    store->count = count;
    fclose(fp);
    printf("[OK] %d gift(s) loaded from %s.\n", store->count, GIFT_FILE);
    return 1;
}


int export_gifts_csv(const GiftStore *store)
{
    if (!store) return 0;

    FILE *fp = fopen(GIFT_FILE_CSV, "w");
    if (!fp) {
        printf("[ERROR] Cannot open %s for writing.\n", GIFT_FILE_CSV);
        return 0;
    }

    /* CSV header */
    fprintf(fp, "gift_id,name,value,guest_id\n");

    for (int i = 0; i < store->count; i++) {
        const Gift *g = &store->gifts[i];
        fprintf(fp, "%d,\"%s\",%.2f,%d\n",
                g->gift_id, g->name, g->value, g->guest_id);
    }

    fclose(fp);
    printf("[OK] Gifts exported to %s.\n", GIFT_FILE_CSV);
    return 1;
}
