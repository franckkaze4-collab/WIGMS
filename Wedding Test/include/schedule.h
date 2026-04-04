#ifndef SCHEDULE_H
#define SCHEDULE_H

/*
 * ============================================================
 *  MODULE 8 - Event Timeline & Schedule Management
 * ============================================================
 */

#include <time.h>
#include "types.h"

typedef enum {
    EVT_CEREMONY = 0,
    EVT_RECEPTION,
    EVT_DINNER,
    EVT_SPEECH,
    EVT_ENTERTAINMENT,
    EVT_PHOTO,
    EVT_LOGISTICS,
    EVT_BREAK,
    EVT_TYPE_COUNT
} EventCategory;

typedef enum {
    EVT_SCHEDULED = 0,
    EVT_IN_PROGRESS,
    EVT_COMPLETED,
    EVT_DELAYED,
    EVT_CANCELLED
} EventStatus;

typedef enum {
    PRIO_CRITICAL = 0,
    PRIO_HIGH,
    PRIO_NORMAL,
    PRIO_OPTIONAL
} EventPriority;

typedef struct EventNode {
    int            event_id;
    char           title[100];
    char           description[250];
    EventCategory  category;
    EventStatus    status;
    EventPriority  priority;

    time_t         scheduled_start;
    time_t         scheduled_end;
    time_t         actual_start;
    time_t         actual_end;
    int            duration_minutes;
    int            delay_minutes;

    char           responsible[100];
    char           location[100];
    char           notes[200];

    struct EventNode *prev;
    struct EventNode *next;
} EventNode;

typedef struct {
    EventNode *head;
    EventNode *tail;
    int        event_count;
    char       wedding_date[20];
    char       venue[100];
} WeddingSchedule;

void schedule_init(WeddingSchedule *sched, const char *date, const char *venue);

EventNode* schedule_add_event(WeddingSchedule *sched,
                              const char *title,
                              const char *description,
                              EventCategory category,
                              EventPriority priority,
                              time_t start,
                              int duration_mins,
                              const char *responsible,
                              const char *location);

int schedule_remove_event(WeddingSchedule *sched, int event_id);

int schedule_start_event(WeddingSchedule *sched, int event_id);
int schedule_end_event(WeddingSchedule *sched, int event_id);
int schedule_delay_event(WeddingSchedule *sched, int event_id, int delay_minutes);
int schedule_cancel_event(WeddingSchedule *sched, int event_id);

EventNode* schedule_find_by_id(WeddingSchedule *sched, int id);
EventNode* schedule_current_event(WeddingSchedule *sched);
EventNode* schedule_next_event(WeddingSchedule *sched);
long       schedule_minutes_to_next(WeddingSchedule *sched);

void schedule_save(WeddingSchedule *sched, const char *path);
void schedule_load(WeddingSchedule *sched, const char *path);
void schedule_free(WeddingSchedule *sched);

const char* schedule_category_name(EventCategory c);
const char* schedule_status_name(EventStatus s);
const char* schedule_priority_name(EventPriority p);
char*       schedule_format_hhmm(time_t t, char *buf, int len);

extern int schedule_event_id_counter;

#endif /* SCHEDULE_H */

