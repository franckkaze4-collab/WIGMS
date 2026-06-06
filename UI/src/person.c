#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "person.h"

/* ----------------------------------------------------------
   clear_input_buffer()
   After scanf() reads a number it leaves '\n' in the buffer.
   fgets() would then immediately read that leftover newline
   as an empty line. This function discards everything up to
   and including the newline.
   ---------------------------------------------------------- */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

/* ----------------------------------------------------------
   create_person()
   Console-interactive creator.
   ---------------------------------------------------------- */
Person create_person(int id) {
    Person p;
    char buf[10];

    p.id = id;

    printf("  Name        : ");
    fgets(p.name, sizeof(p.name), stdin);
    p.name[strcspn(p.name, "\n")] = '\0';

    printf("  Age         : ");
    scanf("%d", &p.age);
    clear_input_buffer();

    printf("  Social class (VIP/Family/Friend/Other): ");
    fgets(p.social_class, sizeof(p.social_class), stdin);
    p.social_class[strcspn(p.social_class, "\n")] = '\0';

    printf("  Side - (L)e marie / (A) mariee: ");
    fgets(buf, sizeof(buf), stdin);
    p.side = (toupper((unsigned char)buf[0]) == 'L') ? LE : LA;

    return p;
}

/* ----------------------------------------------------------
   create_person_data()
   GUI-friendly creator: no stdin/scanf, just copies inputs.
   ---------------------------------------------------------- */
Person create_person_data(int id,
                            const char *name,
                            int age,
                            const char *social_class,
                            Side side) {
    Person p;
    p.id = id;

    if (!name) name = "Unknown";
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';

    p.age = age;
    if (p.age < 0) p.age = 0;

    if (!social_class) social_class = "Friend";
    strncpy(p.social_class, social_class, sizeof(p.social_class) - 1);
    p.social_class[sizeof(p.social_class) - 1] = '\0';

    p.side = side;
    return p;
}

/* ----------------------------------------------------------
   display_person()
   Console printer.
   ---------------------------------------------------------- */
void display_person(Person p) {
    printf("  [ID: %d] %-25s Age: %3d  Class: %-10s  Side: %s\n",
           p.id,
           p.name,
           p.age,
           p.social_class,
           p.side == LE ? "Le marie" : "La mariee");
}

/* ----------------------------------------------------------
   update_person()
   Console-interactive updater.
   ---------------------------------------------------------- */
void update_person(Person *p) {
    char buf[100];

    printf("  New name (Enter to keep [%s]): ", p->name);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0) strncpy(p->name, buf, sizeof(p->name) - 1);

    printf("  New age (0 to keep [%d]): ", p->age);
    int age = 0;
    scanf("%d", &age);
    clear_input_buffer();
    if (age != 0) p->age = age;

    printf("  New social class (Enter to keep [%s]): ", p->social_class);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) > 0)
        strncpy(p->social_class, buf, sizeof(p->social_class) - 1);

    printf("  New side (L/A, Enter to keep): ");
    fgets(buf, sizeof(buf), stdin);
    if (buf[0] != '\n') {
        p->side = (toupper((unsigned char)buf[0]) == 'L') ? LE : LA;
    }

    printf("  Person updated successfully.\n");
}

