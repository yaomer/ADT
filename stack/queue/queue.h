#ifndef _QUEUE_H
#define _QUEUE_H

#include "../stack_array.h"

typedef struct {
    Stack *push;
    Stack *pop;
} Queue;

Queue   *queue_init(void);
void    queue_push(Queue *, int);
int     queue_pop(Queue *);
void    queue_clear(Queue **);

#endif  /* _QUEUE_H */