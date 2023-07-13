#include <ncurses/curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const int DELAY = 250;

typedef struct Pose
{
    int x;
    int y;
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
node_print(Node *n)
{
    printf("(%d, %d)", n->data.x, n->data.y);
}

// TODO need to malloc the head and tail
Deque *
deque_new()
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
deque_print(Deque *deq)
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
    printf("\n");
}

Node *
deque_push_front(Deque *deq, Node *n)
{
    deq->head->next->prev = n;
    n->next = deq->head->next;
    n->prev = deq->head;
    deq->head->next = n;
    return n;
}

Node *
deque_push_back(Deque *deq, Node *n)
{
    deq->tail->prev->next = n;
    n->prev = deq->tail->prev;
    n->next = deq->tail;
    deq->tail->prev = n;
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

    return to_remove;
}

void
deque_pop_back(Deque *deq)
{
    node_destroy(deque_pop_back_r(deq));
}

typedef struct Snake
{
    int nlines;
    int ncols;

}
Snake;



int main()
{
//    time_t t;
//    srand(time(&t));
//        
//    initscr();
//    cbreak();
//    keypad(stdscr, TRUE);
//    noecho();

    Deque *deq = deque_new();
    Node *n = node_new((Pose) {0, 0});
    deque_push_front(deq, n);
    n = node_new((Pose) {1, 0});
    deque_push_back(deq, n);
    n = node_new((Pose) {2, 0});
    deque_push_front(deq, n);
    deque_print(deq);
    deque_pop_front(deq);
    deque_print(deq);

    deque_destroy(deq);
//    endwin();

    return EXIT_SUCCESS;
}
