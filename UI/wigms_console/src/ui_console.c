#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ui_console.h"

/* ---- Helpers ---- */
static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static void press_enter(void) {
    printf("\nPress Enter to continue...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int read_int(void) {
    int v;
    scanf("%d", &v);
    clear_input();
    return v;
}

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

static void print_separator(void) {
    printf("============================================================\n");
}

static void print_header(const char *title) {
    printf("\n");
    print_separator();
    printf("  %s\n", title);
    print_separator();
}

/* ---- Gift Catalog ---- */
void init_gift_catalog(AppState *state) {
    state->gift_catalog_count = MAX_GIFT_CATALOG;

    const char *names[20] = {
        "Plates Set", "Television", "PlayStation", "Chairs Set",
        "Cooking Pots", "Spoons & Forks Set", "Parlour Furniture",
        "Bed Sheets Set", "Blender", "Microwave Oven",
        "Coffee Maker", "Dining Table", "Wall Clock",
        "Lamps & Lighting", "Rug Carpet", "Curtains Set",
        "Wardrobe", "Shoe Rack", "Bookshelf", "Flower Vase Set"
    };
    float prices[20] = {
        25000, 350000, 200000, 150000,
        30000, 15000, 500000,
        20000, 35000, 80000,
        45000, 250000, 15000,
        30000, 75000, 40000,
        300000, 25000, 35000, 12000
    };

    const char *subs[20][5] = {
        {"Ceramic 12pcs", "Porcelain 12pcs", "Stoneware 6pcs"},
        {"32 inch", "43 inch", "55 inch"},
        {"PS5", "PS4", "PS5 + Extra Controller"},
        {"Wooden 4pcs", "Metal 6pcs", "Plastic 8pcs"},
        {"Aluminum Set 3pcs", "Non-stick Set 5pcs", "Stainless Set 4pcs"},
        {"Silver 24pcs", "Stainless 24pcs", "Plastic 36pcs"},
        {"3-Seater Sofa", "Sofa + 2 Chairs", "Full Set 5pcs"},
        {"Cotton 4pcs", "Satin 6pcs", "Silk 4pcs"},
        {"Basic 500W", "Pro 1000W", "Premium 1500W"},
        {"20L Manual", "30L Digital", "40L Convection"},
        {"Drip Filter", "Espresso", "Automatic"},
        {"6-Seater", "8-Seater", "10-Seater"},
        {"Modern Round", "Classic Wooden", "Digital LED"},
        {"Table Lamp Set", "Floor Lamp", "Chandelier"},
        {"Small 3x2m", "Medium 4x3m", "Large 5x4m"},
        {"Cotton 4pcs", "Velvet 6pcs", "Blackout 4pcs"},
        {"2-Door Sliding", "3-Door", "4-Door"},
        {"Plastic 3-Tier", "Metal 5-Tier", "Wooden 4-Tier"},
        {"Small 3-Shelf", "Large 5-Shelf", "Corner Unit"},
        {"Glass 3pcs", "Ceramic 5pcs", "Crystal 3pcs"}
    };

    int sub_counts[20] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

    for (int i = 0; i < MAX_GIFT_CATALOG; i++) {
        state->gift_catalog[i].id = i + 1;
        strncpy(state->gift_catalog[i].name, names[i], sizeof(state->gift_catalog[i].name) - 1);
        state->gift_catalog[i].price = prices[i];
        state->gift_catalog[i].sub_count = sub_counts[i];
        state->gift_catalog[i].purchase_count = 0;
        for (int j = 0; j < sub_counts[i]; j++) {
            strncpy(state->gift_catalog[i].sub_options[j], subs[i][j],
                    sizeof(state->gift_catalog[i].sub_options[j]) - 1);
        }
    }
}

/* ---- Guest DB helpers ---- */
static int guest_db_find_by_id(AppState *state, int id) {
    for (int i = 0; i < state->guest_db_count; i++)
        if (state->guest_db[i].id == id) return i;
    return -1;
}

void guest_db_load(AppState *state) {
    state->guest_db_count = 0;
    FILE *fp = fopen("data/guest_access.csv", "r");
    if (!fp) fp = fopen("data/guests_access.csv", "r");
    if (!fp) return;

    char line[512];
    if (!fgets(line, sizeof(line), fp)) { fclose(fp); return; }
    while (fgets(line, sizeof(line), fp) && state->guest_db_count < MAX_GUEST_DB) {
        GuestRecord rec;
        memset(&rec, 0, sizeof(rec));
        char *tok = strtok(line, ",\r\n");
        if (!tok) continue;
        rec.id = atoi(tok);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.name, tok, sizeof(rec.name)-1);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; rec.age = atoi(tok);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.category, tok, sizeof(rec.category)-1);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.social_class, tok, sizeof(rec.social_class)-1);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.side, tok, sizeof(rec.side)-1);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.seat_code, tok, sizeof(rec.seat_code)-1);
        tok = strtok(NULL, ",\r\n"); if (!tok) continue; strncpy(rec.parking_zone, tok, sizeof(rec.parking_zone)-1);
        state->guest_db[state->guest_db_count++] = rec;
    }
    fclose(fp);
}

/* ---- Save/Load Registered Guests ---- */
void save_registered_guests(AppState *state) {
    FILE *f = fopen("data/registered_guests.dat", "wb");
    if (!f) return;
    fwrite(&state->reg_guest_count, sizeof(int), 1, f);
    fwrite(state->reg_guests, sizeof(RegisteredGuest), state->reg_guest_count, f);
    fclose(f);
}

void load_registered_guests(AppState *state) {
    FILE *f = fopen("data/registered_guests.dat", "rb");
    if (!f) { state->reg_guest_count = 0; return; }
    if (fread(&state->reg_guest_count, sizeof(int), 1, f) != 1) {
        state->reg_guest_count = 0;
        fclose(f);
        return;
    }
    if (state->reg_guest_count > MAX_REGISTERED) state->reg_guest_count = MAX_REGISTERED;
    fread(state->reg_guests, sizeof(RegisteredGuest), state->reg_guest_count, f);
    fclose(f);
}

/* ---- Save/Load Gift Purchases ---- */
void save_gift_purchases(AppState *state) {
    FILE *f = fopen("data/gift_purchases.dat", "wb");
    if (!f) return;
    fwrite(&state->gift_purchase_count, sizeof(int), 1, f);
    fwrite(state->gift_purchases, sizeof(GiftPurchase), state->gift_purchase_count, f);
    fclose(f);
}

void load_gift_purchases(AppState *state) {
    FILE *f = fopen("data/gift_purchases.dat", "rb");
    if (!f) { state->gift_purchase_count = 0; return; }
    if (fread(&state->gift_purchase_count, sizeof(int), 1, f) != 1) {
        state->gift_purchase_count = 0;
        fclose(f);
        return;
    }
    if (state->gift_purchase_count > MAX_GIFT_PURCHASES) state->gift_purchase_count = MAX_GIFT_PURCHASES;
    fread(state->gift_purchases, sizeof(GiftPurchase), state->gift_purchase_count, f);
    fclose(f);
}

/* ---- Find registered guest by name ---- */
static int find_reg_guest_by_name(AppState *state, const char *name) {
    for (int i = 0; i < state->reg_guest_count; i++) {
        if (strcmp(state->reg_guests[i].name, name) == 0) return i;
    }
    return -1;
}

