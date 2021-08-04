#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "c_logger.h"
#include "c_logger_value.h"
#define N_MINE 10
#define BD_WD 9
#define BD_HT 9

#define BOM -1

void zero_boards();
bool init_board();
void render();

//実際に表示するボード{未めくり(9)、0,1,2,3,4,5,6,7,8,bom(-1)}
int g_display_board[BD_HT][BD_WD];
int g_board[BD_HT][BD_WD];
int g_under_board[BD_HT][BD_WD];  //爆弾の数が入っているボード、爆弾は-1

int main() {
    bool is_game_finished = 0;
    init_board();
    render();
    /*
    初期画面
        ボード初期化
    ゲーム画面
        操作を入力
        データを操作
        データをもとに画面を出力
    */
}

void render() {
    system("clear");
    for (size_t i = 0; i < BD_HT; i++) {
        for (size_t j = 0; j < BD_WD; j++) {
            switch (g_display_board[i][j]) {
                case BOM:
                    printf("x");
                    break;
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    printf("%d", g_display_board[i][j]);
                    break;
                case 9:
                    printf("□");
                    break;
                default:
                    c_logger_error_log("invalid g_display_board");
                    break;
            }
        }
        puts("");
    }
}

//成功なら1,失敗なら0
bool init_board() {
    zero_boards();
    for (size_t i = 0; i < N_MINE; i++) {
        int h = rand() % BD_HT, w = rand() % BD_HT;
        while (g_under_board[h][w] == BOM)
            h = rand() % BD_HT, w = rand() % BD_HT;
        g_under_board[h][w] = BOM;
    }
}

void zero_boards() {
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_display_board[i][j] = 0;
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_board[i][j] = 0;
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_under_board[i][j] = 0;
}
