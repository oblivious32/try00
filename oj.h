#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include "game_logic.h" 
#define OJ_NORMAL_CONTINUE 20   // OJ返回 (20, 20) 表示游戏正常继续
#define OJ_GAME_OVER     100   // OJ返回 (100, 100) 表示游戏结束

// 函数声明
void readInitialInput(GS *game);
char decideNextDirection(GS *game);
void printMoveAndScore(GS *game, char direction);
void readOJResponse(int resp[2]);
void placeNewFood(GS *game, int row, int col);
void printFinalMapAndScore(GS *game);
#endif