#ifndef INCLUDE_LIST_H
#define INCLUDE_LIST_H

#include <stddef.h>
#include "container_of.h"

/* Very similar to the Linux-kernel list.h header (GPLv2) */

struct list_node_struct {
    struct list_node_struct *next, *prev;
};

typedef struct list_node_struct list_node_t;
typedef struct list_node_struct list_head_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    list_head_t name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(list_head_t *list)
{
    list->next = list;
    list->prev = list;
}

/* If both are NULL, then this node isn't currently in a list */
static inline int list_node_is_in_list(list_node_t *node)
{
    return node->next || node->prev;
}

static inline void __list_add(list_node_t *new,
                              list_node_t *prev,
                              list_node_t *next)
{
    next->prev = new;
    new->next = next;

    new->prev = prev;
    prev->next = new;
}

static inline void list_add(list_head_t *head, list_node_t *new)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(list_head_t *head, list_node_t *new)
{
    __list_add(new, head->prev, head);
}

static inline void __list_del(list_node_t *prev, list_node_t *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(list_node_t *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(list_node_t *entry)
{
    __list_del_entry(entry);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline void list_del_checked(list_node_t *entry)
{
    if (entry->next && entry->prev)
        list_del(entry);
}

static inline void list_replace(list_node_t *new, list_node_t *old)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

static inline void list_move(list_head_t *head, list_node_t *entry)
{
    __list_del_entry(entry);
    list_add(head, entry);
}

static inline void list_move_tail(list_head_t *head, list_node_t *entry)
{
    __list_del_entry(entry);
    list_add_tail(head, entry);
}

static inline void list_move_tail_checked(list_head_t *head, list_node_t *entry)
{
    if (entry->next && entry->prev)
        __list_del_entry(entry);
    list_add_tail(head, entry);
}

static inline int list_is_last(const list_head_t *head, const list_node_t *entry)
{
    return entry->next == head;
}

static inline int list_empty(const list_head_t *head)
{
    return head == head->next;
}

static inline void list_rotate_left(list_head_t *head)
{
    if (!list_empty(head))
        list_move_tail(head, head->next);
}

static inline void list_rotate_right(list_head_t *head)
{
    if (!list_empty(head))
        list_move(head, head->prev);
}

/* Moves 'first', which is already in list 'head', to the position of the first
 * entry in 'head', by rotating the list. */
static inline void list_new_first(list_head_t *head, list_node_t *new_first)
{
    list_node_t *last = head->prev;
    list_node_t *first = head->next;
    list_node_t *new_last = new_first->prev;

    if (first == new_first)
        return ;

    /* Connect first and last list node together */
    last->next = first;
    first->prev = last;

    /* Make 'new_last' and 'new_first' the first and last nodes of the list */
    new_last->next = head;
    new_first->prev = head;

    head->prev = new_last;
    head->next = new_first;
}

static inline void list_new_last(list_head_t *head, list_node_t *new_last)
{
    list_node_t *last = head->prev;
    list_node_t *first = head->next;
    list_node_t *new_first = new_last->next;

    if (last == new_last)
        return ;

    last->next = first;
    first->prev = last;

    new_last->next = head;
    new_first->prev = head;

    head->prev = new_last;
    head->next = new_first;
}

static inline list_node_t *__list_first(list_head_t *head)
{
    return head->next;
}

#define list_first(head, type, member) \
    container_of(__list_first(head), type, member)

static inline list_node_t *__list_last(list_head_t *head)
{
    return head->prev;
}

#define list_last(head, type, member) \
    container_of(__list_last(head), type, member)

static inline list_node_t *__list_take_first(list_head_t *head)
{
    list_node_t *node = __list_first(head);
    list_del(node);
    return node;
}

#define list_take_first(head, type, member) \
    container_of(__list_take_first(head), type, member)

static inline list_node_t *__list_take_last(list_head_t *head)
{
    list_node_t *node = __list_last(head);
    list_del(node);
    return node;
}

#define list_take_last(head, type, member) \
    container_of(__list_take_last(head), type, member)


#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

#define list_first_entry_or_null(ptr, type, member) \
    (!list_empty(ptr)? list_first_entry(ptr, type, member): NULL)

#define list_next_entry(pos, type, member) \
    list_entry((pos)->member.next, type, member)

#define list_prev_entry(pos, type, member) \
    list_entry((pos)->member.prev, type, member)

#define list_foreach(head, pos) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_foreach_prev(head, pos) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_foreach_entry(head, pos, type, member) \
    for (pos = list_first_entry(head, type, member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, type, member))

#define list_foreach_entry_reverse(head, pos, member) \
    for (pos = list_last_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_prev_entry(pos, member))

#define list_foreach_take_entry(head, pos, member)                                  \
    for (pos = list_empty(head)? NULL: list_take_first(head, typeof(*pos), member); \
         pos;                                                                       \
         pos = list_empty(head)? NULL: list_take_first(head, typeof(*pos), member))

#define list_foreach_take_entry_reverse(head, pos, member) \
    for (pos = list_empty(head)? NULL: list_take_last(head, typeof(*pos), member); \
         &(pos)->member != NULL;                           \
         pos = list_empty(head)? NULL: list_take_last(head, typeof(*pos), member))

#define list_ptr_is_head(head, ptr) \
    ((ptr) == (head))

#endif