/* Check if a name already exists across ALL guest storage sources */
static int name_exists_anywhere(AppState *state, const char *name) {
    /* Check self-registered guests */
    for (int i = 0; i < state->reg_guest_count; i++) {
        if (strcmp(state->reg_guests[i].name, name) == 0) return 1;
    }
    /* Check admin-added guests in categories */
    for (Category *c = state->cat_head; c; c = c->next) {
        for (int i = 0; i < c->guest_count; i++) {
            if (strcmp(c->guests[i].name, name) == 0) return 1;
        }
    }
    /* Check CSV-loaded database */
    for (int i = 0; i < state->guest_db_count; i++) {
        if (strcmp(state->guest_db[i].name, name) == 0) return 1;
    }
    return 0;
}

/* =========================================================
   GUEST REGISTRATION MODULE
   ========================================================= */
static int is_valid_phone(const char *phone) {
    int digits = 0;
    for (int i = 0; phone[i]; i++) {
        if (isdigit(phone[i])) digits++;
        else if (phone[i] != ' ' && phone[i] != '-' && phone[i] != '+') return 0;
    }
    return digits > 0 && digits <= 9;
}

static int is_valid_email(const char *email) {
    return strchr(email, '@') != NULL;
}

/* Count registered guests in the same category */
static int count_guests_in_category(AppState *state, const char *cat) {
    int count = 0;
    for (int i = 0; i < state->reg_guest_count; i++) {
        if (strcmp(state->reg_guests[i].category, cat) == 0)
            count++;
    }
    return count;
}

/* Auto-assign a seat position (within the guest's category table) and a parking spot */
static void auto_assign_spot(AppState *state, int guest_index, const char *category) {
    RegisteredGuest *g = &state->reg_guests[guest_index];

    /* Seating: position = count of guests already in this category + 1 */
    g->seat_zone = count_guests_in_category(state, category);

    /* Parking: find first available spot in the 60-spot lot */
    int spot = -1;
    for (int i = 0; i < state->parking.total_spots; i++) {
        if (state->parking.spots[i].status == SPOT_AVAILABLE) {
            spot = state->parking.spots[i].spot_id;
            parking_assign(&state->parking, g->id, "AUTO", VEHICLE_CAR, ZONE_VIP);
            break;
        }
    }
    g->parking_spot = spot;
    g->seat_confirmed = 0;
    g->parking_confirmed = 0;
}

static void menu_guest_registration(AppState *state) {
    print_header("Guest Registration");

    if (state->reg_guest_count >= MAX_REGISTERED) {
        printf("  Registration is full. No more guests can register.\n");
        press_enter();
        return;
    }

    if (!state->cat_head) {
        printf("  No categories have been created yet. Please contact the admin.\n");
        press_enter();
        return;
    }

    RegisteredGuest g;
    memset(&g, 0, sizeof(g));

    /* ---- Person base fields (matching admin's create_person) ---- */
    printf("  Name: ");
    fgets(g.name, sizeof(g.name), stdin);
    trim_newline(g.name);
    if (strlen(g.name) == 0) {
        printf("  Name cannot be empty.\n");
        press_enter();
        return;
    }

    /* Check if name already exists in ANY source */
    if (name_exists_anywhere(state, g.name)) {
        printf("  The name '%s' is already registered in the system.\n", g.name);
        press_enter();
        return;
    }

    printf("  Age: ");
    g.age = read_int();
    if (g.age < 1 || g.age > 150) {
        printf("  Invalid age.\n");
        press_enter();
        return;
    }

    printf("  Social class (VIP/Family/Friend/Other): ");
    fgets(g.social_class, sizeof(g.social_class), stdin);
    trim_newline(g.social_class);

    printf("  Side - (L)e marie / (A) mariee: ");
    char buf[10];
    fgets(buf, sizeof(buf), stdin);
    Side side = (toupper((unsigned char)buf[0]) == 'L') ? LE : LA;

    /* ---- Extended fields ---- */
    printf("  Phone number (max 9 digits): ");
    fgets(g.phone, sizeof(g.phone), stdin);
    trim_newline(g.phone);
    if (!is_valid_phone(g.phone)) {
        printf("  Invalid phone number. Must be up to 9 digits.\n");
        press_enter();
        return;
    }

    printf("  Email (must contain @): ");
    fgets(g.email, sizeof(g.email), stdin);
    trim_newline(g.email);
    if (!is_valid_email(g.email)) {
        printf("  Invalid email. Must contain '@'.\n");
        press_enter();
        return;
    }

    /* ---- Category selection (validated against admin categories) ---- */
    printf("\n  Available categories:\n");
    for (Category *c = state->cat_head; c; c = c->next)
        printf("    %s (guests: %d/4)\n", c->code, c->guest_count);

    int valid_cat = 0;
    Category *cat_target = NULL;
    while (!valid_cat) {
        printf("  Category code: ");
        fgets(g.category, sizeof(g.category), stdin);
        trim_newline(g.category);
        if (strlen(g.category) == 0) {
            printf("  Category cannot be empty.\n");
            continue;
        }
        for (Category *c = state->cat_head; c; c = c->next) {
            if (strcmp(c->code, g.category) == 0) {
                valid_cat = 1;
                cat_target = c;
                break;
            }
        }
        if (!valid_cat) {
            printf("  Category '%s' not found. Available: ", g.category);
            for (Category *c = state->cat_head; c; c = c->next)
                printf("%s%s", c->code, c->next ? ", " : "");
            printf("\n");
        }
    }

    /* ---- Unified ID using person_id_counter ---- */
    int person_id = person_id_counter++;

    /* ---- Add guest as a Person to the matched Category (if space allows) ---- */
    if (cat_target->guest_count < 4) {
        Person p = create_person_data(person_id, g.name, g.age, g.social_class, side);
        cat_target->guests[cat_target->guest_count++] = p;
        category_save(state->cat_head, "data/categories.dat");
        printf("\n  You have been added to category '%s'.\n", cat_target->code);
    } else {
        printf("\n  Category '%s' is full. You are registered but not added to the category.\n",
               cat_target->code);
    }

    /* ---- Store as RegisteredGuest with extended info ---- */
    g.id = person_id;
    state->reg_guests[state->reg_guest_count] = g;
    auto_assign_spot(state, state->reg_guest_count, g.category);
    g = state->reg_guests[state->reg_guest_count];
    state->reg_guest_count++;
    save_registered_guests(state);

    printf("\n  Registration successful!\n");
    printf("  Welcome, %s!\n", g.name);
    printf("  Assigned Seat: %s-%d\n", g.category, g.seat_zone);
    if (g.parking_spot >= 0)
        printf("  Assigned Parking Spot: #%d\n", g.parking_spot);
    else
        printf("  No parking spot available.\n");

    press_enter();
}

/* =========================================================
   PRIORITY MODULE (admin only)
   ========================================================= */
static void display_all_registered_guests(AppState *state) {
    if (state->reg_guest_count == 0) {
        printf("  No registered guests.\n");
        return;
    }
    printf("\n  %-5s %-25s %-12s %-25s %-10s %-16s %-8s\n",
           "ID", "Name", "Phone", "Email", "Class", "Seat Code", "ParkSp");
    printf("  %-5s %-25s %-12s %-25s %-10s %-16s %-8s\n",
           "---", "------------------", "----------", "----------------------", "------", "------------", "------");
    for (int i = 0; i < state->reg_guest_count; i++) {
        RegisteredGuest *g = &state->reg_guests[i];
        char seat_code[60];
        sprintf(seat_code, "%s-%d", g->category, g->seat_zone);
        printf("  %-5d %-25s %-12s %-25s %-10s %-16s %-8d\n",
               g->id, g->name, g->phone, g->email, g->social_class,
               seat_code, g->parking_spot);
    }
    printf("\n  Total registered guests: %d\n", state->reg_guest_count);
}

