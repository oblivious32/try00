#include<stdio.h>
#include<stdlib.h>
#include"game_logic.h"
#include "oj.h"
int main() {
    GS game;
    readInitialInput(&game);
    if (game.game_over) return 1;

    for (;;) {
        char dir = decideNextDirection(&game);
        printMoveAndScore(&game, dir);
        moveSnake(&game, dir);
        checkAndGrow(&game);

        int resp[2];
        readOJResponse(resp);

        int quit = game.game_over;
        if (!quit) {
            int r0 = resp[0];
            if (r0 == OJ_NORMAL_CONTINUE) {
                if (resp[1] != OJ_NORMAL_CONTINUE)
                    placeNewFood(&game, resp[0], resp[1]);
            } else if (r0 == OJ_GAME_OVER) {
                if (resp[1] == OJ_GAME_OVER)
                    quit = 1;
                else
                    placeNewFood(&game, resp[0], resp[1]);
            } else {
                placeNewFood(&game, resp[0], resp[1]);
            }
        }

        if (quit) {
            printFinalMapAndScore(&game);
            return 0;
        }
    }
}