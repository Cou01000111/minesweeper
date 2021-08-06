#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "c_logger.h"
#include "c_logger_value.h"
#define N_MINE 10
#define BD_WD 20
#define BD_HT 10
#define ESC 0x1b
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4d
#define KEY_LEFT 0x4b

// 端末設定保存用大域変数
struct termios CookedTermIos;  // cooked モード用
struct termios RawTermIos;     // raw モード用
#define BOM -1

typedef enum { CENTER,
               UP,
               UP_RIGHT,
               RIGHT,
               DOWN_RIGHT,
               DOWN,
               DOWN_LEFT,
               LEFT,
               UP_LEFT } directions;

typedef struct {
    int x;
    int y;
} t_cursor;

//めくったかどうかがある{(めくった)1,(めくってない)0}
int g_ground_bd[BD_HT][BD_WD];
t_cursor g_cursor;
//{0,1,2,3,4,5,6,7,8,bom(-1)}
int g_under_bd[BD_HT][BD_WD];

int push_buf(char *buf, char *str) {
    strncpy(buf, str, strlen(str));
    return strlen(str);
}

bool render() {
    tcsetattr(STDIN_FILENO, 0, &CookedTermIos);
    system("clear");
    char buf[1024];
    int buf_i = 0;
    for (size_t i = 0; i < BD_HT; i++) {
        for (size_t j = 0; j < BD_WD; j++) {
            if (i == (size_t)g_cursor.y && j == (size_t)g_cursor.x) {
                printf("cursor:x:%d,y:%d\n", g_cursor.x, g_cursor.y);
                buf_i += push_buf(&buf[buf_i], "\x1b[7m");
            }
            if (!g_ground_bd[i][j])
                strncpy(&buf[buf_i], "O", 1);
            else if (g_under_bd[i][j] == BOM)
                strncpy(&buf[buf_i], "x", 1);
            else if (g_under_bd[i][j] >= 0 && g_under_bd[i][j] <= 8)
                sprintf(&buf[buf_i], "%d", g_under_bd[i][j]);
            else {
                c_logger_error_log("invalid g_ground_bd");
                return false;
            }
            buf_i++;
            buf_i += push_buf(&buf[buf_i], "\x1b[0m");
        }
        buf_i += push_buf(&buf[buf_i], "\n");
    }
    strncpy(&buf[buf_i], "\n\0", 2);
    puts(buf);
    tcsetattr(STDIN_FILENO, 0, &RawTermIos);
    return true;
}

void init_boards() {
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_ground_bd[i][j] = 0;
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_under_bd[i][j] = 0;
    g_cursor.x = 0;
    g_cursor.y = 0;
}

void add_bom() {
    for (size_t i = 0; i < N_MINE; i++) {
        srand((int)time(NULL));
        int h = rand() % BD_HT, w = rand() % BD_HT;
        while (g_under_bd[h][w] == BOM)
            h = rand() % BD_HT, w = rand() % BD_HT;
        g_under_bd[h][w] = BOM;
    }
}

void add_bom_count() {
    for (size_t y = 0; y < BD_HT; y++) {
        for (size_t x = 0; x < BD_WD; x++) {
            if (g_under_bd[y][x] == BOM)
                continue;
            //up(y > 0)
            if (y > 0 && g_under_bd[y - 1][x] == BOM) g_under_bd[y][x]++;
            //up right(y > 0 && x < BD_WD)
            if (y > 0 && x < BD_WD && g_under_bd[y - 1][x + 1] == BOM) g_under_bd[y][x]++;
            //right(x < BD_WD)
            if (x < BD_WD && g_under_bd[y][x + 1] == BOM) g_under_bd[y][x]++;
            //down right(y < BD_HT&&x < BD_WD)
            if (y < BD_HT && x < BD_WD && g_under_bd[y + 1][x + 1] == BOM) g_under_bd[y][x]++;
            //down(y < BD_HT)
            if (y < BD_HT && g_under_bd[y + 1][x] == BOM) g_under_bd[y][x]++;
            //down left(y < BD_HT&&x > 0)
            if (y < BD_HT && x > 0 && g_under_bd[y + 1][x - 1] == BOM) g_under_bd[y][x]++;
            //left(x > 0)
            if (x > 0 && g_under_bd[y][x - 1] == BOM) g_under_bd[y][x]++;
            //up left(y < BD_HT&&x > 0)
            if (y < BD_HT && x > 0 && g_under_bd[y - 1][x - 1] == BOM) g_under_bd[y][x]++;
        }
    }
}

