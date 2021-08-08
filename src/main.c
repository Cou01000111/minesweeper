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
#define BD_WD 10
#define BD_HT 5
#define ESC 0x1b
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4d
#define KEY_LEFT 0x4b
#define IS_UPPER_SIDE(y) (y == 0)
#define IS_LOWER_SIDE(y) (y == BD_HT - 1)
#define IS_LEFT_SIDE(x) (x == 0)
#define IS_RIGHT_SIDE(x) (x == BD_WD - 1)

// 端末設定保存用大域変数
struct termios CookedTermIos;  // cooked モード用
struct termios RawTermIos;     // raw モード用
#define BOM -1

//y,x
int vertical_horizontal_oblique[8][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {-1, -1}, {0, -1}};
int vertical_horizontal[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

typedef struct {
    int x;
    int y;
} t_cursor;

//めくったかどうかがある{(めくった)1,(めくってない)0,(pin)2}
int g_ground_bd[BD_HT][BD_WD];
t_cursor g_cursor;
//{0,1,2,3,4,5,6,7,8,bom(-1)}
int g_under_bd[BD_HT][BD_WD];
int g_dig_bord[BD_HT][BD_WD];

int push_buf(char *buf, char *str) {
    strncpy(buf, str, strlen(str));
    return strlen(str);
}

bool render() {
    tcsetattr(STDIN_FILENO, 0, &CookedTermIos);
    system("clear");
    char buf[2048];
    int buf_i = 0;
    for (size_t i = 0; i < BD_HT; i++) {
        for (size_t j = 0; j < BD_WD; j++) {
            if (i == (size_t)g_cursor.y && j == (size_t)g_cursor.x) {
                printf("cursor:x:%d,y:%d\n", g_cursor.x, g_cursor.y);
                buf_i += push_buf(&buf[buf_i], "\x1b[7m");
            }
            if (g_ground_bd[i][j] == 0)
                buf_i += push_buf(&buf[buf_i], "\x1b[33mO");
            else if (g_ground_bd[i][j] == 2)
                buf_i += push_buf(&buf[buf_i], "\x1b[35mP");
            else if (g_under_bd[i][j] == BOM)
                buf_i += push_buf(&buf[buf_i], "\x1b[31mB");
            else if (g_under_bd[i][j] >= 0 && g_under_bd[i][j] <= 8) {
                sprintf(&buf[buf_i], "%d", g_under_bd[i][j]);
                buf_i += 1;
            } else {
                c_logger_error_log("invalid g_ground_bd");
                return false;
            }
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

void init_dig_bord() {
    for (size_t i = 0; i < BD_HT; i++)
        for (size_t j = 0; j < BD_WD; j++) g_dig_bord[i][j] = 0;
}

void add_bom() {
    for (size_t i = 0; i < N_MINE; i++) {
        srand((int)time(NULL));
        int h = rand() % BD_HT, w = rand() % BD_WD;
        while (g_under_bd[h][w] == BOM)
            h = rand() % BD_HT, w = rand() % BD_WD;
        g_under_bd[h][w] = BOM;
    }
}

int check_mine(int y, int x) {
    if (x < 0 || y < 0 || BD_WD <= x || BD_HT <= y) return false;
    return g_under_bd[y][x] == BOM;
}

void add_bom_count() {
    for (size_t y = 0; y < BD_HT; y++) {
        for (size_t x = 0; x < BD_WD; x++) {
            if (g_under_bd[y][x] == BOM)
                continue;
            for (size_t i = 0; i < 8; i++)
                g_under_bd[y][x] += check_mine(y + vertical_horizontal_oblique[i][1], x + vertical_horizontal_oblique[i][0]);
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

void dig(int y, int x) {
    if (x < 0 || y < 0 || BD_WD <= x || BD_HT <= y || g_ground_bd[y][x] == 1 || g_under_bd[y][x] == BOM)
        return;
    else {
        g_ground_bd[y][x] = 1;
        if ((g_under_bd[y][x] >= 1 && g_under_bd[y][x] <= 8) == false)
            for (size_t i = 0; i < 4; i++)
                dig(y + vertical_horizontal[i][1], x + vertical_horizontal[i][0]);
    }
}

void pin(int y, int x) {
    g_ground_bd[y][x] = 2;
}

int main() {
    if (N_MINE > BD_HT * BD_WD) return 0;
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
                    g_cursor.y += (g_cursor.y < (BD_HT - 1)) ? 1 : 0;
                    break;
                case 'C':
                    g_cursor.x += (g_cursor.x < (BD_WD - 1)) ? 1 : 0;
                    break;
                case 'D':
                    g_cursor.x += (g_cursor.x > 0) ? -1 : 0;
                    break;
            }
            render();
        } else if (c[0] == ESC)
            break;
        else if (c[0] == 32 || c[0] == 'x')
            dig(g_cursor.y, g_cursor.x);
        else if (c[0] == 'z')
            pin(g_cursor.y, g_cursor.x);
        else if (c[0] == 'c')
            pin(g_cursor.y, g_cursor.x);
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
