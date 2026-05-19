#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include"game_logic.h"

Pos getSnakePos(const Snake *snake, int index) {
    return snake->body[index];
}

Pos getSnakeHead(const Snake *snake) {
    return snake->body[snake->length - 1];
}

Pos getSnakeTail(const Snake *snake) {
    return snake->body[0];
}

static bool checkSelfCollision(GS *game, int new_head_row, int new_head_col, bool will_eat_food) {
    Snake *snake = &(game->snake);
    int check_count = snake->length - (will_eat_food ? 0 : 1);
    for (int i = 0; i < check_count; i++) {
        if (snake->body[i].row == new_head_row && snake->body[i].col == new_head_col) {
            return true;
        }
    }
    return false;
}

void initGame(GS *game, char input_map[ROWS][COLS+1], int N) {
    for (int i = 0; i < ROWS; i++) {
        strcpy(game->map[i], input_map[i]);
    }

    Snake *snake = &(game->snake);
    snake->length = 0;
    snake->score = 0;
    snake->current_dir = UP;

    int head_row = -1, head_col = -1;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (game->map[i][j] == HEAD) {
                head_row = i;
                head_col = j;
                break;
            }
        }
        if (head_row != -1) break;
    }

    if (head_row != -1) {
        snake->body[0].row = head_row;
        snake->body[0].col = head_col;
        snake->length = 1;

        int cur_r = head_row, cur_c = head_col;
        int dirs[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};

        while (1) {
            bool found = false;
            for (int d = 0; d < 4; d++) {
                int nr = cur_r + dirs[d][0];
                int nc = cur_c + dirs[d][1];
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (game->map[nr][nc] != BODY) continue;

                bool already = false;
                for (int k = 0; k < snake->length; k++) {
                    if (snake->body[k].row == nr && snake->body[k].col == nc) {
                        already = true;
                        break;
                    }
                }
                if (already) continue;

                for (int k = snake->length; k > 0; k--) {
                    snake->body[k] = snake->body[k - 1];
                }
                snake->body[0].row = nr;
                snake->body[0].col = nc;
                snake->length++;
                cur_r = nr;
                cur_c = nc;
                found = true;
                break;
            }
            if (!found) break;
        }

        if (snake->length >= 2) {
            Pos neck = snake->body[snake->length - 2];
            Pos head = snake->body[snake->length - 1];
            if (head.row < neck.row)      snake->current_dir = UP;
            else if (head.row > neck.row) snake->current_dir = DOWN;
            else if (head.col < neck.col) snake->current_dir = LEFT;
            else if (head.col > neck.col) snake->current_dir = RIGHT;
        }
    }

    game->N = N;
    game->step_counter = 0;
    game->game_over = false;
    game->last_grow_step = -1;
}

void moveSnake(GS *game, char direction) {
    Snake *snake = &(game->snake);

    if ((snake->current_dir == UP && direction == DOWN) ||
        (snake->current_dir == DOWN && direction == UP) ||
        (snake->current_dir == LEFT && direction == RIGHT) ||
        (snake->current_dir == RIGHT && direction == LEFT)) {
        setGameOver(game);
        return;
    }

    if (direction != UP && direction != DOWN &&
        direction != LEFT && direction != RIGHT) {
        fprintf(stderr, "[警告] 无效方向输入 '%c'，蛇保持不动\n", direction);
        return;
    }

    Pos head = snake->body[snake->length - 1];
    int new_r = head.row;
    int new_c = head.col;

    switch (direction) {
        case UP:    new_r--; break;
        case DOWN:  new_r++; break;
        case LEFT:  new_c--; break;
        case RIGHT: new_c++; break;
        default: break;
    }

    if (new_r < 0 || new_r >= ROWS || new_c < 0 || new_c >= COLS) {
        setGameOver(game);
        return;
    }

    char target = game->map[new_r][new_c];
    if (target == WALL || target == OBSTACLE) {
        setGameOver(game);
        return;
    }

    bool will_eat_food = (target == FOOD);
    if (checkSelfCollision(game, new_r, new_c, will_eat_food)) {
        setGameOver(game);
        return;
    }

    Pos old_head = head;
    Pos old_tail = snake->body[0];

    if (will_eat_food) {
        snake->body[snake->length].row = new_r;
        snake->body[snake->length].col = new_c;
        snake->length++;
        snake->score += 10;
    } else {
        for (int i = 0; i < snake->length - 1; i++) {
            snake->body[i] = snake->body[i + 1];
        }
        snake->body[snake->length - 1].row = new_r;
        snake->body[snake->length - 1].col = new_c;
    }

    if (snake->length > 1) {
        game->map[old_head.row][old_head.col] = BODY;
    }
    game->map[new_r][new_c] = HEAD;

    if (!will_eat_food) {
        game->saved_tail = old_tail;
        if (old_tail.row != new_r || old_tail.col != new_c) {
            game->map[old_tail.row][old_tail.col] = EMPTY;
        }
    }

    game->step_counter++;
    snake->current_dir = direction;

    if (will_eat_food && game->N > 0 && game->step_counter % game->N == 0) {
        game->last_grow_step = game->step_counter;
    }
}

void checkAndGrow(GS *game) {
    Snake *snake = &(game->snake);

    if (game->N > 0 && game->step_counter % game->N == 0) {
        if (game->step_counter != game->last_grow_step) {
            Pos old_tail = game->saved_tail;
            for (int i = snake->length; i > 0; i--) {
                snake->body[i] = snake->body[i - 1];
            }
            snake->body[0] = old_tail;
            snake->length++;
            game->map[old_tail.row][old_tail.col] = BODY;
            game->last_grow_step = game->step_counter;
        }
    }
}

void setGameOver(GS *game) {
    game->game_over = true;
}
