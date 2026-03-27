#include <stdio.h>
#include "person.h"

Person create_person() {
    Person p;

    printf("Enter ID: ");
    scanf("%d", &p.id);

    printf("Enter name: ");
    scanf("%s", p.name);

    printf("Enter age: ");
    scanf("%d", &p.age);

    printf("Enter social class: ");
    scanf("%s", p.social_class);

    int s;
    printf("Side (0 = LE, 1 = LA): ");
    scanf("%d", &s);

    p.side = s;

    return p;
}

void display_person(Person p) {
    printf("\nID: %d\n", p.id);
    printf("Name: %s\n", p.name);
    printf("Age: %d\n", p.age);
    printf("Class: %s\n", p.social_class);
}

void update_person(Person *p) {
    printf("New name: ");
    scanf("%s", p->name);

    printf("New age: ");
    scanf("%d", &p->age);
}
