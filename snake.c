#include <ncurses/curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COLOR_SNAKE COLOR_GREEN
#define PAIR_SNAKE 1
#define COLOR_FOOD COLOR_RED
#define PAIR_FOOD 2
#define COLOR_BORDER COLOR_WHITE
#define PAIR_BORDER 3

#define INIT_DELAY_MS 100

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

enum DIRECTION {
    DIRECTION_null,
    DIRECTION_left,
    DIRECTION_right,
    DIRECTION_up,
    DIRECTION_down,
};

enum STATE {
    STATE_null,
    STATE_lose,
    STATE_win,
    STATE_active,
};

typedef struct Snake {
    int nlines;
    int ncols;
    enum DIRECTION dir;
    enum STATE state;
    bool flipped;
    Deque *deq;
    Pose food_pos;
} Snake;

Pose
snake_find_food_pos(Snake *snake)
{
    bool **taken = malloc(snake->nlines * sizeof *taken);
    for (int i = 0; i < snake->nlines; i++)
    {
        taken[i] = calloc(1, snake->ncols * sizeof *taken[i]);
    }

    Node *cur = snake->deq->head->next;
    while (cur != snake->deq->tail)
    {
        int y = cur->data.y;
        int x = cur->data.x;
        taken[y][x] = true;

        cur = cur->next;
    }

    int nth_empty = rand() % (snake->nlines * snake->ncols - snake->deq->length);

    int num_empty = 0;
    Pose pos = {-1, -1};
    for (int i = 0; i < snake->nlines; i++)
    {
        for (int j = 0; j < snake->ncols; j++)
        {
            if (taken[i][j] == false)
            {
                if (num_empty == nth_empty)
                {
                    pos = (Pose) {.y = i, .x = j};
                    goto cleanup;
                }
                num_empty++;
            }
        }
    }

    cleanup:

    for (int i = 0; i < snake->nlines; i++)
    {
        free(taken[i]);
    }
    free(taken);

    if (pose_equal(pos, (Pose) {-1, -1}))
    {
        fprintf(stderr, "invalid snake_find_food_pos");
        exit(1);
    }

    return pos;
}

Snake *
snake_new(int nlines, int ncols)
{
    Snake *snake = malloc(sizeof *snake);
    snake->nlines = nlines;
    snake->ncols = ncols;
    snake->deq = deque_new();

    snake->dir = DIRECTION_null;
    snake->state = STATE_null;
    snake->flipped = false;

    Node *first_node = node_new( (Pose) {.y = nlines / 2, .x = ncols / 2});
    deque_push_back(snake->deq, first_node);

    snake->food_pos = snake_find_food_pos(snake);
    
    return snake;
}

void
snake_destroy(Snake *snake)
{
    deque_destroy(snake->deq);
    free(snake);
}


void
snake_set_direction(Snake *snake, enum DIRECTION dir)
{

    switch (snake->dir)
    {
        case DIRECTION_left:
            if (dir != DIRECTION_right)
            {
                snake->dir = dir;
            }
            break;
        case DIRECTION_right:
            if (dir != DIRECTION_left)
            {
                snake->dir = dir;
            }
            break;
        case DIRECTION_up:
            if (dir != DIRECTION_down)
            {
                snake->dir = dir;
            }
            break;
        case DIRECTION_down:
            if (dir != DIRECTION_up)
            {
                snake->dir = dir;
            }
            break;
        default:
            snake->dir = dir;
            break;
    }
    snake->state = STATE_active;
}

// TODO flipping should flip direction last bit is headed not
// this causes internal crashes
void
snake_flip(Snake *snake)
{
    snake->flipped = !snake->flipped;
}

bool
snake_pos_out_of_bounds(Snake *snake, Pose pos)
{
    return  (pos.y < 0 || pos.y > snake->nlines - 1
            || pos.x < 0 || pos.x > snake->ncols - 1);
}

