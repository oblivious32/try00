#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H
#include <stdbool.h>
#define ROWS 20
#define COLS 20
#define EMPTY    '.'   
#define WALL     '#'   
#define HEAD     'H'   
#define BODY     'B'   
#define FOOD     'F'   
#define OBSTACLE 'O'   
#define UP    'W'
#define LEFT  'A'
#define DOWN  'S'
#define RIGHT 'D'
typedef struct {
    int row;
    int col;
} Pos;

typedef struct {
    Pos body[ROWS * COLS];
    int length;
    char current_dir;
    int score;
} Snake;

typedef struct {
    char map[ROWS][COLS + 1];
    Snake snake;
    int N;
    int step_counter;
    bool game_over;
    int last_grow_step;
    Pos saved_tail;
} GS;

Pos getSnakePos(const Snake *snake, int index);
Pos getSnakeHead(const Snake *snake);
Pos getSnakeTail(const Snake *snake);

void initGame(GS *game, char input_map[ROWS][COLS+1], int N);
void moveSnake(GS *game, char direction);
void checkAndGrow(GS *game);
void setGameOver(GS *game);
#endif