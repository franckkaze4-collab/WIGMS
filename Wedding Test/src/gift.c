#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>
#else
  #include <sys/stat.h>
#endif

#include "gift.h"
#include "person.h" /* for clear_input_buffer() */

int gift_id_counter = 1;

static void ensure_data_dir(void) {
#ifdef _WIN32
    /* Best-effort: ignore errors if it already exists */
    _mkdir("data");
#else
    mkdir("data", 0755);
#endif
}

/* ----------------------------------------------------------
   register_gift() - console-interactive
   ---------------------------------------------------------- */
void register_gift(Gift gifts[], int *count) {
    if (!gifts || !count) return;
    if (*count >= MAX_GIFTS) {
        printf("  Gift registry is full (%d gifts).\n", MAX_GIFTS);
        return;
    }

    Gift g;
    g.gift_id = gift_id_counter++;

    printf("  Gift name      : ");
    fgets(g.name, sizeof(g.name), stdin);
    g.name[strcspn(g.name, "\n")] = '\0';

    printf("  Gift value (FCFA): ");
    scanf("%f", &g.value);
    clear_input_buffer();

    printf("  Guest ID who gave this gift: ");
    scanf("%d", &g.guest_id);
    clear_input_buffer();

    gifts[*count] = g;
    (*count)++;

    printf("  Gift '%s' registered (ID: %d).\n", g.name, g.gift_id);
}

void display_gifts(Gift gifts[], int count) {
    if (!gifts || count <= 0) {
        printf("  No gifts registered yet.\n");
        return;
    }

    printf("\n  %-5s %-25s %-12s %-10s\n",
           "ID", "Gift Name", "Value(FCFA)", "Guest ID");
    printf("  %-5s %-25s %-12s %-10s\n",
           "---", "-------------------------", "----------", "--------");

    float total = 0.0f;
    for (int i = 0; i < count; i++) {
        printf("  %-5d %-25s %-12.0f %-10d\n",
               gifts[i].gift_id,
               gifts[i].name,
               gifts[i].value,
               gifts[i].guest_id);
        total += gifts[i].value;
    }
    printf("  -----------------------------------------------\n");
    printf("  TOTAL VALUE: %.0f FCFA  (%d gifts)\n", total, count);
}

void update_gifts(Gift gifts[], int count, int gift_id) {
    if (!gifts) return;

    int idx = -1;
    for (int i = 0; i < count; i++) {
        if (gifts[i].gift_id == gift_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("  Gift ID %d not found.\n", gift_id);
        return;
    }

    char buf[100];
    printf("  New name (Enter to keep [%s]): ", gifts[idx].name);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) {
        strncpy(gifts[idx].name, buf, sizeof(gifts[idx].name) - 1);
        gifts[idx].name[sizeof(gifts[idx].name) - 1] = '\0';
    }

    printf("  New value (0 to keep [%.0f]): ", gifts[idx].value);
    float v = 0.0f;
    scanf("%f", &v);
    clear_input_buffer();
    if (v != 0.0f) gifts[idx].value = v;

    printf("  Gift updated.\n");
}

void delete_gift(Gift gifts[], int *count, int gift_id) {
    if (!gifts || !count) return;

    int idx = -1;
    for (int i = 0; i < *count; i++) {
        if (gifts[i].gift_id == gift_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("  Gift ID %d not found.\n", gift_id);
        return;
    }

    for (int i = idx; i < *count - 1; i++) {
        gifts[i] = gifts[i + 1];
    }
    (*count)--;
    printf("  Gift ID %d deleted.\n", gift_id);
}

float total_gift_value(Gift gifts[], int count) {
    float total = 0.0f;
    for (int i = 0; i < count; i++) {
        total += gifts[i].value;
    }
    return total;
}

void save_gifts(Gift gifts[], int count, const char *filename) {
    if (!gifts || !filename) return;
    if (count < 0) count = 0;
    if (count > MAX_GIFTS) count = MAX_GIFTS;

    ensure_data_dir();
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Cannot open '%s' for writing: %s\n",
                filename, strerror(errno));
        return;
    }

    fwrite(&count, sizeof(int), 1, f);
    fwrite(gifts, sizeof(Gift), count, f);
    fclose(f);

    printf("  %d gifts saved to '%s'.\n", count, filename);
}

void load_gifts(Gift gifts[], int *count, const char *filename) {
    if (!gifts || !count || !filename) return;

    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        /* First run: file doesn't exist */
        *count = 0;
        return;
    }

    int file_count = 0;
    if (fread(&file_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        *count = 0;
        return;
    }

    if (file_count < 0) file_count = 0;
    if (file_count > MAX_GIFTS) file_count = MAX_GIFTS;

    int read = (int)fread(gifts, sizeof(Gift), file_count, f);
    fclose(f);
    *count = read;

    if (read > 0) {
        gift_id_counter = gifts[read - 1].gift_id + 1;
    } else {
        gift_id_counter = 1;
    }
}

/* =========================================================
   GUI-friendly wrappers expected by ui_gtk.c
   ========================================================= */

void gift_register(Gift gifts[], int *count,
                    const char *name, float value, int guest_id) {
    if (!gifts || !count) return;
    if (*count >= MAX_GIFTS) return;

    Gift g;
    g.gift_id = gift_id_counter++;
    strncpy(g.name, name ? name : "Gift", sizeof(g.name) - 1);
    g.name[sizeof(g.name) - 1] = '\0';
    g.value = value;
    g.guest_id = guest_id;

    gifts[*count] = g;
    (*count)++;
}

void gift_display(Gift gifts[], int count) {
    display_gifts(gifts, count);
}

void gift_update(Gift gifts[], int count, int gift_id,
                  const char *name, float value, int guest_id) {
    if (!gifts) return;
    int idx = -1;
    for (int i = 0; i < count; i++) {
        if (gifts[i].gift_id == gift_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return;

    if (name && name[0]) {
        strncpy(gifts[idx].name, name, sizeof(gifts[idx].name) - 1);
        gifts[idx].name[sizeof(gifts[idx].name) - 1] = '\0';
    }
    gifts[idx].value = value;
    gifts[idx].guest_id = guest_id;
}

void gift_delete(Gift gifts[], int *count, int gift_id) {
    delete_gift(gifts, count, gift_id);
}

float gift_total_value(Gift gifts[], int count) {
    return total_gift_value(gifts, count);
}

void gift_save(Gift gifts[], int count, const char *filename) {
    save_gifts(gifts, count, filename);
}

void gift_load(Gift gifts[], int *count, const char *filename) {
    load_gifts(gifts, count, filename);
}

