#include "deque.h"
#include "timer.h"
#include <math.h>
#include <ncurses/curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_SNAKE COLOR_GREEN
#define PAIR_SNAKE 1
#define COLOR_FOOD COLOR_RED
#define PAIR_FOOD 2
#define COLOR_BORDER COLOR_WHITE
#define PAIR_BORDER 3

#define INIT_DELAY_MS 100
#define DEFAULT_LENGTH 15

#define END_NLINES 6
#define END_NCOLS 19

#define SCORE_CONST_NCOLS 10
#define SPEED_NCOLS 8 + 4
#define CONTINUES_NCOLS 11 + 2
#define TIME_NCOLS 2 * 2 + 1

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

Pose snake_find_food_pos(Snake *snake) {
    bool **taken = malloc(snake->nlines * sizeof *taken);
    for (int i = 0; i < snake->nlines; i++) {
        taken[i] = calloc(1, snake->ncols * sizeof *taken[i]);
    }

    Node *cur = snake->deq->head->next;
    while (cur != snake->deq->tail) {
        int y = cur->data.y;
        int x = cur->data.x;
        taken[y][x] = true;

        cur = cur->next;
    }

    int nth_empty =
        rand() % (snake->nlines * snake->ncols - snake->deq->length);

    int num_empty = 0;
    Pose pos = {-1, -1};
    for (int i = 0; i < snake->nlines; i++) {
        for (int j = 0; j < snake->ncols; j++) {
            if (taken[i][j] == false) {
                if (num_empty == nth_empty) {
                    pos = (Pose){.y = i, .x = j};
                    goto cleanup;
                }
                num_empty++;
            }
        }
    }

cleanup:

    for (int i = 0; i < snake->nlines; i++) {
        free(taken[i]);
    }
    free(taken);

    if (pose_equal(pos, (Pose){-1, -1})) {
        fprintf(stderr, "invalid snake_find_food_pos");
        exit(1);
    }

    return pos;
}

Snake *snake_new(int nlines, int ncols) {
    Snake *snake = malloc(sizeof *snake);
    snake->nlines = nlines;
    snake->ncols = ncols;
    snake->deq = deque_new();

    snake->dir = DIRECTION_null;
    snake->state = STATE_null;
    snake->flipped = false;

    Node *first_node = node_new((Pose){.y = nlines / 2, .x = ncols / 2});
    deque_push_back(snake->deq, first_node);

    snake->food_pos = snake_find_food_pos(snake);

    return snake;
}

void snake_destroy(Snake *snake) {
    deque_destroy(snake->deq);
    free(snake);
}

void snake_set_direction(Snake *snake, enum DIRECTION dir) {

    switch (snake->dir) {
    case DIRECTION_left:
        if (dir != DIRECTION_right) {
            snake->dir = dir;
        }
        break;
    case DIRECTION_right:
        if (dir != DIRECTION_left) {
            snake->dir = dir;
        }
        break;
    case DIRECTION_up:
        if (dir != DIRECTION_down) {
            snake->dir = dir;
        }
        break;
    case DIRECTION_down:
        if (dir != DIRECTION_up) {
            snake->dir = dir;
        }
        break;
    default:
        snake->dir = dir;
        break;
    }
    snake->state = STATE_active;
}

