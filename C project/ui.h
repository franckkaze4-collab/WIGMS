#ifndef UI_H
#define UI_H
 
#include "types.h"
 
/* =========================================================
   MODULE 5 - User Interface Management
   ========================================================= */
 
void display_main_menu(Category **head, Gift gifts[], int *gcount);
void category_menu(Category **head);
void person_menu(Category *head);
void priority_menu(Category **head);
void gift_menu(Gift gifts[], int *count);
void display_error_message(const char *msg);
void display_success_message(const char *msg);
int  confirm_action(const char *prompt);
void print_banner(void);
void print_separator(void);
 
#endif /* UI_H */
 

