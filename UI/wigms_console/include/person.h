#ifndef PERSON_H
#define PERSON_H

#include "types.h"

/* =========================================================
   MODULE 1 - Person Management
   Handles creating, displaying and updating Person records
   ========================================================= */

/* Console-interactive versions */
Person create_person(int id);
void   display_person(Person p);
void   update_person(Person *p);
void   clear_input_buffer(void);

/* Data-driven helpers for GUI usage */
Person create_person_data(int id,
                            const char *name,
                            int age,
                            const char *social_class,
                            Side side);

#endif /* PERSON_H */