void snake_flip(Snake *snake) {
    if (snake->dir == DIRECTION_null) {
        return;
    }

    if (snake->deq->length == 0) {
        fprintf(stderr, "snake has no nodes\n");
        return;
    }

    if (snake->deq->length == 1) {
        switch (snake->dir) {
        case DIRECTION_left:
            snake->dir = DIRECTION_right;
            break;
        case DIRECTION_right:
            snake->dir = DIRECTION_left;
            break;
        case DIRECTION_up:
            snake->dir = DIRECTION_down;
            break;
        case DIRECTION_down:
            snake->dir = DIRECTION_up;
            break;
        default:
            fprintf(stderr, "null direction\n");
            break;
        }
    } else {
        Pose last_pos = snake->deq->tail->prev->data;
        Pose second_last_pos = snake->deq->tail->prev->prev->data;
        if (snake->flipped == true) {
            last_pos = snake->deq->head->next->data;
            second_last_pos = snake->deq->head->next->next->data;
        }
        Pose displacement = {.y = second_last_pos.y - last_pos.y,
                             .x = second_last_pos.x - last_pos.x};

        if (pose_equal(displacement, (Pose){.y = 0, .x = -1})) {
            snake->dir = DIRECTION_right;
        } else if (pose_equal(displacement, (Pose){.y = 0, .x = 1})) {
            snake->dir = DIRECTION_left;
        } else if (pose_equal(displacement, (Pose){.y = -1, .x = 0})) {
            snake->dir = DIRECTION_down;
        } else if (pose_equal(displacement, (Pose){.y = 1, .x = 0})) {
            snake->dir = DIRECTION_up;
        }
    }
    snake->flipped = !snake->flipped;
    snake->state = STATE_active;
}

bool snake_pos_out_of_bounds(Snake *snake, Pose pos) {
    return (pos.y < 0 || pos.y > snake->nlines - 1 || pos.x < 0 ||
            pos.x > snake->ncols - 1);
}