bool
snake_contains_pos(Snake *snake, Pose pos)
{
    Node *cur = snake->deq->head->next;
    while (cur != snake->deq->tail)
    {
        if (pose_equal(cur->data, pos))
        {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void
snake_update(Snake *snake)
{
    if (snake->state != STATE_active)
    {
        fprintf(stderr, "not active\n");
        return;
    }

    Pose next_pos = deque_get_head(snake->deq)->data;
    if (snake->flipped == true)
    {
        next_pos = deque_get_tail(snake->deq)->data;
    }

    switch (snake->dir)
    {
        case DIRECTION_left:
            next_pos.x--;
            break;
        case DIRECTION_right:
            next_pos.x++;
            break;
        case DIRECTION_up:
            next_pos.y--;
            break;
        case DIRECTION_down:
            next_pos.y++;
            break;
        default:
            fprintf(stderr, "null direction\n");
            return;
    }
    
    if (snake_pos_out_of_bounds(snake, next_pos)
            || snake_contains_pos(snake, next_pos))
    {
        snake->state = STATE_lose;
        return;
    }

    Node *n = node_new(next_pos);
    if (snake->flipped == false)
    {
        deque_push_front(snake->deq, n);
    }
    else
    {
        deque_push_back(snake->deq, n);
    }

    if (pose_equal(next_pos, snake->food_pos) == true)
    {
        snake->food_pos = snake_find_food_pos(snake);
    }
    else
    {
        if (snake->flipped == false)
        {
            deque_pop_back(snake->deq);
        }
        else
        {
            deque_pop_front(snake->deq);
        }
    }

    if (snake->deq->length == snake->nlines * snake->ncols) {
        snake->state = STATE_win;
    }
}

// TODO add a variable which keeps track of speed as double (x2, etc)
// TODO have you win or you lose pop up as a notification with score and high score 
// have those on the right side too
typedef struct SnakeView {
    WINDOW *win;
    WINDOW *border;
    int delay_ms;
} SnakeView;

SnakeView *
snakeview_new(int nlines, int ncols, int begin_y, int begin_x)
{
    SnakeView *view = malloc(sizeof *view);
    view->border = newwin(nlines + 2, ncols + 2, begin_y, begin_x);
    view->win = derwin(view->border, nlines, ncols, 1, 1);
    view->delay_ms = INIT_DELAY_MS;

    wattron(view->border, COLOR_PAIR(PAIR_BORDER));
    box(view->border, 0, 0);
    wattroff(view->border, COLOR_PAIR(PAIR_BORDER));

    wnoutrefresh(view->border);

    return view;
}

void
snakeview_destroy(SnakeView *view)
{
    delwin(view->win);
    delwin(view->border);
    free(view);
}

void
snakeview_redraw(SnakeView *view, Snake *model)
{
    wclear(view->win);

    wattron(view->win, COLOR_PAIR(PAIR_SNAKE));
    Node *cur = model->deq->head->next;
    while (cur != model->deq->tail)
    {
        mvwaddch(view->win, cur->data.y, cur->data.x, ACS_BLOCK);
        cur = cur->next;
    }
    wattroff(view->win, COLOR_PAIR(PAIR_SNAKE));

    wattron(view->win, COLOR_PAIR(PAIR_FOOD));
    mvwaddch(view->win, model->food_pos.y, model->food_pos.x, ACS_BLOCK);
    attroff(COLOR_PAIR(PAIR_FOOD));

    wnoutrefresh(view->win);
    doupdate();
}

typedef struct SnakeController {
    Snake *model;
    SnakeView *view;
} SnakeController;

// TODO smartedly handle the size. Maybe instead the user just directly
// passes the lines and cols. because we want space on the right side for info
SnakeController *
snakecontroller_new(int nlines, int ncols, int begin_y, int begin_x)
{
    SnakeController *controller = malloc(sizeof *controller);
    controller->model = snake_new(nlines, ncols);
    controller->view = snakeview_new(nlines, ncols, begin_y, begin_x);

    return controller;
}

void
snakecontroller_destroy(SnakeController *controller)
{
    snake_destroy(controller->model);
    snakeview_destroy(controller->view);
    free(controller);
}

void
snakecontroller_loop(SnakeController *controller)
{
    snakeview_redraw(controller->view, controller->model);
    int ch;
    timeout(0);
    while ((ch = getch()) != KEY_F(1))
    {
        switch (ch)
        {
            case KEY_LEFT:
                snake_set_direction(controller->model, DIRECTION_left);
                break;
            case KEY_RIGHT:
                snake_set_direction(controller->model, DIRECTION_right);
                break;
            case KEY_UP:
                snake_set_direction(controller->model, DIRECTION_up);
                break;
            case KEY_DOWN:
                snake_set_direction(controller->model, DIRECTION_down);
                break;
            case ' ':
                snake_flip(controller->model);
                break;
            case 'f':
                controller->view->delay_ms /= 1.5;
                break;
            case 's':
                controller->view->delay_ms *= 1.5;
                break;
            default:
                break;
        }
        if (controller->model->state == STATE_active)
        {
            snake_update(controller->model);
            snakeview_redraw(controller->view, controller->model);
        }

        if (controller->model->state == STATE_win)
        {
            mvprintw(14, 15, "YOU WIN!");
        }
        else if (controller->model->state == STATE_lose)
        {
            mvprintw(14, 15, "YOU LOSE!");
        }
        napms(controller->view->delay_ms);
    }
}

int main()
{
    time_t t;
    srand(time(&t));

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

    use_default_colors();
    start_color();
    init_pair(PAIR_SNAKE, COLOR_SNAKE, -1);
    init_pair(PAIR_FOOD, COLOR_FOOD, -1);
    init_pair(PAIR_BORDER, COLOR_BORDER, -1);

    refresh();

    SnakeController *controller = snakecontroller_new(LINES - 2, COLS - 2, 0, 0);
    snakecontroller_loop(controller);
    snakecontroller_destroy(controller);

    endwin();

    return EXIT_SUCCESS;
}