static int class_rank_val(const char *cls) {
    if (!cls || strlen(cls) == 0) return 4;
    if (strcmp(cls, "VIP") == 0) return 1;
    if (strcmp(cls, "Family") == 0) return 2;
    if (strcmp(cls, "Friend") == 0) return 3;
    return 4;
}

static float guest_gift_total(AppState *state, int guest_id) {
    float total = 0;
    for (int i = 0; i < state->gift_purchase_count; i++) {
        if (state->gift_purchases[i].guest_id == guest_id)
            total += state->gift_purchases[i].total_amount;
    }
    return total;
}

static void menu_priority(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Priority Management (Admin)");
        printf("  [1] Display All Registered Guests\n");
        printf("  [2] Sort by Name (Alphabetical)\n");
        printf("  [3] Sort by Social Class (VIP > Family > Friend > Other)\n");
        printf("  [4] Sort by Gift Amount\n");
        printf("  [5] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        switch (opt) {
            case 1:
                display_all_registered_guests(state);
                break;
            case 2: {
                /* Bubble sort by name */
                for (int i = 0; i < state->reg_guest_count - 1; i++) {
                    for (int j = 0; j < state->reg_guest_count - i - 1; j++) {
                        if (strcmp(state->reg_guests[j].name, state->reg_guests[j+1].name) > 0) {
                            RegisteredGuest tmp = state->reg_guests[j];
                            state->reg_guests[j] = state->reg_guests[j+1];
                            state->reg_guests[j+1] = tmp;
                        }
                    }
                }
                printf("  Sorted alphabetically.\n");
                display_all_registered_guests(state);
                break;
            }
            case 3: {
                for (int i = 0; i < state->reg_guest_count - 1; i++) {
                    for (int j = 0; j < state->reg_guest_count - i - 1; j++) {
                        if (class_rank_val(state->reg_guests[j].social_class) >
                            class_rank_val(state->reg_guests[j+1].social_class)) {
                            RegisteredGuest tmp = state->reg_guests[j];
                            state->reg_guests[j] = state->reg_guests[j+1];
                            state->reg_guests[j+1] = tmp;
                        }
                    }
                }
                printf("  Sorted by social class (VIP first).\n");
                display_all_registered_guests(state);
                break;
            }
            case 4: {
                for (int i = 0; i < state->reg_guest_count - 1; i++) {
                    for (int j = 0; j < state->reg_guest_count - i - 1; j++) {
                        float t1 = guest_gift_total(state, state->reg_guests[j].id);
                        float t2 = guest_gift_total(state, state->reg_guests[j+1].id);
                        if (t1 > t2) {
                            RegisteredGuest tmp = state->reg_guests[j];
                            state->reg_guests[j] = state->reg_guests[j+1];
                            state->reg_guests[j+1] = tmp;
                        }
                    }
                }
                printf("  Sorted by gift amount (ascending).\n");
                display_all_registered_guests(state);
                break;
            }
            case 5: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 4) press_enter();
    }
}

/* =========================================================
   GIFT MODULE - Admin & Guest
   ========================================================= */

/* Admin view: list all gifts with purchase history */
static void menu_gifts_admin(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Gift Management (Admin)");
        printf("  [1] View Gift Catalog (20 items)\n");
        printf("  [2] View All Guest Purchases\n");
        printf("  [3] View Purchase Totals\n");
        printf("  [4] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        switch (opt) {
            case 1: {
                print_header("Gift Catalog");
                for (int i = 0; i < state->gift_catalog_count; i++) {
                    GiftCatalogItem *item = &state->gift_catalog[i];
                    printf("  [%2d] %-30s Price: %.0f FCFA (x%d sold, max 3)\n",
                           item->id, item->name, item->price, item->purchase_count);
                    for (int j = 0; j < item->sub_count; j++) {
                        printf("       %d. %s\n", j+1, item->sub_options[j]);
                    }
                }
                break;
            }
            case 2: {
                print_header("All Gift Purchases");
                if (state->gift_purchase_count == 0) {
                    printf("  No purchases yet.\n");
                } else {
                    printf("  %-8s %-25s %-30s %-10s %-10s\n",
                           "GuestID", "Guest Name", "Gift", "Qty", "Total");
                    for (int i = 0; i < state->gift_purchase_count; i++) {
                        GiftPurchase *p = &state->gift_purchases[i];
                        char gname[100] = "?";
                        char sub[50] = "";
                        int gi = -1;
                        for (int j = 0; j < state->gift_catalog_count; j++) {
                            if (state->gift_catalog[j].id == p->catalog_item_id) {
                                strncpy(gname, state->gift_catalog[j].name, sizeof(gname)-1);
                                gi = j;
                                break;
                            }
                        }
                        if (gi >= 0 && p->sub_index >= 0 && p->sub_index < state->gift_catalog[gi].sub_count) {
                            strncpy(sub, state->gift_catalog[gi].sub_options[p->sub_index], sizeof(sub)-1);
                        }
                        char guest_name[100] = "Unknown";
                        for (int k = 0; k < state->reg_guest_count; k++) {
                            if (state->reg_guests[k].id == p->guest_id) {
                                strncpy(guest_name, state->reg_guests[k].name, sizeof(guest_name)-1);
                                break;
                            }
                        }
                        printf("  %-8d %-25s %s - %-20s %-10d %.0f FCFA\n",
                               p->guest_id, guest_name, gname, sub, p->quantity, p->total_amount);
                    }
                }
                break;
            }
            case 3: {
                float grand_total = 0;
                for (int i = 0; i < state->gift_purchase_count; i++)
                    grand_total += state->gift_purchases[i].total_amount;
                printf("\n  Total purchases: %d\n", state->gift_purchase_count);
                printf("  Grand total value: %.0f FCFA\n", grand_total);
                break;
            }
            case 4: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 3) press_enter();
    }
}

/* Guest view: choose gifts from catalog */
static void menu_gifts_guest(AppState *state) {
    print_header("Gift Selection (Guest)");

    printf("  Enter your name: ");
    char name[100];
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    int gidx = find_reg_guest_by_name(state, name);
    if (gidx < 0) {
        printf("  You are not registered. Please register first in Guest Module.\n");
        press_enter();
        return;
    }

    int guest_id = state->reg_guests[gidx].id;

    int choosing = 1;
    while (choosing) {
        print_header("Available Gifts");
        printf("  Choose a gift from the catalog:\n\n");
        for (int i = 0; i < state->gift_catalog_count; i++) {
            GiftCatalogItem *item = &state->gift_catalog[i];
            if (item->purchase_count >= 3) {
                printf("  [%2d] %-30s Price: %.0f FCFA (UNAVAILABLE - sold out)\n",
                       item->id, item->name, item->price);
            } else {
                printf("  [%2d] %-30s Price: %.0f FCFA (sold: %d/3)\n",
                       item->id, item->name, item->price, item->purchase_count);
            }
        }
        printf("\n  [0] Finish selecting\n");
        print_separator();
        printf("  Choose item number: ");

        int choice = read_int();
        if (choice == 0) {
            choosing = 0;
            break;
        }

        if (choice < 1 || choice > state->gift_catalog_count) {
            printf("  Invalid choice.\n");
            press_enter();
            continue;
        }

        GiftCatalogItem *item = &state->gift_catalog[choice - 1];
        if (item->purchase_count >= 3) {
            printf("  Sorry, this gift is no longer available (sold out).\n");
            press_enter();
            continue;
        }

        /* Show sub-options */
        printf("\n  Sub-options for %s:\n", item->name);
        for (int j = 0; j < item->sub_count; j++) {
            printf("    [%d] %s\n", j+1, item->sub_options[j]);
        }
        printf("  Choose sub-option: ");
        int sub = read_int();
        if (sub < 1 || sub > item->sub_count) sub = 1;

        printf("  Quantity to purchase: ");
        int qty = read_int();
        if (qty < 1) qty = 1;

        float total = item->price * qty;
        printf("\n  Summary: %s (%s) x%d = %.0f FCFA\n",
               item->name, item->sub_options[sub-1], qty, total);
        printf("  Confirm purchase? (y/n): ");
        char confirm[10];
        fgets(confirm, sizeof(confirm), stdin);
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            item->purchase_count++;

            GiftPurchase gp;
            gp.guest_id = guest_id;
            gp.catalog_item_id = item->id;
            gp.sub_index = sub - 1;
            gp.quantity = qty;
            gp.total_amount = total;

            state->gift_purchases[state->gift_purchase_count++] = gp;
            save_gift_purchases(state);

            printf("  Purchase confirmed! Thank you!\n");

            if (item->purchase_count >= 3) {
                printf("  (This gift is now sold out.)\n");
            }
        } else {
            printf("  Purchase cancelled.\n");
        }
        press_enter();
    }
}