bool snake_contains_pos(Snake *snake, Pose pos) {
    Node *cur = snake->deq->head->next;
    while (cur != snake->deq->tail) {
        if (pose_equal(cur->data, pos)) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void snake_update(Snake *snake) {
    if (snake->state != STATE_active) {
        fprintf(stderr, "not active\n");
        return;
    }

    Pose next_pos = deque_get_head(snake->deq)->data;
    if (snake->flipped == true) {
        next_pos = deque_get_tail(snake->deq)->data;
    }

    switch (snake->dir) {
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

    if (snake_pos_out_of_bounds(snake, next_pos) ||
        snake_contains_pos(snake, next_pos)) {
        snake->state = STATE_lose;
        return;
    }

    Node *n = node_new(next_pos);
    if (snake->flipped == false) {
        deque_push_front(snake->deq, n);
    } else {
        deque_push_back(snake->deq, n);
    }

    if (pose_equal(next_pos, snake->food_pos) == true) {
        if (snake->deq->length == snake->nlines * snake->ncols) {
            snake->state = STATE_win;
        } else {
            snake->food_pos = snake_find_food_pos(snake);
        }
    } else {
        if (snake->flipped == false) {
            deque_pop_back(snake->deq);
        } else {
            deque_pop_front(snake->deq);
        }
    }
}

typedef struct SnakeView {
    WINDOW *win;
    WINDOW *border;
} SnakeView;

SnakeView *snakeview_new(int nlines, int ncols, int begin_y, int begin_x) {

    SnakeView *view = malloc(sizeof *view);

    view->border = newwin(nlines + 2, ncols + 2, begin_y - 1, begin_x - 1);
    view->win = derwin(view->border, nlines, ncols, 1, 1);

    wattron(view->border, COLOR_PAIR(PAIR_BORDER));

    box(view->border, 0, 0);

    wattroff(view->border, COLOR_PAIR(PAIR_BORDER));

    wnoutrefresh(view->border);
    wnoutrefresh(view->win);
    doupdate();

    return view;
}

void snakeview_destroy(SnakeView *view) {
    delwin(view->win);
    delwin(view->border);

    free(view);
}

void mvwaddch_four(WINDOW *win, int y, int x, const chtype ch) {
    int y_tf = y * 2;
    int x_tf = x * 2;

    mvwaddch(win, y_tf, x_tf, ch);
    mvwaddch(win, y_tf, x_tf + 1, ch);
    mvwaddch(win, y_tf + 1, x_tf, ch);
    mvwaddch(win, y_tf + 1, x_tf + 1, ch);
}
void snakeview_redraw(SnakeView *view, Deque *deq, Pose food_pos) {
    wclear(view->win);

    wattron(view->win, COLOR_PAIR(PAIR_SNAKE));
    Node *cur = deq->head->next;
    while (cur != deq->tail) {
        mvwaddch_four(view->win, cur->data.y, cur->data.x, ACS_BLOCK);
        cur = cur->next;
    }
    wattroff(view->win, COLOR_PAIR(PAIR_SNAKE));

    // checking for win basically
    if (deque_contains(deq, food_pos) == false) {
        wattron(view->win, COLOR_PAIR(PAIR_FOOD));
        mvwaddch_four(view->win, food_pos.y, food_pos.x, ACS_BLOCK);
        wattroff(view->win, COLOR_PAIR(PAIR_FOOD));
    }

    wrefresh(view->win);
}

typedef struct InfoView {
    WINDOW *win;
    WINDOW *border;

    WINDOW *score_win;
    WINDOW *speed_win;
    WINDOW *continues_win;
    WINDOW *time_win;
} InfoView;

InfoView *infoview_new(int nlines, int score_ncols, int speed_ncols,
                       int continues_ncols, int time_ncols, int begin_y,
                       int begin_x) {
    InfoView *info = malloc(sizeof *info);

    int ncols =
        score_ncols + speed_ncols + continues_ncols + time_ncols + 3 * 3 + 2;

    info->border = newwin(nlines + 2, ncols + 2, begin_y - 1, begin_x - 1);
    info->win = derwin(info->border, nlines, ncols, 1, 1);

    int midsegidx[3] = {
        1 + 3 + score_ncols,
        1 + 2 * 3 + score_ncols + speed_ncols,
        1 + 3 * 3 + score_ncols + speed_ncols + continues_ncols,
    };
    info->score_win = derwin(info->win, nlines, score_ncols, 0, 1);
    info->speed_win = derwin(info->win, nlines, speed_ncols, 0, midsegidx[0]);
    info->continues_win =
        derwin(info->win, nlines, continues_ncols, 0, midsegidx[1]);
    info->time_win = derwin(info->win, nlines, time_ncols, 0, midsegidx[2]);

    wattron(info->border, COLOR_PAIR(PAIR_BORDER));

    box(info->border, 0, 0);

    for (int i = 0; i < (sizeof midsegidx / sizeof midsegidx[0]); i++) {
        mvwaddch(info->border, 0, midsegidx[i] - 1, ACS_TTEE);
        mvwaddch(info->border, 1, midsegidx[i] - 1, ACS_VLINE);
        mvwaddch(info->border, 2, midsegidx[i] - 1, ACS_BTEE);
    }

    wattroff(info->border, COLOR_PAIR(PAIR_BORDER));

    wnoutrefresh(info->border);
    wnoutrefresh(info->win);
    wnoutrefresh(info->score_win);
    wnoutrefresh(info->speed_win);
    wnoutrefresh(info->continues_win);
    wnoutrefresh(info->time_win);
    doupdate();

    return info;
}

void infoview_destroy(InfoView *info) {
    delwin(info->score_win);
    delwin(info->speed_win);
    delwin(info->continues_win);
    delwin(info->time_win);

    delwin(info->win);
    delwin(info->border);

    free(info);
}

void infoview_update_info(InfoView *info, int score, int max_score,
                          double speed, int continues, int time_sec) {
    int minutes = time_sec / 60;
    int secs = time_sec % 60;

    int ndigs = log10(max_score) + 1;
    mvwprintw(info->score_win, 0, 0, "Score: %*d / %d", ndigs, score,
              max_score);
    mvwprintw(info->speed_win, 0, 0, "Speed: x%0.2fd", speed);
    wclear(info->continues_win);
    mvwprintw(info->continues_win, 0, 0, "Continues: %d", continues);
    mvwprintw(info->time_win, 0, 0, "%02d:%02d", minutes, secs);

    wnoutrefresh(info->score_win);
    wnoutrefresh(info->speed_win);
    wnoutrefresh(info->continues_win);
    wnoutrefresh(info->time_win);
    doupdate();
}

typedef struct SnakeController {
    Snake *model;
    SnakeView *view;
    InfoView *info;
    int max_score;
    double delay_ms;
    int continues;
    Timer *timer;

    int high_score;
} SnakeController;

SnakeController *snakecontroller_new(int nlines, int ncols, int begin_y,
                                     int begin_x) {
    SnakeController *controller = malloc(sizeof *controller);
    controller->model = snake_new(nlines, ncols);
    controller->view = snakeview_new(nlines * 2, ncols * 2, begin_y, begin_x);

    controller->max_score =
        controller->model->nlines * controller->model->ncols;
    int score_ncols =
        (floor(log10(controller->max_score)) + 1) * 2 + SCORE_CONST_NCOLS;
    int info_ncols = score_ncols + SPEED_NCOLS + CONTINUES_NCOLS + TIME_NCOLS +
                     3 * 3 + 2 * 2;
    controller->info =
        infoview_new(1, score_ncols, SPEED_NCOLS, CONTINUES_NCOLS, TIME_NCOLS,
                     begin_y - 3, COLS - info_ncols);
    controller->delay_ms = INIT_DELAY_MS;
    controller->continues = 0;
    controller->timer = timer_new();

    controller->high_score = 0;

    return controller;
}

void snakecontroller_destroy(SnakeController *controller) {
    snake_destroy(controller->model);
    snakeview_destroy(controller->view);
    infoview_destroy(controller->info);
    free(controller);
}

void snake_controller_redraw(SnakeController *controller) {
    snakeview_redraw(controller->view, controller->model->deq,
                     controller->model->food_pos);
    infoview_update_info(
        controller->info, controller->model->deq->length, controller->max_score,
        INIT_DELAY_MS / controller->delay_ms, controller->continues,
        timer_get_time(controller->timer));
}

void snakecontroller_end_loop(SnakeController *controller) {
    int maxy, maxx;
    getmaxyx(controller->view->win, maxy, maxx);

    int y = END_NLINES;
    int x = END_NCOLS;

    // don't need continue line
    if (controller->model->state == STATE_win) {
        y--;
    }

    WINDOW *end_border =
        derwin(controller->view->win, y, x, (maxy - y) / 2, (maxx - x) / 2);
    WINDOW *end_win = derwin(end_border, y - 2, x - 2, 1, 1);

    timer_pause(controller->timer);
    int score = controller->model->deq->length;
    controller->high_score =
        score > controller->high_score ? score : controller->high_score;

    if (controller->model->state == STATE_lose) {
        wprintw(end_win, "    YOU LOSE!\n");
        wprintw(end_win, " High Score: %d\n", controller->high_score);
        wprintw(end_win, " <c to continue>\n");
    } else if (controller->model->state == STATE_win) {
        wprintw(end_win, "     YOU WIN!\n");
        wprintw(end_win, " High Score: %d\n", controller->high_score);
    } else {
        fprintf(stderr, "invalid end state\n");
    }
    wprintw(end_win, " <r to restart>\n");
    box(end_border, 0, 0);
    wnoutrefresh(end_border);
    wnoutrefresh(end_win);
    doupdate();

    int ch;
    int nlines = controller->model->nlines;
    int ncols = controller->model->ncols;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
        case 'r':
            snake_destroy(controller->model);
            controller->model = snake_new(nlines, ncols);
            controller->delay_ms = INIT_DELAY_MS;
            controller->continues = 0;
            timer_restart(controller->timer);
            delwin(end_win);
            delwin(end_border);
            return;
        case 'c':
            if (controller->model->state == STATE_lose) {
                controller->continues += 1;
                controller->model->state = STATE_null;
                delwin(end_win);
                delwin(end_border);
                return;
            }
        default:
            break;
        }
    }

    delwin(end_win);
    delwin(end_border);
    snakecontroller_destroy(controller);
    endwin();
    exit(0);
}

#define HELP_NLINES 8
#define HELP_NCOLS 30

void snakecontroller_help_loop(SnakeController *controller) {
    int maxy, maxx;
    getmaxyx(controller->view->win, maxy, maxx);

    int y = HELP_NLINES;
    int x = HELP_NCOLS;

    WINDOW *help_border =
        derwin(controller->view->win, y, x, (maxy - y) / 2, (maxx - x) / 2);
    WINDOW *help_win = derwin(help_border, y - 2, x - 2, 1, 1);

    if (timer_paused(controller->timer) == false) {
        timer_pause(controller->timer);
    }

    wprintw(help_win, "           HELP\n");

    wprintw(help_win, " <space to flip direction>\n");
    wprintw(help_win, " <f to increase speed>\n");
    wprintw(help_win, " <s to decrease speed>\n");
    wprintw(help_win, " <h to show help / pause>\n");
    wprintw(help_win, " <F1 to quit>\n");
    box(help_border, 0, 0);
    wnoutrefresh(help_border);
    wnoutrefresh(help_win);
    doupdate();

    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
        case 'h':
            delwin(help_win);
            delwin(help_border);
            snake_controller_redraw(controller);
            return;
        default:
            break;
        }
    }

    delwin(help_win);
    delwin(help_border);
    snakecontroller_destroy(controller);
    endwin();
    exit(0);
}

