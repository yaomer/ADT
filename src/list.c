#include <stdbool.h>

#include "alloc.h"

struct list_node {
    void *data;
    struct list_node *prev;
    struct list_node *next;
};

typedef int (*__list_comp_handler)(const void *, const void *);
typedef void (*__list_free_handler)(void *);

typedef struct __list {
    size_t size;
    struct list_node *front;
    struct list_node *back;
    __list_comp_handler list_comp;
    __list_free_handler list_free;
} list_t;

struct __list_iterator {
    struct list_node *node;
};

typedef struct __list_iterator * list_iterator;

#define __check_list(list) (assert(list))

static void __list_free_node(list_t *list, struct list_node *node)
{
    if (list->list_free)
        list->list_free(node->data);
    free(node);
    list->size--;
}

static struct list_node *__alloc_node(void *data)
{
    struct list_node *node = Calloc(1, sizeof(struct list_node));
    node->data = data;
    return node;
}

static list_iterator __alloc_iterator(struct list_node *node)
{
    list_iterator iter = Malloc(sizeof(*iter));
    iter->node = node;
    return iter;
}

static struct list_node *__list_find(list_t *list, const void *data)
{
    struct list_node *iter = list->front;
    for ( ; iter; iter = iter->next) {
        if (list->list_comp(iter->data, data) == 0)
            return iter;
    }
    return NULL;
}

static void __list_insert_front(list_t *list, struct list_node *node)
{
    if (list->front) {
        node->next = list->front;
        list->front->prev = node;
    } else {
        list->back = node;
    }
    list->front = node;
    list->size++;
}

static void __list_insert_back(list_t *list, struct list_node *node)
{
    if (list->back) {
        list->back->next = node;
        node->prev = list->back;
    } else {
        list->front = node;
    }
    list->back = node;
    list->size++;
}

static void __list_insert_before(list_t *list,
        struct list_node *pos, struct list_node *node)
{
    node->prev = pos->prev;
    if (pos->prev)
        pos->prev->next = node;
    pos->prev = node;
    node->next = pos;
    if (!node->prev)
        list->front = node;
    list->size++;
}

static void __list_insert(list_t *list, struct list_node *node)
{
    struct list_node *iter = list->front;
    for ( ; iter; iter = iter->next) {
        if (list->list_comp(node->data, iter->data) < 0) {
            __list_insert_before(list, iter, node);
            return;
        }
    }
    __list_insert_back(list, node);
}

static void __list_pop_front(list_t *list)
{
    if (list->front) {
        struct list_node *node = list->front;
        list->front = list->front->next;
        if (list->front)
            list->front->prev = NULL;
        else
            list->back = NULL;
        __list_free_node(list, node);
    }
}

static void __list_pop_back(list_t *list)
{
    if (list->back) {
        struct list_node *node = list->back;
        list->back = list->back->prev;
        if (list->back)
            list->back->next = NULL;
        else
            list->front = NULL;
        __list_free_node(list, node);
    }
}

static void __list_erase(list_t *list, struct list_node *node)
{
    if (node->prev && node->next) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    } else if (node->prev) {
        node->prev->next = NULL;
        list->back = node->prev;
    } else if (node->next) {
        node->next->prev = NULL;
        list->front = node->next;
    } else {
        list->front = list->back = NULL;
    }
    __list_free_node(list, node);
}

list_t *list_init(__list_comp_handler lcomp, __list_free_handler lfree)
{
    list_t *list = Calloc(1, sizeof(list_t));
    list->list_comp = lcomp;
    list->list_free = lfree;
    return list;
}

bool list_empty(list_t *list)
{
    __check_list(list);
    return list->size == 0;
}

size_t list_size(list_t *list)
{
    __check_list(list);
    return list->size;
}

void *list_get(list_iterator iter)
{
    return iter->node->data;
}

void *list_front(list_t *list)
{
    __check_list(list);
    return list->front->data;
}

void *list_back(list_t *list)
{
    __check_list(list);
    return list->back->data;
}

list_iterator list_begin(list_t *list)
{
    __check_list(list);
    return __alloc_iterator(list->front);
}

list_iterator list_end(list_t *list)
{
    __check_list(list);
    return __alloc_iterator(list->back);
}

bool list_next(list_iterator iter)
{
    if (iter->node) iter->node = iter->node->next;
    return iter->node != NULL;
}

bool list_prev(list_iterator iter)
{
    if (iter->node) iter->node = iter->node->prev;
    return iter->node != NULL;
}

void list_free_iterator(list_iterator iter)
{
    free(iter);
}

void list_push_front(list_t *list, void *data)
{
    __check_list(list);
    __list_insert_front(list, __alloc_node(data));
}

void list_pop_front(list_t *list)
{
    __check_list(list);
    __list_pop_front(list);
}

void list_push_back(list_t *list, void *data)
{
    __check_list(list);
    __list_insert_back(list, __alloc_node(data));
}

void list_pop_back(list_t *list)
{
    __check_list(list);
    __list_pop_back(list);
}

void list_insert_before(list_t *list, list_iterator pos, void *data)
{
    __check_list(list);
    if (!pos || !pos->node) return;
    __list_insert_before(list, pos->node, __alloc_node(data));
}

list_iterator list_find(list_t *list, const void *data)
{
    __check_list(list);
    struct list_node *node = __list_find(list, data);
    return node ? __alloc_iterator(node) : NULL;
}

void list_insert(list_t *list, void *data)
{
    __check_list(list);
    __list_insert(list, __alloc_node(data));
}

void list_erase(list_t *list, const void *data)
{
    __check_list(list);
    struct list_node *node = __list_find(list, data);
    if (node) __list_erase(list, node);
}

list_t *list_merge(list_t *list1, list_t *list2)
{
    __check_list(list1);
    __check_list(list2);
    list_t *list = list_init(list1->list_comp, list1->list_free);
    struct list_node *iter1 = list1->front;
    struct list_node *iter2 = list2->front;
    struct list_node *_iter1 = iter1, *_iter2 = iter2;
    while (iter1 && iter2) {
        if (list->list_comp(iter1->data, iter2->data) < 0) {
            _iter1 = iter1->next;
            __list_insert_back(list, iter1);
            list1->size--;
        } else {
            _iter2 = iter2->next;
            __list_insert_back(list, iter2);
            list2->size--;
        }
        iter1 = _iter1;
        iter2 = _iter2;
    }
    if (iter1) {
        list->back->next = iter1;
        list->back = list1->back;
        list->size += list1->size;
    } else if (iter2) {
        list->back->next = iter2;
        list->back = list2->back;
        list->size += list2->size;
    }
    free(list1);
    free(list2);
    return list;
}

void list_reverse(list_t *list)
{
    __check_list(list);
    struct list_node *iter = list->front;
    struct list_node *_iter;
    list->front = list->back = NULL;
    while (iter) {
        _iter = iter->next;
        iter->next = iter->prev = NULL;
        __list_insert_front(list, iter);
        iter = _iter;
    }
}

void list_unique(list_t *list)
{
    __check_list(list);
    struct list_node *iter = list->front;
    while (iter && iter->next) {
        if (list->list_comp(iter->data, iter->next->data) == 0) {
            __list_erase(list, iter->next);
        } else
            iter = iter->next;
    }
}

void list_clear(list_t *list)
{
    __check_list(list);
    while (list->front)
        list_pop_front(list);
}

void list_free(list_t *list)
{
    list_clear(list);
    free(list);
}