/* ---- Combined Gift Menu (dispatches based on role) ---- */
static void menu_gifts(AppState *state, int is_admin) {
    if (is_admin) {
        menu_gifts_admin(state);
    } else {
        menu_gifts_guest(state);
    }
}

/* =========================================================
   PARKING MODULE - Admin & Guest
   ========================================================= */

static void display_parking_yard(ParkingLot *lot) {
    print_header("Parking Yard - Top View (10x6 Grid)");
    printf("  Legend: [A]=Available  [O]=Occupied  [R]=Reserved  [B]=Blocked\n\n");
    printf("  ");
    for (int col = 1; col <= 10; col++) printf(" %2d ", col);
    printf("\n");
    for (int row = 0; row < 6; row++) {
        printf("  %d ", row + 1);
        for (int col = 0; col < 10; col++) {
            int idx = row * 10 + col;
            if (idx >= lot->total_spots) { printf(" -- "); continue; }
            ParkingSpot *s = &lot->spots[idx];
            char ch = '?';
            switch (s->status) {
                case SPOT_AVAILABLE: ch = 'A'; break;
                case SPOT_OCCUPIED:  ch = 'O'; break;
                case SPOT_RESERVED:  ch = 'R'; break;
                case SPOT_BLOCKED:   ch = 'B'; break;
            }
            printf(" [%c] ", ch);
        }
        printf("\n");
    }
}

static void menu_parking_admin(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Parking Management (Admin)");
        printf("  [1] Assign Parking Spot\n");
        printf("  [2] Release Parking Spot\n");
        printf("  [3] Reserve Parking Spot\n");
        printf("  [4] Block/Unblock Spot\n");
        printf("  [5] Find by Guest ID\n");
        printf("  [6] Find by Plate\n");
        printf("  [7] Display All Spots\n");
        printf("  [8] Display Yard View\n");
        printf("  [9] Parking Statistics\n");
        printf("  [10] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        int id, spot_id;
        char plate[20], note[100];
        ParkingSpot *sp;

        switch (opt) {
            case 1:
                printf("  Guest ID: "); id = read_int();
                printf("  Plate: "); fgets(plate, sizeof(plate), stdin); trim_newline(plate);
                printf("  Vehicle (0=Car, 1=Moto, 2=Bus, 3=VIP): ");
                int vt = read_int();
                if (vt < 0 || vt > 3) vt = 0;
                printf("  Preferred Zone (0=VIP, 1=Standard, 2=Moto, 3=Bus): ");
                int pz = read_int();
                if (pz < 0 || pz >= ZONE_COUNT) pz = ZONE_STANDARD;
                spot_id = parking_assign(&state->parking, id, plate, (VehicleType)vt, (ParkingZone)pz);
                if (spot_id >= 0)
                    printf("  Assigned to spot #%d\n", spot_id);
                else
                    printf("  No available spot.\n");
                break;
            case 2:
                printf("  Spot ID to release: ");
                if (parking_release(&state->parking, read_int()))
                    printf("  Spot released.\n");
                else
                    printf("  Release failed.\n");
                break;
            case 3:
                printf("  Spot ID: "); spot_id = read_int();
                printf("  Guest ID: "); id = read_int();
                printf("  Note: "); fgets(note, sizeof(note), stdin); trim_newline(note);
                if (parking_reserve(&state->parking, spot_id, id, note))
                    printf("  Spot reserved.\n");
                else
                    printf("  Reservation failed.\n");
                break;
            case 4:
                printf("  Spot ID: "); spot_id = read_int();
                printf("  (B)lock or (U)nblock? ");
                char c = getchar(); clear_input();
                if (toupper(c) == 'B') {
                    printf("  Reason: "); fgets(note, sizeof(note), stdin); trim_newline(note);
                    if (parking_block(&state->parking, spot_id, note))
                        printf("  Spot blocked.\n");
                    else
                        printf("  Block failed.\n");
                } else {
                    if (parking_unblock(&state->parking, spot_id))
                        printf("  Spot unblocked.\n");
                    else
                        printf("  Unblock failed.\n");
                }
                break;
            case 5:
                printf("  Guest ID: ");
                sp = parking_find_by_guest(&state->parking, read_int());
                if (sp)
                    printf("  Found at spot #%d, Zone %s, Plate: %s\n",
                           sp->spot_id, parking_zone_name(sp->zone), sp->plate);
                else
                    printf("  No parking found.\n");
                break;
            case 6:
                printf("  Plate: "); fgets(plate, sizeof(plate), stdin); trim_newline(plate);
                sp = parking_find_by_plate(&state->parking, plate);
                if (sp)
                    printf("  Found spot #%d, Guest ID: %d, Zone: %s\n",
                           sp->spot_id, sp->guest_id, parking_zone_name(sp->zone));
                else
                    printf("  Plate not found.\n");
                break;
            case 7:
                print_header("All Parking Spots");
                for (int i = 0; i < state->parking.total_spots; i++) {
                    ParkingSpot *s = &state->parking.spots[i];
                    printf("  #%3d | Zone %s | %-12s | Guest: %3d | Plate: %-10s\n",
                           s->spot_id, parking_zone_name(s->zone),
                           parking_status_name(s->status), s->guest_id, s->plate);
                }
                break;
            case 8: display_parking_yard(&state->parking); break;
            case 9: {
                ParkingStats st = parking_get_stats(&state->parking);
                print_header("Parking Statistics");
                printf("  Total Spots: %d\n", st.total_spots);
                printf("  Available:    %d\n", st.available);
                printf("  Occupied:     %d\n", st.occupied);
                printf("  Reserved:     %d\n", st.reserved);
                printf("  Blocked:      %d\n", st.blocked);
                printf("  VIP Available: %d\n", st.vip_available);
                printf("  VIP Occupied:  %d\n", st.vip_occupied);
                break;
            }
            case 10: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 9) press_enter();
    }
}

