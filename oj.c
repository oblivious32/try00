#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_logic.h"
#include "oj.h"


void readInitialInput(GS *game) {
    char input_map[ROWS][COLS + 1];
    int N;
    int i = 0;
    while (i < ROWS) {
        if (fgets(input_map[i], COLS + 2, stdin) == NULL) {
            game->game_over = true;
            return;
        }
        char *nl = strchr(input_map[i], '\n');
        if (nl) *nl = '\0';
        i++;
    }
    if (scanf("%d", &N) != 1) {
        game->game_over = true;
        return;
    }
    getchar();
    initGame(game, input_map, N);
}
static bool isBlocked(char cell) {
    return (cell == WALL || cell == OBSTACLE || cell == BODY || cell == HEAD);
}

static char oppositeDir(char dir) {
    switch (dir) {
        case UP:    return DOWN;
        case DOWN:  return UP;
        case LEFT:  return RIGHT;
        case RIGHT: return LEFT;
        default:    return DOWN;
    }
}

static int floodCount(GS *game, int sr, int sc, int limit) {
    Pos queue[ROWS * COLS];
    int qf = 0, qb = 0;
    bool visited[ROWS][COLS];
    memset(visited, 0, sizeof(visited));

    queue[qb].row = sr;
    queue[qb].col = sc;
    qb++;
    visited[sr][sc] = true;
    int cnt = 0;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (qf < qb && cnt < limit) {
        Pos cur = queue[qf];
        qf++;
        cnt++;
        for (int d = 0; d < 4; d++) {
            int nr = cur.row + dr[d];
            int nc = cur.col + dc[d];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (visited[nr][nc]) continue;
            if (isBlocked(game->map[nr][nc])) continue;
            visited[nr][nc] = true;
            queue[qb].row = nr;
            queue[qb].col = nc;
            qb++;
        }
    }
    return cnt;
}

static char bfsToFood(GS *game) {
    Snake *snake = &(game->snake);
    Pos head = getSnakeHead(snake);

    bool food_exists = false;
    for (int i = 0; i < ROWS && !food_exists; i++)
        for (int j = 0; j < COLS && !food_exists; j++)
            if (game->map[i][j] == FOOD) food_exists = true;
    if (!food_exists) return '\0';

    Pos queue[ROWS * COLS];
    int qf = 0, qb = 0;
    Pos prev[ROWS][COLS];
    bool visited[ROWS][COLS];
    memset(visited, 0, sizeof(visited));

    queue[qb] = head;
    qb++;
    visited[head.row][head.col] = true;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    Pos food_pos = {-1, -1};
    bool found = false;

    while (qf < qb) {
        Pos cur = queue[qf];
        qf++;
        if (game->map[cur.row][cur.col] == FOOD) {
            food_pos = cur;
            found = true;
            break;
        }
        for (int d = 0; d < 4; d++) {
            int nr = cur.row + dr[d];
            int nc = cur.col + dc[d];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            if (visited[nr][nc]) continue;
            if (isBlocked(game->map[nr][nc]) && game->map[nr][nc] != FOOD) continue;
            visited[nr][nc] = true;
            prev[nr][nc] = cur;
            queue[qb].row = nr;
            queue[qb].col = nc;
            qb++;
        }
    }

    if (!found) return '\0';

    Pos cur = food_pos;
    while (prev[cur.row][cur.col].row != head.row ||
           prev[cur.row][cur.col].col != head.col) {
        cur = prev[cur.row][cur.col];
    }

    if (cur.row < head.row)      return UP;
    else if (cur.row > head.row) return DOWN;
    else if (cur.col < head.col) return LEFT;
    else                         return RIGHT;
}

char decideNextDirection(GS *game) {
    Snake *snake = &(game->snake);
    Pos head = getSnakeHead(snake);
    char opp = oppositeDir(snake->current_dir);

    char bfs_dir = bfsToFood(game);
    if (bfs_dir != '\0') {
        return bfs_dir;
    }

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};
    char dirs[] = {UP, DOWN, LEFT, RIGHT};
    char best = snake->current_dir;
    int best_cnt = -1;

    for (int d = 0; d < 4; d++) {
        if (dirs[d] == opp) continue;
        int nr = head.row + dr[d];
        int nc = head.col + dc[d];
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
        if (isBlocked(game->map[nr][nc])) continue;
        bool hit_self = false;
        for (int i = 0; i < snake->length - 1; i++) {
            if (snake->body[i].row == nr && snake->body[i].col == nc) {
                hit_self = true;
                break;
            }
        }
        if (hit_self) continue;

        int space = floodCount(game, nr, nc, 200);
        if (space > best_cnt) {
            best_cnt = space;
            best = dirs[d];
        }
    }

    if (best_cnt >= 0) return best;

    int nr = head.row, nc = head.col;
    switch (snake->current_dir) {
        case UP:    nr--; break;
        case DOWN:  nr++; break;
        case LEFT:  nc--; break;
        case RIGHT: nc++; break;
    }
    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
        !isBlocked(game->map[nr][nc])) {
        return snake->current_dir;
    }
    return UP;
}
void printMoveAndScore(GS *game, char direction) {
    putchar(direction);
    putchar('\n');
    printf("%d\n", game->snake.score);
    fflush(stdout);
}
void readOJResponse(int resp[2]) {
    char buf[32];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        resp[0] = OJ_GAME_OVER;
        resp[1] = OJ_GAME_OVER;
        return;
    }
    if (sscanf(buf, "%d %d", &resp[0], &resp[1]) != 2) {
        resp[0] = OJ_GAME_OVER;
        resp[1] = OJ_GAME_OVER;
    }
}
void placeNewFood(GS *game, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) goto fallback;
    {
        char t = game->map[row][col];
        if (t == WALL || t == OBSTACLE || t == HEAD || t == BODY) goto fallback;
        game->map[row][col] = FOOD;
        return;
    }
fallback:
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (game->map[i][j] == EMPTY) {
                game->map[i][j] = FOOD;
                return;
            }
        }
    }
}

/**
 * 输出最终的地图和分数
 * 当游戏结束时调用，输出20行地图和1行分数
 */
void printFinalMapAndScore(GS *game) {
    int r = ROWS;
    int idx = 0;
    while (r--) {
        puts(game->map[idx]);
        idx++;
    }
    printf("%d\n", game->snake.score);
    fflush(stdout);
}