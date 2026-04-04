#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "schedule.h"

int schedule_event_id_counter = 1;

static const char *CAT_NAMES[] = {
    "Ceremony","Reception","Dinner","Speech",
    "Entertainment","Photo","Logistics","Break"
};
static const char *STATUS_NAMES[] = {
    "Scheduled","In Progress","Completed","Delayed","Cancelled"
};
static const char *PRIO_NAMES[] = {
    "CRITICAL","HIGH","NORMAL","OPTIONAL"
};

const char* schedule_category_name(EventCategory c) {
    if (c < 0 || c >= EVT_TYPE_COUNT) return "Unknown";
    return CAT_NAMES[c];
}
const char* schedule_status_name(EventStatus s) {
    if (s < 0 || s > EVT_CANCELLED) return "Unknown";
    return STATUS_NAMES[s];
}
const char* schedule_priority_name(EventPriority p) {
    if (p < 0 || p > PRIO_OPTIONAL) return "Unknown";
    return PRIO_NAMES[p];
}

char* schedule_format_hhmm(time_t t, char *buf, int len) {
    if (!buf || len <= 0) return buf;
    if (t == 0) {
        strncpy(buf, "--:--", (size_t)len);
        buf[len - 1] = '\0';
        return buf;
    }
    struct tm *tm_info = localtime(&t);
    if (!tm_info) {
        strncpy(buf, "??:??", (size_t)len);
        buf[len - 1] = '\0';
        return buf;
    }
    strftime(buf, (size_t)len, "%H:%M", tm_info);
    return buf;
}

