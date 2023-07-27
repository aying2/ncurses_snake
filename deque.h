#ifndef DEQUE_H
#define DEQUE_H
#include <stdbool.h>

typedef struct Pose
{
    int y;
    int x;
}
Pose;

typedef struct Node
{
    Pose data;
    struct Node *next;
    struct Node *prev;
}
Node;

typedef struct Deque
{
    Node *head;
    Node *tail;
    int length;
}
Deque;

bool
pose_equal(Pose a, Pose b);

Node *
node_new(Pose pos);

void
node_destroy(Node *n);

void
node_print(Node const *n);

Deque *
deque_new(void);

void
deque_destroy(Deque *deq);

void
deque_print(Deque const *deq);

Node *
deque_push_front(Deque *deq, Node *n);

Node *
deque_push_back(Deque *deq, Node *n);

Node *
deque_pop_front_r(Deque *deq);

void
deque_pop_front(Deque *deq);

Node *
deque_pop_back_r(Deque *deq);

void
deque_pop_back(Deque *deq);

void
deque_clear(Deque *deq);

bool
deque_contains(Deque const *deq, Pose pos);

Node *
deque_get_head(Deque const *deq);

Node *
deque_get_tail(Deque const *deq);
#endif // !DEQUE_H
