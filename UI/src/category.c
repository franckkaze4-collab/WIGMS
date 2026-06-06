#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>
#else
  #include <sys/stat.h>
#endif

#include "category.h"
#include "person.h"

int category_id_counter = 1;
int person_id_counter   = 1;

static void ensure_data_dir(void) {
#ifdef _WIN32
    _mkdir("data");
#else
    mkdir("data", 0755);
#endif
}

static void safe_str_copy(char *dst, size_t dst_sz, const char *src) {
    if (!dst || dst_sz == 0) return;
    if (!src) src = "";
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

/* =========================================================
   Console-interactive module 2 functions
   ========================================================= */

Category* create_category(int id) {
    Category *c = (Category*)malloc(sizeof(Category));
    if (c == NULL) {
        fprintf(stderr, "ERROR: malloc failed in create_category()\n");
        exit(EXIT_FAILURE);
    }

    c->id = id;
    c->guest_count = 0;
    c->next = NULL;
    c->code[0] = '\0';

    printf("  Category code (e.g. FAM_LE, VIP_LA): ");
    fgets(c->code, sizeof(c->code), stdin);
    c->code[strcspn(c->code, "\n")] = '\0';

    printf("  How many guests in this category? (max 4): ");
    int n = 0;
    scanf("%d", &n);
    clear_input_buffer();
    if (n > 4) n = 4;
    if (n < 0) n = 0;
    c->guest_count = n;

    for (int i = 0; i < n; i++) {
        printf("\n  --- Guest %d ---\n", i + 1);
        c->guests[i] = create_person(person_id_counter++);
    }

    /* Guests beyond guest_count are irrelevant */
    return c;
}

void insert_category(Category **tete) {
    if (!tete) return;
    Category *new_cat = create_category(category_id_counter++);
    new_cat->next = *tete;
    *tete = new_cat;
    printf("\n  Category '%s' inserted successfully.\n", new_cat->code);
}

void delete_category(Category **tete, int id) {
    if (!tete || *tete == NULL) {
        printf("  List is empty.\n");
        return;
    }

    Category *curr = *tete;
    Category *prev = NULL;

    while (curr != NULL && curr->id != id) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        printf("  Category with ID %d not found.\n", id);
        return;
    }

    if (prev == NULL) {
        *tete = curr->next;
    } else {
        prev->next = curr->next;
    }

    printf("  Category '%s' (ID: %d) deleted.\n", curr->code, curr->id);
    free(curr);
}

void update_category(Category *tete, int id) {
    Category *curr = tete;
    while (curr != NULL && curr->id != id) {
        curr = curr->next;
    }

    if (curr == NULL) {
        printf("  Category ID %d not found.\n", id);
        return;
    }

    printf("  New code (Enter to keep [%s]): ", curr->code);
    char buf[50];
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0)
        safe_str_copy(curr->code, sizeof(curr->code), buf);

    printf("  Update guests? (y/n): ");
    char yn[8];
    fgets(yn, sizeof(yn), stdin);
    if (yn[0] == 'y' || yn[0] == 'Y') {
        printf("  How many guests in this category? (max 4): ");
        int n = 0;
        scanf("%d", &n);
        clear_input_buffer();
        if (n > 4) n = 4;
        if (n < 0) n = 0;
        curr->guest_count = n;

        for (int i = 0; i < n; i++) {
            printf("\n  --- Guest %d ---\n", i + 1);
            display_person(curr->guests[i]);
            update_person(&curr->guests[i]);
        }
    }

    printf("  Category updated.\n");
}

int count_guest(Category *tete) {
    int total = 0;
    Category *curr = tete;
    while (curr != NULL) {
        total += curr->guest_count;
        curr = curr->next;
    }
    return total;
}

static void swap_category_data(Category *a, Category *b) {
    if (!a || !b || a == b) return;
    int id = a->id;
    a->id = b->id;
    b->id = id;

    char code[50];
    safe_str_copy(code, sizeof(code), a->code);
    safe_str_copy(a->code, sizeof(a->code), b->code);
    safe_str_copy(b->code, sizeof(b->code), code);

    int gc = a->guest_count;
    a->guest_count = b->guest_count;
    b->guest_count = gc;

    Person gtmp[4];
    memcpy(gtmp, a->guests, sizeof(a->guests));
    memcpy(a->guests, b->guests, sizeof(a->guests));
    memcpy(b->guests, gtmp, sizeof(b->guests));
}

void sort_categories_desc(Category **tete) {
    if (!tete || *tete == NULL || (*tete)->next == NULL) return;

    int swapped;
    do {
        swapped = 0;
        Category *curr = *tete;
        while (curr->next != NULL) {
            if (curr->guest_count < curr->next->guest_count) {
                /* Swap node DATA, not pointers */
                swap_category_data(curr, curr->next);
                swapped = 1;
            }
            curr = curr->next;
        }
    } while (swapped);

    printf("  Categories sorted by guest count (descending).\n");
}

void display_all_categories(Category *tete) {
    if (tete == NULL) {
        printf("  No categories registered yet.\n");
        return;
    }
    printf("\n  %-5s %-20s %-10s\n", "ID", "Code", "Guests");
    printf("  %-5s %-20s %-10s\n", "---", "--------------------", "------");

    Category *curr = tete;
    int nodes = 0;
    while (curr != NULL) {
        nodes++;
        printf("  %-5d %-20s %-10d\n", curr->id, curr->code, curr->guest_count);
        curr = curr->next;
    }
    printf("  Total categories: %d | Total guests: %d\n", nodes, count_guest(tete));
}