static void safe_copy(char *dst, size_t dst_sz, const char *src) {
    if (!dst || dst_sz == 0) return;
    if (!src) src = "";
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

void schedule_init(WeddingSchedule *sched, const char *date, const char *venue) {
    if (!sched) return;
    sched->head = NULL;
    sched->tail = NULL;
    sched->event_count = 0;
    safe_copy(sched->wedding_date, sizeof(sched->wedding_date), date ? date : "");
    safe_copy(sched->venue, sizeof(sched->venue), venue ? venue : "");
}

static void insert_sorted(WeddingSchedule *sched, EventNode *e) {
    if (!sched->head) {
        sched->head = sched->tail = e;
        return;
    }

    if (difftime(e->scheduled_start, sched->head->scheduled_start) <= 0) {
        e->next = sched->head;
        sched->head->prev = e;
        sched->head = e;
        return;
    }

    if (difftime(e->scheduled_start, sched->tail->scheduled_start) >= 0) {
        e->prev = sched->tail;
        sched->tail->next = e;
        sched->tail = e;
        return;
    }

    EventNode *cur = sched->head;
    while (cur->next &&
           difftime(e->scheduled_start, cur->next->scheduled_start) > 0) {
        cur = cur->next;
    }

    e->next = cur->next;
    e->prev = cur;
    if (cur->next) cur->next->prev = e;
    cur->next = e;
}

EventNode* schedule_add_event(WeddingSchedule *sched,
                              const char *title,
                              const char *description,
                              EventCategory category,
                              EventPriority priority,
                              time_t start,
                              int duration_mins,
                              const char *responsible,
                              const char *location) {
    if (!sched) return NULL;
    if (duration_mins < 1) duration_mins = 1;

    EventNode *e = (EventNode*)malloc(sizeof(EventNode));
    if (!e) return NULL;
    memset(e, 0, sizeof(*e));

    e->event_id = schedule_event_id_counter++;
    safe_copy(e->title, sizeof(e->title), title ? title : "Event");
    safe_copy(e->description, sizeof(e->description), description ? description : "");
    e->category = category;
    e->status = EVT_SCHEDULED;
    e->priority = priority;
    e->scheduled_start = start;
    e->scheduled_end = start + (time_t)duration_mins * 60;
    e->actual_start = 0;
    e->actual_end = 0;
    e->duration_minutes = duration_mins;
    e->delay_minutes = 0;
    safe_copy(e->responsible, sizeof(e->responsible), responsible ? responsible : "");
    safe_copy(e->location, sizeof(e->location), location ? location : "");
    e->notes[0] = '\0';
    e->prev = e->next = NULL;

    insert_sorted(sched, e);
    sched->event_count++;
    return e;
}

EventNode* schedule_find_by_id(WeddingSchedule *sched, int id) {
    if (!sched) return NULL;
    EventNode *cur = sched->head;
    while (cur) {
        if (cur->event_id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

int schedule_remove_event(WeddingSchedule *sched, int event_id) {
    if (!sched) return 0;
    EventNode *e = schedule_find_by_id(sched, event_id);
    if (!e) return 0;

    if (e->prev) e->prev->next = e->next;
    else sched->head = e->next;

    if (e->next) e->next->prev = e->prev;
    else sched->tail = e->prev;

    free(e);
    sched->event_count--;
    return 1;
}

int schedule_start_event(WeddingSchedule *sched, int event_id) {
    EventNode *e = schedule_find_by_id(sched, event_id);
    if (!e) return 0;
    if (e->status == EVT_CANCELLED || e->status == EVT_COMPLETED) return 0;

    e->actual_start = time(NULL);
    double delay_secs = difftime(e->actual_start, e->scheduled_start);
    e->delay_minutes = (delay_secs > 0) ? (int)(delay_secs / 60.0) : 0;
    e->status = EVT_IN_PROGRESS;
    return 1;
}

int schedule_end_event(WeddingSchedule *sched, int event_id) {
    EventNode *e = schedule_find_by_id(sched, event_id);
    if (!e) return 0;
    if (e->status != EVT_IN_PROGRESS) return 0;

    e->actual_end = time(NULL);
    e->status = EVT_COMPLETED;
    return 1;
}

int schedule_delay_event(WeddingSchedule *sched, int event_id, int delay_minutes) {
    EventNode *e = schedule_find_by_id(sched, event_id);
    if (!e) return 0;
    if (delay_minutes == 0) return 1;

    e->delay_minutes += delay_minutes;
    e->scheduled_start += (time_t)delay_minutes * 60;
    e->scheduled_end += (time_t)delay_minutes * 60;
    if (e->status == EVT_SCHEDULED) e->status = EVT_DELAYED;
    return 1;
}

int schedule_cancel_event(WeddingSchedule *sched, int event_id) {
    EventNode *e = schedule_find_by_id(sched, event_id);
    if (!e) return 0;
    e->status = EVT_CANCELLED;
    return 1;
}

EventNode* schedule_current_event(WeddingSchedule *sched) {
    if (!sched) return NULL;
    EventNode *cur = sched->head;
    while (cur) {
        if (cur->status == EVT_IN_PROGRESS) return cur;
        cur = cur->next;
    }
    return NULL;
}

EventNode* schedule_next_event(WeddingSchedule *sched) {
    if (!sched) return NULL;
    EventNode *cur = sched->head;
    while (cur) {
        if (cur->status == EVT_SCHEDULED || cur->status == EVT_DELAYED) return cur;
        cur = cur->next;
    }
    return NULL;
}

long schedule_minutes_to_next(WeddingSchedule *sched) {
    EventNode *n = schedule_next_event(sched);
    if (!n) return -1;
    double secs = difftime(n->scheduled_start, time(NULL));
    return (long)(secs / 60.0);
}

typedef struct {
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
} EventSnapshot;

void schedule_save(WeddingSchedule *sched, const char *path) {
    if (!sched || !path) return;
    FILE *f = fopen(path, "wb");
    if (!f) return;

    fwrite(&sched->event_count, sizeof(int), 1, f);
    fwrite(sched->wedding_date, sizeof(sched->wedding_date), 1, f);
    fwrite(sched->venue, sizeof(sched->venue), 1, f);

    EventNode *cur = sched->head;
    while (cur) {
        EventSnapshot s;
        memset(&s, 0, sizeof(s));
        s.event_id = cur->event_id;
        safe_copy(s.title, sizeof(s.title), cur->title);
        safe_copy(s.description, sizeof(s.description), cur->description);
        s.category = cur->category;
        s.status = cur->status;
        s.priority = cur->priority;
        s.scheduled_start = cur->scheduled_start;
        s.scheduled_end = cur->scheduled_end;
        s.actual_start = cur->actual_start;
        s.actual_end = cur->actual_end;
        s.duration_minutes = cur->duration_minutes;
        s.delay_minutes = cur->delay_minutes;
        safe_copy(s.responsible, sizeof(s.responsible), cur->responsible);
        safe_copy(s.location, sizeof(s.location), cur->location);
        safe_copy(s.notes, sizeof(s.notes), cur->notes);

        fwrite(&s, sizeof(s), 1, f);
        cur = cur->next;
    }
    fclose(f);
}

void schedule_free(WeddingSchedule *sched) {
    if (!sched) return;
    EventNode *cur = sched->head;
    while (cur) {
        EventNode *next = cur->next;
        free(cur);
        cur = next;
    }
    sched->head = sched->tail = NULL;
    sched->event_count = 0;
}

void schedule_load(WeddingSchedule *sched, const char *path) {
    if (!sched || !path) return;
    FILE *f = fopen(path, "rb");
    if (!f) return;

    schedule_free(sched);

    int count = 0;
    if (fread(&count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return;
    }
    fread(sched->wedding_date, sizeof(sched->wedding_date), 1, f);
    fread(sched->venue, sizeof(sched->venue), 1, f);

    int max_id = 0;
    for (int i = 0; i < count; i++) {
        EventSnapshot s;
        if (fread(&s, sizeof(s), 1, f) != 1) break;

        EventNode *e = (EventNode*)malloc(sizeof(EventNode));
        if (!e) break;
        memset(e, 0, sizeof(*e));

        e->event_id = s.event_id;
        safe_copy(e->title, sizeof(e->title), s.title);
        safe_copy(e->description, sizeof(e->description), s.description);
        e->category = s.category;
        e->status = s.status;
        e->priority = s.priority;
        e->scheduled_start = s.scheduled_start;
        e->scheduled_end = s.scheduled_end;
        e->actual_start = s.actual_start;
        e->actual_end = s.actual_end;
        e->duration_minutes = s.duration_minutes;
        e->delay_minutes = s.delay_minutes;
        safe_copy(e->responsible, sizeof(e->responsible), s.responsible);
        safe_copy(e->location, sizeof(e->location), s.location);
        safe_copy(e->notes, sizeof(e->notes), s.notes);
        e->prev = e->next = NULL;

        /* append in file order */
        if (!sched->head) {
            sched->head = sched->tail = e;
        } else {
            e->prev = sched->tail;
            sched->tail->next = e;
            sched->tail = e;
        }
        sched->event_count++;
        if (e->event_id > max_id) max_id = e->event_id;
    }

    fclose(f);
    if (max_id >= schedule_event_id_counter) schedule_event_id_counter = max_id + 1;
}