//成功なら1,失敗なら0
bool init_g_values() {
    init_boards();
    add_bom();
    add_bom_count();
    return true;
}

void dig(int y, int x, directions dirc) {
    char str[1000];
    sprintf(str, "y:%d,x:%d", y, x);
    c_logger_debug_log(str);
    g_ground_bd[y][x] = 1;
    //up(y > 0)
    if (dirc != UP && y > 0 && g_under_bd[y - 1][x] == 0) dig(y - 1, x, DOWN);
    //up right(y > 0 && x < BD_WD)
    if (dirc != UP_RIGHT && y > 0 && x < BD_WD && g_under_bd[y - 1][x + 1] == 0) dig(y - 1, x + 1, DOWN_LEFT);
    //right(x < BD_WD)
    if (dirc != RIGHT && x < BD_WD && g_under_bd[y][x + 1] == 0) dig(y, x + 1, LEFT);
    //down right(y < BD_HT&&x < BD_WD)
    if (dirc != DOWN_RIGHT && y < BD_HT && x < BD_WD && g_under_bd[y + 1][x + 1] == 0) dig(y + 1, x + 1, UP_LEFT);
    //down(y < BD_HT)
    if (dirc != DOWN && y < BD_HT && g_under_bd[y + 1][x] == 0) dig(y + 1, x, UP);
    //down left(y < BD_HT&&x > 0)
    if (dirc != DOWN_LEFT && y < BD_HT && x > 0 && g_under_bd[y + 1][x - 1] == 0) dig(y + 1, x - 1, UP_RIGHT);
    //left(x > 0)
    if (dirc != LEFT && x > 0 && g_under_bd[y][x - 1] == 0) dig(y, x - 1, RIGHT);
    //up left(y < BD_HT&&x > 0)
    if (dirc != UP_LEFT && y < BD_HT && x > 0 && g_under_bd[y - 1][x - 1] == 0) dig(y - 1, x - 1, DOWN_RIGHT);
}

int main() {
    // 初期状態の端末設定 (cooked モード) を取得・保存する．
    tcgetattr(STDIN_FILENO, &CookedTermIos);
    // raw モードの端末設定を作成・保存する．
    RawTermIos = CookedTermIos;
    cfmakeraw(&RawTermIos);
    // 端末を raw モードに設定する．
    tcsetattr(STDIN_FILENO, 0, &RawTermIos);
    int c[2];
    bool isnt_game_fin = true;
    init_g_values();
    render();
    while (isnt_game_fin) {
        if ((c[0] = getchar()) == ESC && (c[1] = getchar()) == '[') {
            switch ((c[0] = getchar())) {
                case 'A':
                    g_cursor.y += (g_cursor.y > 0) ? -1 : 0;
                    break;
                case 'B':
                    g_cursor.y += (g_cursor.y < BD_HT - 1) ? 1 : 0;
                    break;
                case 'C':
                    g_cursor.x += (g_cursor.x < BD_WD - 1) ? 1 : 0;
                    break;
                case 'D':
                    g_cursor.x += (g_cursor.x > 0) ? -1 : 0;
                    break;
            }
            render();
        } else if (c[0] == ESC)
            break;
        else if (c[0] == 32) {
            dig(g_cursor.y, g_cursor.x, CENTER);
        }
    }
    /*
    初期画面
        ボード初期化
    ゲーム画面
        操作を入力
        データを操作
        データをもとに画面を出力
    */
    tcsetattr(STDIN_FILENO, 0, &CookedTermIos);
}