static void menu_parking_guest(AppState *state) {
    print_header("Parking (Guest)");

    printf("  Enter your name: ");
    char name[100];
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    int gidx = find_reg_guest_by_name(state, name);
    if (gidx < 0) {
        printf("  You are not registered. Please register first.\n");
        press_enter();
        return;
    }

    RegisteredGuest *g = &state->reg_guests[gidx];

    if (g->parking_spot < 0) {
        printf("  No parking spot was assigned to you.\n");
        press_enter();
        return;
    }

    int row = (g->parking_spot - 1) / 10 + 1;
    int col = (g->parking_spot - 1) % 10 + 1;
    printf("  Your assigned parking spot: #%d (Row %d, Col %d)\n", g->parking_spot, row, col);

    if (g->parking_confirmed) {
        printf("  You have already confirmed your parking spot.\n");
    } else {
        printf("  Have you occupied this parking spot? (y/n): ");
        char confirm[10];
        fgets(confirm, sizeof(confirm), stdin);
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            g->parking_confirmed = 1;
            save_registered_guests(state);
            printf("  Parking confirmed! Thank you.\n");
        } else {
            printf("  Please occupy your assigned spot when possible.\n");
        }
    }

    press_enter();
}

/* ---- Combined Parking Menu ---- */
static void menu_parking(AppState *state, int is_admin) {
    if (is_admin) {
        menu_parking_admin(state);
    } else {
        menu_parking_guest(state);
    }
}

/* =========================================================
   SEATING MODULE - Admin & Guest
   ========================================================= */

static void display_hall_view(DiningHall *hall) {
    print_header("Ceremony Hall - Top View");
    printf("  Legend: Table Types shown per row\n\n");

    if (hall->table_count == 0) {
        printf("  No tables set up yet.\n");
        return;
    }

    for (int i = 0; i < hall->table_count; i++) {
        DiningTable *t = &hall->tables[i];
        printf("  Table #%d (ID:%d) | %-15s | Cap:%2d | Assigned:%2d | %s\n",
               t->table_number, t->table_id,
               seating_table_type_name(t->type),
               t->capacity, t->assigned_count, t->name);
        printf("  Seats: ");
        for (int s = 0; s < t->capacity; s++) {
            SeatAssignment *sa = &t->seats[s];
            if (sa->guest_id >= 0)
                printf("[%2d:%-12s]", sa->guest_id, sa->guest_name);
            else
                printf("[empty       ]");
        }
        printf("\n\n");
    }
}

static void menu_seating_admin(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Seating Management (Admin)");
        printf("  (Tables are auto-created per category)\n");
        printf("  [1] Add Table\n");
        printf("  [2] Assign Guest to Table\n");
        printf("  [3] Remove Guest from Table\n");
        printf("  [4] Move Guest\n");
        printf("  [5] Update RSVP\n");
        printf("  [6] Display All Tables\n");
        printf("  [7] Display Hall View\n");
        printf("  [8] Meal Summary\n");
        printf("  [9] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        int tid, gid;
        char name[100];
        DiningTable *tbl;
        SeatAssignment *sa;

        switch (opt) {
            case 1:
                printf("  Table Type (0=VIP,1=Family,2=Friends,3=Children,4=Staff): ");
                int tt = read_int();
                if (tt < 0 || tt > 4) tt = 0;
                printf("  Capacity (1-%d): ", MAX_SEATS_PER_TABLE);
                int cap = read_int();
                if (cap < 1) cap = 1;
                if (cap > MAX_SEATS_PER_TABLE) cap = MAX_SEATS_PER_TABLE;
                printf("  Table Name: ");
                fgets(name, sizeof(name), stdin); trim_newline(name);
                tid = seating_add_table(&state->dining, (TableType)tt, cap, name);
                if (tid >= 0) printf("  Table added (ID: %d).\n", tid);
                else printf("  Failed to add table.\n");
                break;
            case 2:
                printf("  Table ID: "); tid = read_int();
                tbl = seating_find_table(&state->dining, tid);
                if (!tbl) { printf("  Table not found.\n"); break; }
                printf("  Guest ID: "); gid = read_int();
                printf("  Guest Name: "); fgets(name, sizeof(name), stdin); trim_newline(name);
                printf("  Meal (0=Meat,1=Fish,2=Vegetarian,3=Kids,4=Not Set): ");
                int meal = read_int();
                if (meal < 0 || meal > 4) meal = 4;
                printf("  Diet (0=None,1=Halal,2=Kosher,3=Allergy,4=Diabetic): ");
                int diet = read_int();
                if (diet < 0 || diet > 4) diet = 0;
                int seat_id = seating_assign_guest(&state->dining, tid, gid, name,
                                                   (MealPreference)meal, (DietaryRestriction)diet);
                if (seat_id >= 0) printf("  Assigned to seat #%d.\n", seat_id);
                else printf("  Assignment failed.\n");
                break;
            case 3:
                printf("  Guest ID to remove: ");
                if (seating_remove_guest(&state->dining, read_int()))
                    printf("  Guest removed.\n");
                else
                    printf("  Guest not found.\n");
                break;
            case 4:
                printf("  Guest ID: "); gid = read_int();
                sa = seating_find_guest(&state->dining, gid);
                if (!sa) { printf("  Guest not seated.\n"); break; }
                printf("  New Table ID: ");
                if (seating_move_guest(&state->dining, gid, read_int()))
                    printf("  Guest moved.\n");
                else
                    printf("  Move failed.\n");
                break;
            case 5:
                printf("  Guest ID: "); gid = read_int();
                printf("  RSVP (0=Pending,1=Confirmed,2=Declined,3=No-Show): ");
                int rsvp = read_int();
                if (rsvp < 0 || rsvp > 3) rsvp = 0;
                if (seating_update_rsvp(&state->dining, gid, (RsvpStatus)rsvp))
                    printf("  RSVP updated.\n");
                else
                    printf("  Guest not found.\n");
                break;
            case 6: {
                print_header("All Tables");
                if (state->dining.table_count == 0) {
                    printf("  No tables set up yet.\n");
                } else {
                    printf("  %-5s %-6s %-16s %-4s %-8s %s\n",
                           "ID", "Number", "Type", "Cap", "Assigned", "Name");
                    printf("  %-5s %-6s %-16s %-4s %-8s %s\n",
                           "---", "------", "--------------", "---", "--------", "----");
                    for (int i = 0; i < state->dining.table_count; i++) {
                        DiningTable *t = &state->dining.tables[i];
                        printf("  %-5d %-6d %-16s %-4d %-8d %s\n",
                               t->table_id, t->table_number,
                               seating_table_type_name(t->type),
                               t->capacity, t->assigned_count, t->name);
                    }
                    printf("\n  Total tables: %d / %d\n",
                           state->dining.table_count, MAX_TABLES);
                }
                break;
            }
            case 7: display_hall_view(&state->dining); break;
            case 8: {
                MealSummary ms = seating_meal_summary(&state->dining);
                print_header("Meal Summary");
                printf("  Meat:       %d\n", ms.meat_count);
                printf("  Fish:       %d\n", ms.fish_count);
                printf("  Vegetarian: %d\n", ms.vegetarian_count);
                printf("  Kids Meal:  %d\n", ms.kids_count);
                printf("  Halal:      %d\n", ms.halal_count);
                printf("  Allergy:    %d\n", ms.allergy_count);
                break;
            }
            case 9: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 8) press_enter();
    }
}

static void menu_seating_guest(AppState *state) {
    print_header("Seating (Guest)");

    printf("  Enter your name: ");
    char name[100];
    fgets(name, sizeof(name), stdin);
    trim_newline(name);

    int gidx = find_reg_guest_by_name(state, name);
    if (gidx < 0) {
        printf("  You are not registered. Please register first.\n");
        press_enter();
        return;
    }

    RegisteredGuest *g = &state->reg_guests[gidx];

    char seat_code[60];
    sprintf(seat_code, "%s-%d", g->category, g->seat_zone);
    printf("  Your assigned seat: %s\n", seat_code);

    if (g->seat_confirmed) {
        printf("  You have already confirmed your seat.\n");
    } else {
        printf("  Have you taken your seat? (y/n): ");
        char confirm[10];
        fgets(confirm, sizeof(confirm), stdin);
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            g->seat_confirmed = 1;
            save_registered_guests(state);
            printf("  Seating confirmed! Thank you.\n");
        } else {
            printf("  Please take your assigned seat when possible.\n");
        }
    }

    press_enter();
}

