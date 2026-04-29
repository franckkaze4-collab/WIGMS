#ifndef PERSON_H
#define PERSON_H

typedef enum { LE, LA } Side;

typedef struct {
 int id;
 char name[100];
 int age;
 char social_class[50];
 Side side;
} person;

person create_person();
void display_person(person p);
void update_person(person *p);

#endif
