#include <stdio.h>
#include <string.h>

#include "priority.h"

static Category* get_middle(Category *head) {
    if (!head) return NULL;

    Category *slow = head;
    Category *fast = head->next;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}

static Category* merge(Category *left, Category *right, int by_age) {
    if (left == NULL) return right;
    if (right == NULL) return left;

    int left_val;
    int right_val;

    if (by_age) {
        left_val = (left->guest_count > 0) ? left->guests[0].age : 0;
        right_val = (right->guest_count > 0) ? right->guests[0].age : 0;
    } else {
        left_val = left->guest_count;
        right_val = right->guest_count;
    }

    if (left_val <= right_val) {
        left->next = merge(left->next, right, by_age);
        return left;
    } else {
        right->next = merge(left, right->next, by_age);
        return right;
    }
}

static Category* merge_sort_internal(Category *head, int by_age) {
    if (!head || !head->next) return head;

    Category *mid = get_middle(head);
    Category *right = mid->next;
    mid->next = NULL;

    Category *left_sorted  = merge_sort_internal(head, by_age);
    Category *right_sorted = merge_sort_internal(right, by_age);

    return merge(left_sorted, right_sorted, by_age);
}

void prioritize_by_age(Category *tete) {
    Category *curr = tete;
    while (curr != NULL) {
        for (int i = 1; i < curr->guest_count; i++) {
            Person key = curr->guests[i];
            int j = i - 1;
            while (j >= 0 && curr->guests[j].age > key.age) {
                curr->guests[j + 1] = curr->guests[j];
                j--;
            }
            curr->guests[j + 1] = key;
        }
        curr = curr->next;
    }
    printf("  Guests within each category sorted by age (ascending).\n");
}

static int class_rank(const char *cls) {
    if (!cls) return 4;
    if (strncmp(cls, "VIP", 3) == 0) return 1;
    if (strncmp(cls, "Family", 6) == 0) return 2;
    if (strncmp(cls, "Friend", 6) == 0) return 3;
    return 4;
}

void prioritize_by_social_class(Category *tete) {
    Category *curr = tete;
    while (curr != NULL) {
        for (int i = 1; i < curr->guest_count; i++) {
            Person key = curr->guests[i];
            int j = i - 1;
            while (j >= 0 &&
                   class_rank(curr->guests[j].social_class) >
                   class_rank(key.social_class)) {
                curr->guests[j + 1] = curr->guests[j];
                j--;
            }
            curr->guests[j + 1] = key;
        }
        curr = curr->next;
    }
    printf("  Guests sorted by social class (VIP first).\n");
}

void merge_sort_categories(Category **tete) {
    if (!tete) return;
    *tete = merge_sort_internal(*tete, 0 /* by guest_count */);
    printf("  Categories sorted by guest count using Merge Sort (DPR).\n");
}

void priority_sort_by_age(Category *tete) {
    prioritize_by_age(tete);
}

void priority_sort_by_class(Category *tete) {
    prioritize_by_social_class(tete);
}

void priority_merge_sort_categories(Category **tete) {
    merge_sort_categories(tete);
}

