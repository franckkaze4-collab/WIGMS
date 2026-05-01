#include "priority.h"
#include <stdlib.h>
#include <string.h>

/**
 * STEP 1: DIVIDE (DPR Pattern)[span_8](end_span)
 * Splits the linked list into two halves.
 */
Person* divide_list(Person* head) {
    if (!head || !head->next) return NULL;
    
    Person *slow = head, *fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    
    Person* second_half = slow->next;
    slow->next = NULL; // Break the list in two
    return second_half;
}

/**
 * STEP 3: RECOMBINE (DPR Pattern)[span_9](end_span)
 * Merges two sorted lists back together.
 * Criteria: 1 = Age, 2 = Social Class
 */
Person* merge_lists(Person* a, Person* b, int criteria) {
    if (!a) return b;
    if (!b) return a;

    Person* result = NULL;

    // Logic: Lower age or higher social rank comes first
    int should_swap = 0;
    if (criteria == 1) { // Age
        should_swap = (a->age <= b->age);
    } else { // Social Class (Alphabetical for this example)
        should_swap = (strcmp(a->social_class, b->social_class) <= 0);
    }

    if (should_swap) {
        result = a;
        result->next = merge_lists(a->next, b, criteria);
    } else {
        result = b;
        result->next = merge_lists(a, b->next, criteria);
    }
    return result;
}

/**
 * STEP 2: PROCESS (DPR Pattern)[span_10](end_span)
 * The main recursive Merge Sort function.
 */
Person* merge_sort_process(Person* head, int criteria) {
    if (!head || !head->next) return head;

    // Divide
    Person* second = divide_list(head);

    // Process (Recursive Sort)
    head = merge_sort_process(head, criteria);
    second = merge_sort_process(second, criteria);

    // Recombine
    return merge_lists(head, second, criteria);
}

/**
 * PERSISTENCE: Save to CSV
 * Replaces MySQL as requested by the lecturer.
 */
void save_guests_to_csv(Person* head, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    Person* curr = head;
    while (curr) {
        // ID, Name, Age, Class, Side, Parking
        fprintf(file, "%d,%s,%d,%s,%d,%d\n", 
                curr->id, curr->name, curr->age, 
                curr->social_class, curr->side, curr->needs_parking);
        curr = curr->next;
    }
    fclose(file);
}

/**
 * FILE LOADING & AUTO-ID
 * Loads data and ensures the structure is ready for processing.
 */
Person* load_guests_from_csv(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    Person *head = NULL, *tail = NULL;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        Person* new_p = malloc(sizeof(Person));
        sscanf(line, "%d,%[^,],%d,%[^,],%d,%d", 
               &new_p->id, new_p->name, &new_p->age, 
               new_p->social_class, (int*)&new_p->side, &new_p->needs_parking);
        new_p->next = NULL;

        if (!head) head = new_p;
        else tail->next = new_p;
        tail = new_p;
    }
    fclose(file);
    return head;
}