void snakecontroller_loop(SnakeController *controller) {
    timer_start(controller->timer);
    snakeview_redraw(controller->view, controller->model->deq,
                     controller->model->food_pos);

    infoview_update_info(
        controller->info, controller->model->deq->length, controller->max_score,
        INIT_DELAY_MS / controller->delay_ms, controller->continues,
        timer_get_time(controller->timer));
    int ch;
    timeout(0);
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
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
            controller->delay_ms /= 1.5;
            break;
        case 's':
            controller->delay_ms *= 1.5;
            break;
        case 'h':
            snakecontroller_help_loop(controller);
            break;
        default:
            break;
        }
        if (controller->model->state == STATE_active) {
            if (timer_paused(controller->timer) == true) {
                timer_unpause(controller->timer);
            }
            snake_update(controller->model);
            snake_controller_redraw(controller);
        } else if (timer_paused(controller->timer) == false) {
            timer_pause(controller->timer);
        }

        if (controller->model->state == STATE_win ||
            controller->model->state == STATE_lose) {
            snakecontroller_end_loop(controller);
            snake_controller_redraw(controller);
        }

        napms(controller->delay_ms);
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    if (has_colors() == FALSE) {
        endwin();
        fprintf(stderr, "Your terminal does not support color\n");
        exit(1);
    }

    use_default_colors();
    start_color();
    init_pair(PAIR_SNAKE, COLOR_SNAKE, -1);
    init_pair(PAIR_FOOD, COLOR_FOOD, -1);
    init_pair(PAIR_BORDER, COLOR_BORDER, -1);

    refresh();

    int nlines = DEFAULT_LENGTH;
    int ncols = DEFAULT_LENGTH;

    if (argc == 2) {
        if (strcmp(argv[1], "MAX") != 0 || strcmp(argv[1], "max") != 0) {
            nlines = (LINES - 2 - 3) / 2;
            ncols = (COLS - 2) / 2;
        } else {
            nlines = strtol(argv[1], NULL, 0);
            ncols = strtol(argv[1], NULL, 0);
        }
    } else if (argc == 3) {
        nlines = strtol(argv[1], NULL, 0);
        ncols = strtol(argv[2], NULL, 0);
    }
    int view_nlines = nlines * 2 + 2 + 3;
    int view_ncols = ncols * 2 + 2;

    if (view_nlines > LINES || view_ncols > COLS || nlines * 2 < END_NLINES ||
        ncols * 2 < END_NCOLS || nlines * 2 < HELP_NLINES ||
        ncols * 2 < HELP_NCOLS || nlines <= 0 || ncols <= 0) {
        endwin();
        fprintf(stderr, "invalid dimensions\n");
        exit(1);
    }

    SnakeController *controller =
        snakecontroller_new(nlines, ncols, 4, (COLS - ncols * 2) / 2);
    snakecontroller_loop(controller);
    snakecontroller_destroy(controller);

    endwin();

    return EXIT_SUCCESS;
}
