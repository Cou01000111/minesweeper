#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "c_logger.h"
#include "c_logger_value.h"
<<<<<<< HEAD
#define N_MINE 15
#define BD_WD 20
#define BD_HT 5
=======
#define N_MINE 100
#define BD_WD 40
#define BD_HT 12
>>>>>>> 288f29ad6e7ca87caeaf28bee2219aae58d3e124
#define ESC 0x1b
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4d
#define KEY_LEFT 0x4b
#define IS_UPPER_SIDE(y) (y == 0)
#define IS_LOWER_SIDE(y) (y == BD_HT - 1)
#define IS_LEFT_SIDE(x) (x == 0)
#define IS_RIGHT_SIDE(x) (x == BD_WD - 1)
#define IS_CURSOR (i == (size_t)g_cursor.y && j == (size_t)g_cursor.x)
#define BOM -1
#define TO_COOKED_MODE tcsetattr(STDIN_FILENO, 0, &CookedTermIos);
#define TO_RAW_MODE tcsetattr(STDIN_FILENO, 0, &RawTermIos);

struct termios CookedTermIos;  // cooked モード用
struct termios RawTermIos;     // raw モード用

typedef struct {
    int x;
    int y;
} t_pos;

typedef enum {
    NOT_DUG,
    DUG,
    PIN
} t_ground;

typedef enum {
    NONE,
    DIG_BOM,
    CLEAR
} t_game_endings;

//y,x
const t_pos VER_HORI_OBLIQUE[8] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {-1, -1}, {0, -1}};
const t_pos VER_HORI[4] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

//{NOT_DUG,DUG,PIN}
t_ground g_ground_bd[BD_HT][BD_WD];
t_pos g_cursor;
//{0,1,2,3,4,5,6,7,8,bom(-1)}
int g_under_bd[BD_HT][BD_WD];

int push_buf(char *buf, char *str) {
    strncpy(buf, str, strlen(str));
    return strlen(str);
}

bool render(bool has_all_bom_show) {
    TO_COOKED_MODE
    system("clear");
    char buf[(BD_HT * (BD_WD + 1) * 14) + 2];
    int buf_i = 0;
    for (size_t i = 0; i < BD_HT; i++) {
        for (size_t j = 0; j < BD_WD; j++) {
            if (g_under_bd[i][j] == BOM && has_all_bom_show)
                buf_i += push_buf(&buf[buf_i], IS_CURSOR ? "\x1b[7m\x1b[31mB" : "\x1b[31mB");
            else if (g_ground_bd[i][j] == NOT_DUG)
                buf_i += push_buf(&buf[buf_i], IS_CURSOR ? "\x1b[33mO" : "\x1b[7m\x1b[33mO");
            else if (g_ground_bd[i][j] == PIN)
                buf_i += push_buf(&buf[buf_i], IS_CURSOR ? "\x1b[7m\x1b[35mP" : "\x1b[43m\x1b[35mP");
            else if (g_under_bd[i][j] == BOM)
                buf_i += push_buf(&buf[buf_i], IS_CURSOR ? "\x1b[7m\x1b[31mB" : "\x1b[31mB");
            else if (g_under_bd[i][j] >= 0 && g_under_bd[i][j] <= 8) {
                sprintf(&buf[buf_i], IS_CURSOR ? "\x1b[7m%d" : "\x1b[37m%d", g_under_bd[i][j]);
                buf_i += IS_CURSOR ? 5 : 6;
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
                g_under_bd[y][x] += check_mine(y + VER_HORI_OBLIQUE[i].y, x + VER_HORI_OBLIQUE[i].x);
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
    if (x < 0 || y < 0 || BD_WD <= x || BD_HT <= y || g_ground_bd[y][x] == DUG || g_under_bd[y][x] == BOM)
        return;
    else {
        g_ground_bd[y][x] = 1;
        if ((g_under_bd[y][x] >= 1 && g_under_bd[y][x] <= 8) == false)
            for (size_t i = 0; i < 4; i++)
                dig(y + VER_HORI[i].y, x + VER_HORI[i].x);
    }
}

void pin(int y, int x) {
    if (g_ground_bd[y][x] == DUG) return;
    g_ground_bd[y][x] = (g_ground_bd[y][x] == PIN) ? NOT_DUG : PIN;
}

bool has_all_bom_found() {
    //全てのマス-地雷数 個 掘っていた場合true
    int count = 0;
    for (size_t y = 0; y < BD_HT; y++)
        for (size_t x = 0; x < BD_WD; x++)
            if (g_ground_bd[y][x] == NOT_DUG || g_ground_bd[y][x] == PIN) count++;
    //printf("あと%d", (BD_HT * BD_WD - N_MINE) - count);
    return (count == N_MINE);
}

void main_game(bool *is_game_run, t_game_endings *game_ending) {
    TO_RAW_MODE;
    int c[2];
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
    } else if (c[0] == ESC)
        *is_game_run = false;
    else if (c[0] == 32 || c[0] == 'x') {
        if (g_under_bd[g_cursor.y][g_cursor.x] == BOM) {
            *game_ending = DIG_BOM;
            *is_game_run = false;
        }
        dig(g_cursor.y, g_cursor.x);
    } else if (c[0] == 'r')
        pin(g_cursor.y, g_cursor.x);
    render(false);
    if (has_all_bom_found()) {
        *is_game_run = false;
        *game_ending = CLEAR;
    }
}

void ending(t_game_endings game_ending) {
    TO_COOKED_MODE;
    if (game_ending == CLEAR)
        puts("< GAME CLEAR!! >");
    else if (game_ending == DIG_BOM)
        puts("< GAME OVER !! >");
    else
        puts("< ERROR !! >");
}

int main() {
    t_game_endings game_ending = NONE;
    bool is_game_run = true;
    if (N_MINE > BD_HT * BD_WD) return 0;
    tcgetattr(STDIN_FILENO, &CookedTermIos);
    RawTermIos = CookedTermIos;
    cfmakeraw(&RawTermIos);
    TO_RAW_MODE
    init_g_values();
    render(false);
    while (is_game_run)
        main_game(&is_game_run, &game_ending);
    render(true);
    ending(game_ending);
}
