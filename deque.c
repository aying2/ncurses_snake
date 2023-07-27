#include <stdlib.h>
#include <stdio.h>
#include "deque.h"

bool
pose_equal(Pose a, Pose b)
{
    return a.x == b.x && a.y == b.y;
}

Node *
node_new(Pose pos)
{
    Node *n = malloc(sizeof *n);
    n->data = pos;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

void
node_destroy(Node *n)
{
    free(n);
}

void
node_print(Node const *n)
{
    printf("(%d, %d)", n->data.x, n->data.y);
}

Deque *
deque_new(void)
{
    Deque *deq = malloc(sizeof *deq);
    deq->head = malloc(sizeof *deq->head);
    deq->tail = malloc(sizeof *deq->tail);

    deq->head->next = deq->tail;
    deq->head->prev = NULL;
    deq->head->data = (Pose) {-1, -1};

    deq->tail->next = NULL;
    deq->tail->prev = deq->head;
    deq->tail->data = (Pose) {-1, -1};

    deq->length = 0;
    return deq;
}

void
deque_destroy(Deque *deq)
{
    Node *cur = deq->head;
    while (cur)
    {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }

    free(deq);
}

void
deque_print(Deque const *deq)
{
    Node *cur = deq->head;
    printf("forward:\n");
    while (cur->next)
    {
        node_print(cur);
        printf("->");
        cur = cur->next;
    }
    node_print(cur);
    printf("\n");

    printf("backward:\n");
    while (cur->prev)
    {
        node_print(cur);
        printf("->");
        cur = cur->prev;
    }
    node_print(cur);
    printf("%d", deq->length);
    printf("\n");
}

Node *
deque_push_front(Deque *deq, Node *n)
{
    deq->head->next->prev = n;
    n->next = deq->head->next;
    n->prev = deq->head;
    deq->head->next = n;
    deq->length++;
    return n;
}

Node *
deque_push_back(Deque *deq, Node *n)
{
    deq->tail->prev->next = n;
    n->prev = deq->tail->prev;
    n->next = deq->tail;
    deq->tail->prev = n;
    deq->length++;
    return n;
}

Node *
deque_pop_front_r(Deque *deq)
{
    Node *to_remove = deq->head->next;
    if (to_remove == deq->tail)
    {
        return NULL;
    }
    deq->head->next = to_remove->next;
    to_remove->next->prev = deq->head;

    to_remove->next = NULL;
    to_remove->prev = NULL;

    deq->length--;

    return to_remove;
}

void
deque_pop_front(Deque *deq)
{
    node_destroy(deque_pop_front_r(deq));
}


Node *
deque_pop_back_r(Deque *deq)
{
    Node *to_remove = deq->tail->prev;
    if (to_remove == deq->head)
    {
        return NULL;
    }
    deq->tail->prev = to_remove->prev;
    to_remove->prev->next = deq->tail;

    to_remove->next = NULL;
    to_remove->prev = NULL;

    deq->length--;

    return to_remove;
}

void
deque_pop_back(Deque *deq)
{
    node_destroy(deque_pop_back_r(deq));
}

void
deque_clear(Deque *deq)
{
    Node *n = NULL;
    while ((n = deque_pop_front_r(deq)) != NULL)
    {
        node_destroy(n);
    }
    deq->length = 0;
}

bool
deque_contains(Deque const *deq, Pose pos)
{
    Node *cur = deq->head->next;
    while (cur != deq->tail)
    {
        if (pose_equal(cur->data, pos))
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

Node *
deque_get_head(Deque const *deq)
{
    if (deq->head->next == deq->tail)
    {
        return NULL;
    }
    return deq->head->next;
}

Node *
deque_get_tail(Deque const *deq)
{
    if (deq->tail->prev == deq->head)
    {
        return NULL;
    }
    return deq->tail->prev;
}