/* ---- Combined Seating Menu ---- */
static void menu_seating(AppState *state, int is_admin) {
    if (is_admin) {
        menu_seating_admin(state);
    } else {
        menu_seating_guest(state);
    }
}

/* =========================================================
   SCHEDULE MODULE - Admin & Guest
   ========================================================= */

static void menu_schedule_admin(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Schedule Management (Admin)");
        printf("  [1] Add Event\n");
        printf("  [2] Remove Event\n");
        printf("  [3] Start Event\n");
        printf("  [4] End Event\n");
        printf("  [5] Delay Event\n");
        printf("  [6] Cancel Event\n");
        printf("  [7] Display All Events\n");
        printf("  [8] Current / Next Event\n");
        printf("  [9] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        char title[100], desc[250], resp[100], loc[100], buf[64];
        int id, mins;
        EventNode *ev;

        switch (opt) {
            case 1:
                printf("  Title: "); fgets(title, sizeof(title), stdin); trim_newline(title);
                printf("  Description: "); fgets(desc, sizeof(desc), stdin); trim_newline(desc);
                printf("  Category (0=Ceremony,1=Reception,2=Dinner,3=Speech,4=Entertainment,5=Photo,6=Logistics,7=Break): ");
                int cat = read_int();
                if (cat < 0 || cat > 7) cat = 0;
                printf("  Priority (0=Critical,1=High,2=Normal,3=Optional): ");
                int pri = read_int();
                if (pri < 0 || pri > 3) pri = 2;
                printf("  Start Hour (0-23): "); int h = read_int();
                printf("  Start Minute (0-59): "); int m = read_int();
                printf("  Duration (minutes): "); mins = read_int();
                if (mins < 1) mins = 30;

                time_t now = time(NULL);
                struct tm *tm_now = localtime(&now);
                tm_now->tm_hour = h;
                tm_now->tm_min = m;
                tm_now->tm_sec = 0;
                time_t start = mktime(tm_now);

                printf("  Responsible: "); fgets(resp, sizeof(resp), stdin); trim_newline(resp);
                printf("  Location: "); fgets(loc, sizeof(loc), stdin); trim_newline(loc);

                ev = schedule_add_event(&state->schedule, title, desc,
                                         (EventCategory)cat, (EventPriority)pri,
                                         start, mins, resp, loc);
                if (ev) printf("  Event added (ID: %d).\n", ev->event_id);
                else printf("  Failed to add event.\n");
                break;
            case 2:
                printf("  Event ID to remove: ");
                if (schedule_remove_event(&state->schedule, read_int()))
                    printf("  Event removed.\n");
                else
                    printf("  Event not found.\n");
                break;
            case 3:
                printf("  Event ID to start: ");
                if (schedule_start_event(&state->schedule, read_int()))
                    printf("  Event started.\n");
                else
                    printf("  Failed to start.\n");
                break;
            case 4:
                printf("  Event ID to end: ");
                if (schedule_end_event(&state->schedule, read_int()))
                    printf("  Event ended.\n");
                else
                    printf("  Failed to end.\n");
                break;
            case 5:
                printf("  Event ID: "); id = read_int();
                printf("  Delay (minutes): "); mins = read_int();
                if (schedule_delay_event(&state->schedule, id, mins))
                    printf("  Event delayed.\n");
                else
                    printf("  Failed to delay.\n");
                break;
            case 6:
                printf("  Event ID to cancel: ");
                if (schedule_cancel_event(&state->schedule, read_int()))
                    printf("  Event cancelled.\n");
                else
                    printf("  Failed to cancel.\n");
                break;
            case 7: {
                print_header("All Events");
                for (ev = state->schedule.head; ev; ev = ev->next) {
                    printf("  [ID: %3d] %-25s | %-15s | %-12s | %s\n",
                           ev->event_id, ev->title,
                           schedule_category_name(ev->category),
                           schedule_status_name(ev->status),
                           schedule_format_hhmm(ev->scheduled_start, buf, sizeof(buf)));
                }
                break;
            }
            case 8: {
                ev = schedule_current_event(&state->schedule);
                if (ev) printf("  Current: %s (%s)\n", ev->title, schedule_status_name(ev->status));
                ev = schedule_next_event(&state->schedule);
                if (ev) {
                    long mins_to = schedule_minutes_to_next(&state->schedule);
                    printf("  Next: %s at %s (in %ld min)\n",
                           ev->title, schedule_format_hhmm(ev->scheduled_start, buf, sizeof(buf)), mins_to);
                }
                break;
            }
            case 9: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 8) press_enter();
    }
}

static void menu_schedule_guest(AppState *state) {
    print_header("Schedule (Guest - View Only)");

    if (state->schedule.event_count == 0) {
        printf("  No events planned yet.\n");
        press_enter();
        return;
    }

    char buf[64];
    printf("\n  Wedding: %s | Venue: %s\n\n",
           state->schedule.wedding_date, state->schedule.venue);
    printf("  %-5s %-25s %-15s %-10s %-5s\n",
           "ID", "Event", "Category", "Status", "Time");
    printf("  %-5s %-25s %-15s %-10s %-5s\n",
           "---", "------------------", "-------------", "--------", "-----");
    for (EventNode *ev = state->schedule.head; ev; ev = ev->next) {
        printf("  [%3d] %-25s %-15s %-10s %s\n",
               ev->event_id, ev->title,
               schedule_category_name(ev->category),
               schedule_status_name(ev->status),
               schedule_format_hhmm(ev->scheduled_start, buf, sizeof(buf)));
    }

    press_enter();
}

/* ---- Combined Schedule Menu ---- */
static void menu_schedule(AppState *state, int is_admin) {
    if (is_admin) {
        menu_schedule_admin(state);
    } else {
        menu_schedule_guest(state);
    }
}

/* =========================================================
   ADMIN MAIN MENU (full access)
   ========================================================= */
static void menu_categories(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Category Management");
        printf("  [1] Create Category (add guests from Guest menu)\n");
        printf("  [2] Delete Category\n");
        printf("  [3] Update Category\n");
        printf("  [4] Display All Categories\n");
        printf("  [5] Display All Guests\n");
        printf("  [6] Count Guests\n");
        printf("  [7] Sort Categories (Descending)\n");
        printf("  [8] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        switch (opt) {
            case 1: insert_category(&state->cat_head); break;
            case 2:
                printf("  Category ID to delete: ");
                delete_category(&state->cat_head, read_int());
                break;
            case 3:
                printf("  Category ID to update: ");
                update_category(state->cat_head, read_int());
                break;
            case 4: display_all_categories(state->cat_head); break;
            case 5: display_all_guests(state->cat_head); break;
            case 6: printf("\n  Total guests: %d\n", count_guest(state->cat_head)); break;
            case 7: sort_categories_desc(&state->cat_head); printf("  Categories sorted.\n"); break;
            case 8: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 7 && opt != 8) press_enter();
    }
}