void display_all_guests(Category *tete) {
    if (tete == NULL) {
        printf("  No guests registered yet.\n");
        return;
    }

    Category *curr = tete;
    while (curr != NULL) {
        printf("\n  === Category: %s (ID: %d) ===\n", curr->code, curr->id);
        for (int i = 0; i < curr->guest_count; i++) {
            display_person(curr->guests[i]);
        }
        curr = curr->next;
    }
    printf("\n  Grand total guests: %d\n", count_guest(tete));
}

void free_list(Category **tete) {
    if (!tete) return;
    Category *curr = *tete;
    while (curr != NULL) {
        Category *next = curr->next;
        free(curr);
        curr = next;
    }
    *tete = NULL;
}

/* =========================================================
   GUI-friendly wrappers expected by ui_gtk.c
   ========================================================= */

Category* category_create(int id, const char *code) {
    Category *c = (Category*)malloc(sizeof(Category));
    if (!c) {
        fprintf(stderr, "ERROR: malloc failed in category_create()\n");
        exit(EXIT_FAILURE);
    }

    c->id = id;
    c->guest_count = 0;
    c->next = NULL;
    safe_str_copy(c->code, sizeof(c->code), code);

    /* Initialize guest slots (not strictly required, but safe) */
    for (int i = 0; i < 4; i++) {
        c->guests[i].id = 0;
        c->guests[i].name[0] = '\0';
        c->guests[i].age = 0;
        c->guests[i].social_class[0] = '\0';
        c->guests[i].side = LE;
    }

    return c;
}

void category_insert(Category **tete, Category *node) {
    if (!tete || !node) return;
    node->next = *tete;
    *tete = node;
}

Category* category_find_by_id(Category *head, int id) {
    Category *curr = head;
    while (curr != NULL) {
        if (curr->id == id) return curr;
        curr = curr->next;
    }
    return NULL;
}

void category_delete(Category **tete, int id) {
    delete_category(tete, id);
}

void category_sort_desc(Category **tete) {
    sort_categories_desc(tete);
}

void category_free_all(Category **tete) {
    free_list(tete);
}

int category_count_nodes(Category *head) {
    int n = 0;
    Category *curr = head;
    while (curr != NULL) {
        n++;
        curr = curr->next;
    }
    return n;
}

int category_count_all_guests(Category *head) {
    return count_guest(head);
}

int category_get_next_id(Category *head) {
    /* Keep counter in sync with data loaded from disk. */
    int max_id = 0;
    for (Category *c = head; c; c = c->next) {
        if (c->id > max_id) max_id = c->id;
    }
    if (category_id_counter <= max_id) category_id_counter = max_id + 1;
    return category_id_counter++;
}

Category* category_sort_alpha(Category *head) {
    if (!head || !head->next) return head;

    int swapped;
    do {
        swapped = 0;
        for (Category *c = head; c && c->next; c = c->next) {
            if (strcmp(c->code, c->next->code) > 0) {
                swap_category_data(c, c->next);
                swapped = 1;
            }
        }
    } while (swapped);

    return head;
}

Category* category_sort_by_guests(Category *head) {
    /* Sort by guest_count DESC (matches Module 2 default). */
    if (!head || !head->next) return head;
    sort_categories_desc(&head);
    return head;
}

void category_free(Category *head) {
    while (head) {
        Category *next = head->next;
        free(head);
        head = next;
    }
}

/* =========================================================
   Save / Load categories and persons to/from binary file
   ========================================================= */

void category_save(Category *head, const char *filename) {
    if (!filename) return;

    ensure_data_dir();

    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: Cannot open '%s' for writing: %s\n",
                filename, strerror(errno));
        return;
    }

    /* Count nodes first */
    int node_count = category_count_nodes(head);

    fwrite(&node_count, sizeof(int), 1, f);
    fwrite(&category_id_counter, sizeof(int), 1, f);
    fwrite(&person_id_counter, sizeof(int), 1, f);

    Category *curr = head;
    while (curr != NULL) {
        fwrite(curr, sizeof(Category), 1, f);
        curr = curr->next;
    }

    fclose(f);
}

void category_load(Category **head, const char *filename) {
    if (!head || !filename) return;

    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        /* First run: file doesn't exist */
        *head = NULL;
        return;
    }

    int node_count = 0;
    if (fread(&node_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        *head = NULL;
        return;
    }

    if (node_count < 0) node_count = 0;

    fread(&category_id_counter, sizeof(int), 1, f);
    fread(&person_id_counter, sizeof(int), 1, f);

    /* Read nodes and build list in correct order (maintain insertion order) */
    Category *new_head = NULL;
    Category *tail = NULL;

    for (int i = 0; i < node_count; i++) {
        Category *c = (Category*)malloc(sizeof(Category));
        if (c == NULL) break;
        if (fread(c, sizeof(Category), 1, f) != 1) {
            free(c);
            break;
        }
        c->next = NULL;

        if (new_head == NULL) {
            new_head = c;
            tail = c;
        } else {
            tail->next = c;
            tail = c;
        }
    }

    fclose(f);
    *head = new_head;
}

