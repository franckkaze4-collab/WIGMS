#ifndef PRIORITY_H
#define PRIORITY_H

#include "types.h"

/* =========================================================
   MODULE 3 - Priority Management (DPR / Merge Sort)
   ========================================================= */

/* Console versions */
void prioritize_by_age(Category *tete);
void prioritize_by_social_class(Category *tete);
void merge_sort_categories(Category **tete);

/* GUI-friendly wrappers (same behavior, different names) */
void priority_sort_by_age(Category *tete);
void priority_sort_by_class(Category *tete);
void priority_merge_sort_categories(Category **tete);

#endif /* PRIORITY_H */