static void menu_guests_admin(AppState *state) {
    int running = 1;
    while (running) {
        print_header("Person Management (Admin)");
        printf("  [1] List All Guests\n");
        printf("  [2] Find Guest by ID\n");
        printf("  [3] Find Guest by Name\n");
        printf("  [4] Add Guest to Category\n");
        printf("  [5] Display Guest Count\n");
        printf("  [6] Back to Main Menu\n");
        print_separator();
        printf("  Choose: ");

        int opt = read_int();
        switch (opt) {
            case 1: {
                print_header("All Guests (All Sources)");
                int total = 0;

                /* 1. Guests added by admin to category nodes */
                printf("\n  --- Guests in Categories (Admin-Added) ---\n");
                for (Category *c = state->cat_head; c; c = c->next) {
                    for (int i = 0; i < c->guest_count; i++) {
                        Person *p = &c->guests[i];
                        printf("  [ID: %3d] %-25s Age: %3d  Class: %-10s  Category: %s\n",
                               p->id, p->name, p->age, p->social_class, c->code);
                        total++;
                    }
                }

                /* 2. Self-registered guests (reg_guests) */
                if (state->reg_guest_count > 0) {
                    printf("\n  --- Self-Registered Guests ---\n");
                    for (int i = 0; i < state->reg_guest_count; i++) {
                        RegisteredGuest *g = &state->reg_guests[i];
                        char seat_code[60];
                        sprintf(seat_code, "%s-%d", g->category, g->seat_zone);
                        printf("  [ID: %3d] %-25s Age: %3d  Class: %-10s  Category: %s  Seat: %-16s  Park: #%d\n",
                               g->id, g->name, g->age, g->social_class, g->category,
                               seat_code, g->parking_spot);
                        total++;
                    }
                }

                /* 3. CSV-loaded guest database */
                if (state->guest_db_count > 0) {
                    printf("\n  --- Guests from Database (CSV) ---\n");
                    for (int i = 0; i < state->guest_db_count; i++) {
                        GuestRecord *g = &state->guest_db[i];
                        printf("  [ID: %3d] %-25s Age: %3d  Class: %-10s  Category: %s  Side: %-10s  Seat: %-6s  Park: %s\n",
                               g->id, g->name, g->age, g->social_class, g->category,
                               g->side, g->seat_code, g->parking_zone);
                        total++;
                    }
                }

                if (total == 0) printf("  No guests found in any source.\n");
                printf("\n  Grand total guests across all sources: %d\n", total);
                break;
            }
            case 2: {
                printf("  Guest ID: ");
                int id = read_int();
                int found = 0;
                /* Search in categories (admin-added) */
                for (Category *c = state->cat_head; c && !found; c = c->next) {
                    for (int i = 0; i < c->guest_count; i++) {
                        if (c->guests[i].id == id) {
                            Person *p = &c->guests[i];
                            printf("  [ID: %d] Name: %s, Age: %d, Class: %s, Category: %s (Admin-Added)\n",
                                   p->id, p->name, p->age, p->social_class, c->code);
                            found = 1;
                            break;
                        }
                    }
                }
                /* Search in registered guests */
                for (int i = 0; i < state->reg_guest_count && !found; i++) {
                    if (state->reg_guests[i].id == id) {
                        RegisteredGuest *g = &state->reg_guests[i];
                        printf("  [ID: %d] Name: %s, Age: %d, Class: %s, Category: %s (Self-Registered)\n",
                               g->id, g->name, g->age, g->social_class, g->category);
                        found = 1;
                    }
                }
                /* Search in CSV database */
                int i = guest_db_find_by_id(state, id);
                if (i >= 0 && !found) {
                    GuestRecord *g = &state->guest_db[i];
                    printf("  [ID: %d] Name: %s, Age: %d, Class: %s, Category: %s (CSV)\n",
                           g->id, g->name, g->age, g->social_class, g->category);
                    found = 1;
                }
                if (!found) printf("  Guest not found in any source.\n");
                break;
            }
            case 3: {
                printf("  Name (partial): ");
                char query[100];
                fgets(query, sizeof(query), stdin);
                trim_newline(query);
                int found = 0;
                /* Search in categories */
                for (Category *c = state->cat_head; c; c = c->next) {
                    for (int i = 0; i < c->guest_count; i++) {
                        if (strstr(c->guests[i].name, query)) {
                            Person *p = &c->guests[i];
                            printf("  [ID: %3d] %s, Age: %d, Class: %s, Category: %s (Admin-Added)\n",
                                   p->id, p->name, p->age, p->social_class, c->code);
                            found = 1;
                        }
                    }
                }
                /* Search in registered guests */
                for (int i = 0; i < state->reg_guest_count; i++) {
                    if (strstr(state->reg_guests[i].name, query)) {
                        RegisteredGuest *g = &state->reg_guests[i];
                        printf("  [ID: %3d] %s, Age: %d, Class: %s, Category: %s (Self-Registered)\n",
                               g->id, g->name, g->age, g->social_class, g->category);
                        found = 1;
                    }
                }
                /* Search in CSV database */
                for (int i = 0; i < state->guest_db_count; i++) {
                    if (strstr(state->guest_db[i].name, query)) {
                        GuestRecord *g = &state->guest_db[i];
                        printf("  [ID: %3d] %s, Age: %d, Class: %s, Category: %s (CSV)\n",
                               g->id, g->name, g->age, g->social_class, g->category);
                        found = 1;
                    }
                }
                if (!found) printf("  No matches found.\n");
                break;
            }
            case 4: {
                if (!state->cat_head) {
                    printf("  No categories exist. Create one first via Category menu.\n");
                    break;
                }
                if (state->reg_guest_count >= MAX_REGISTERED) {
                    printf("  Registration system is full.\n");
                    break;
                }
                printf("\n  Available categories:\n");
                for (Category *c = state->cat_head; c; c = c->next)
                    printf("    [ID: %d] %s (guests: %d/4)\n", c->id, c->code, c->guest_count);
                printf("  Enter category ID: ");
                int cat_id = read_int();
                Category *target = NULL;
                for (Category *c = state->cat_head; c; c = c->next) {
                    if (c->id == cat_id) { target = c; break; }
                }
                if (!target) {
                    printf("  Category not found.\n");
                    break;
                }
                if (target->guest_count >= 4) {
                    printf("  Category is full (max 4 guests).\n");
                    break;
                }

                /* ---- Person base fields (harmonised with guest flow) ---- */
                char p_name[100];
                printf("  Name: ");
                fgets(p_name, sizeof(p_name), stdin);
                trim_newline(p_name);
                if (strlen(p_name) == 0) {
                    printf("  Name cannot be empty.\n");
                    break;
                }

                /* Check duplicate name globally before proceeding */
                if (name_exists_anywhere(state, p_name)) {
                    printf("  The name '%s' is already registered in the system.\n", p_name);
                    break;
                }

                printf("  Age: ");
                int p_age = read_int();
                if (p_age < 1 || p_age > 150) {
                    printf("  Invalid age.\n");
                    break;
                }

                char p_class[50];
                printf("  Social class (VIP/Family/Friend/Other): ");
                fgets(p_class, sizeof(p_class), stdin);
                trim_newline(p_class);

                printf("  Side - (L)e marie / (A) mariee: ");
                char sbuf[10];
                fgets(sbuf, sizeof(sbuf), stdin);
                Side p_side = (toupper((unsigned char)sbuf[0]) == 'L') ? LE : LA;

                /* Extended info for RegisteredGuest */
                RegisteredGuest rg;
                memset(&rg, 0, sizeof(rg));
                printf("  Phone number (max 9 digits): ");
                fgets(rg.phone, sizeof(rg.phone), stdin);
                trim_newline(rg.phone);
                printf("  Email (must contain @): ");
                fgets(rg.email, sizeof(rg.email), stdin);
                trim_newline(rg.email);

                /* ---- Create and store ---- */
                int person_id = person_id_counter++;
                Person p = create_person_data(person_id, p_name, p_age, p_class, p_side);
                target->guests[target->guest_count++] = p;
                category_save(state->cat_head, "data/categories.dat");

                rg.id = person_id;
                strncpy(rg.name, p_name, sizeof(rg.name) - 1);
                rg.age = p_age;
                strncpy(rg.social_class, p_class, sizeof(rg.social_class) - 1);
                strncpy(rg.category, target->code, sizeof(rg.category) - 1);

                state->reg_guests[state->reg_guest_count] = rg;
                auto_assign_spot(state, state->reg_guest_count, target->code);
                state->reg_guest_count++;
                save_registered_guests(state);

                RegisteredGuest *rgp = &state->reg_guests[state->reg_guest_count - 1];
                printf("  Guest '%s' added to category '%s' with seat %s-%d and parking #%d.\n",
                       p_name, target->code, target->code, rgp->seat_zone, rgp->parking_spot);
                break;
            }
            case 5: {
                int cat_guest_count = category_count_all_guests(state->cat_head);
                printf("\n  Guests in Categories (Admin-Added): %d\n", cat_guest_count);
                printf("  Self-Registered Guests: %d\n", state->reg_guest_count);
                printf("  Guests in CSV Database: %d\n", state->guest_db_count);
                printf("  Grand total: %d\n",
                       cat_guest_count + state->reg_guest_count + state->guest_db_count);
                break;
            }
            case 6: running = 0; break;
            default: printf("  Invalid option.\n");
        }
        if (opt >= 1 && opt <= 5) press_enter();
    }
}

