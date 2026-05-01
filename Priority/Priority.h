#ifndef PRIORITY_H
#define PRIORITY_H

#include <stdio.h>

// Side enum as defined in Module 1 requirements
typedef enum { LE, LA } Side;

// Person structure representing a guest
typedef struct Person {
    int id;                 // Managed automatically
    char name[100];
    int age;                
    char social_class[50];  // Criterion for prioritize_by_social_class
    Side side;
    int needs_parking;      // Added for the Parking Management requirement
    struct Person* next;   // For linked list implementation
} Person;

// Module 3: DPR (Divide-Process-Recombine) Functions
Person* divide_list(Person* head);
Person* merge_lists(Person* a, Person* b, int criteria);
Person* merge_sort_process(Person* head, int criteria);

// Persistence Functions
void save_guests_to_csv(Person* head, const char* filename);
Person* load_guests_from_csv(const char* filename);

#endif
