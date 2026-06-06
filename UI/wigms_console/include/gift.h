#ifndef GIFT_H
#define GIFT_H

#include "types.h"

/* =========================================================
   MODULE 4 - Gift Management (Array + File Persistence)
   ========================================================= */

/* Console-interactive versions */
void  register_gift(Gift gifts[], int *count);
void  display_gifts(Gift gifts[], int count);
void  update_gifts(Gift gifts[], int count, int gift_id);
void  delete_gift(Gift gifts[], int *count, int gift_id);
float total_gift_value(Gift gifts[], int count);
void  save_gifts(Gift gifts[], int count, const char *filename);
void  load_gifts(Gift gifts[], int *count, const char *filename);

extern int gift_id_counter;

/* =========================================================
   GUI-friendly wrappers (data-driven)
   ========================================================= */
void  gift_register(Gift gifts[], int *count,
                     const char *name, float value, int guest_id);
void  gift_display(Gift gifts[], int count); /* optional, not required */
void  gift_update(Gift gifts[], int count, int gift_id,
                     const char *name, float value, int guest_id);
void  gift_delete(Gift gifts[], int *count, int gift_id);
float gift_total_value(Gift gifts[], int count);
void  gift_save(Gift gifts[], int count, const char *filename);
void  gift_load(Gift gifts[], int *count, const char *filename);

#endif /* GIFT_H */