/* ---- Dashboard ---- */
static void menu_dashboard(AppState *state) {
    print_header("Dashboard / Statistics");

    int cat_count = category_count_nodes(state->cat_head);
    int cat_guest_count = category_count_all_guests(state->cat_head);
    printf("  Categories: %d\n", cat_count);
    printf("  Guests in Categories: %d\n", cat_guest_count);
    printf("  Self-Registered Guests: %d\n", state->reg_guest_count);
    printf("  Guests (CSV Database): %d\n", state->guest_db_count);
    printf("  Grand total across all sources: %d\n",
           cat_guest_count + state->reg_guest_count + state->guest_db_count);

    float total_gift_val = total_gift_value(state->gifts, state->gift_count);
    printf("  Gifts Registered: %d\n", state->gift_count);
    printf("  Total Gift Value: %.0f FCFA\n", total_gift_val);

    float purchase_total = 0;
    for (int i = 0; i < state->gift_purchase_count; i++)
        purchase_total += state->gift_purchases[i].total_amount;
    printf("  Gift Purchases: %d (%.0f FCFA)\n", state->gift_purchase_count, purchase_total);

    ParkingStats pst = parking_get_stats(&state->parking);
    printf("\n  Parking:\n");
    printf("    Total Spots: %d\n", pst.total_spots);
    printf("    Available:    %d\n", pst.available);
    printf("    Occupied:     %d\n", pst.occupied);

    MealSummary ms = seating_meal_summary(&state->dining);
    printf("\n  Seating:\n");
    printf("    Tables:  %d / %d\n", state->dining.table_count, MAX_TABLES);
    printf("    Meals:   Meat=%d, Fish=%d, Veg=%d, Kids=%d\n",
           ms.meat_count, ms.fish_count, ms.vegetarian_count, ms.kids_count);

    printf("\n  Schedule Events: %d\n", state->schedule.event_count);

    press_enter();
}

/* ---- Ensure every registered guest has valid seat and parking ---- */
static void migrate_seating_parking(AppState *state) {
    for (int i = 0; i < state->reg_guest_count; i++) {
        RegisteredGuest *g = &state->reg_guests[i];

        /* Fix seat position: if 0 or invalid, recompute based on category */
        int pos = 0;
        for (int j = 0; j < i; j++) {
            if (strcmp(state->reg_guests[j].category, g->category) == 0)
                pos++;
        }
        g->seat_zone = pos + 1;

        /* Fix parking: if no spot assigned or spot not found, assign new */
        if (g->parking_spot < 0) {
            for (int s = 0; s < state->parking.total_spots; s++) {
                if (state->parking.spots[s].status == SPOT_AVAILABLE) {
                    int sid = parking_assign(&state->parking, g->id, "AUTO",
                                              VEHICLE_CAR, ZONE_VIP);
                    g->parking_spot = sid;
                    break;
                }
            }
        }

        g->seat_confirmed = 0;
        g->parking_confirmed = 0;
    }
    save_registered_guests(state);
    parking_save(&state->parking, "data/parking.dat");
}

/* ---- Save all data ---- */
static void save_all(AppState *state) {
    category_save(state->cat_head, "data/categories.dat");
    gift_save(state->gifts, state->gift_count, "data/gifts.dat");
    parking_save(&state->parking, "data/parking.dat");
    seating_save(&state->dining, "data/seating.dat");
    schedule_save(&state->schedule, "data/schedule.dat");
    save_registered_guests(state);
    save_gift_purchases(state);
    printf("  All data saved.\n");
}

/* =========================================================
   MAIN CONSOLE LOOP (role-based)
   ========================================================= */
void ui_console_run(AppState *state, int is_admin) {
    /* Initialize gift catalog on first run */
    if (state->gift_catalog_count == 0) {
        init_gift_catalog(state);
    }

    /* Load persistent data for new modules */
    load_registered_guests(state);
    load_gift_purchases(state);

    /* Fix seat/parking assignments for all guests (including legacy data) */
    migrate_seating_parking(state);

    int running = 1;
    while (running) {
        printf("\n");
        print_separator();
        printf("     WIGMS - %s Mode\n", is_admin ? "Admin" : "Guest");
        print_separator();

        if (is_admin) {
            printf("  [1] Category Management\n");
            printf("  [2] Person Management\n");
            printf("  [3] Priority Management\n");
            printf("  [4] Gift Management\n");
            printf("  [5] Parking Management\n");
            printf("  [6] Seating Management\n");
            printf("  [7] Schedule Management\n");
            printf("  [8] Dashboard / Statistics\n");
            printf("  [9] Save All Data\n");
            printf("  [0] Exit\n");
        } else {
            printf("  [1] Guest Registration\n");
            printf("  [2] Gift Selection\n");
            printf("  [3] Parking\n");
            printf("  [4] Seating\n");
            printf("  [5] Schedule (View Only)\n");
            printf("  [0] Exit\n");
        }
        print_separator();
        printf("  Choose: ");

        int opt = read_int();

        if (is_admin) {
            switch (opt) {
                case 1:  menu_categories(state); break;
                case 2:  menu_guests_admin(state); break;
                case 3:  menu_priority(state); break;
                case 4:  menu_gifts(state, 1); break;
                case 5:  menu_parking(state, 1); break;
                case 6:  menu_seating(state, 1); break;
                case 7:  menu_schedule(state, 1); break;
                case 8:  menu_dashboard(state); break;
                case 9:  save_all(state); press_enter(); break;
                case 0:
                    save_all(state);
                    printf("  Goodbye!\n");
                    running = 0;
                    break;
                default: printf("  Invalid option.\n"); press_enter();
            }
        } else {
            switch (opt) {
                case 1:  menu_guest_registration(state); break;
                case 2:  menu_gifts(state, 0); break;
                case 3:  menu_parking(state, 0); break;
                case 4:  menu_seating(state, 0); break;
                case 5:  menu_schedule(state, 0); break;
                case 0:
                    printf("  Goodbye!\n");
                    running = 0;
                    break;
                default: printf("  Invalid option.\n"); press_enter();
            }
        }
    }
}
