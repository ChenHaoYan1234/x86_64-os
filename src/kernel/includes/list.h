#pragma once

struct List
{
    struct List *prev;
    struct List *next;
};

static inline void list_init(struct List *list)
{
    list->prev = list;
    list->next = list;
}

static inline void list_add_to_behind(struct List *entry, struct List *new)
{
    new->next = entry->next;
    new->prev = entry;
    entry->next->prev = new;
    entry->next = new;
}

static inline void list_add_to_front(struct List *entry, struct List *new)
{
    new->next = entry;
    new->prev = entry->prev;
    entry->prev->next = new;
    entry->prev = new;
}

static inline void list_del(struct List *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

static inline bool list_empty(struct List *list)
{
    return (list->next == list && list->prev == list);
}

static inline struct List *list_prev(struct List *list)
{
    return list->prev;
}

static inline struct List *list_next(struct List *list)
{
    return list->next;
}
