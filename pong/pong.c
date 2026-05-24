#include "utils/utils.c"
#include "utils/vec.c"
#include <stdio.h>
#include <stdlib.h>

#define W 50
#define H 10

typedef struct {
    ivec2 pos;
    ivec2 vel;
} Ball;

typedef struct {
    ivec2 pos;
    int points;
} PaddlePlayer;

void draw(Ball ball, PaddlePlayer p1, PaddlePlayer p2) {
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (
                (ball.pos.x == x && ball.pos.y == y) or
                
                (p1.pos.x == x && p1.pos.y == (y+1)) or
                (p1.pos.x == x && p1.pos.y == y)     or
                (p1.pos.x == x && p1.pos.y == (y-1)) or

                (p2.pos.x == x && p2.pos.y == (y+1)) or
                (p2.pos.x == x && p2.pos.y == y)     or
                (p2.pos.x == x && p2.pos.y == (y-1))
            ) {
                fputs("\033[48;5;7m  \033[0m", stdout);
            } else {
                fputs("\033[48;5;232m  \033[0m", stdout);
            }
        }
        printf("\n");
    }
    printf("Player 1: %d | Player 2: %d |\n", p1.points, p2.points);
}

int main() {
    seed(now_ms() ^ (uintptr_t)&main);

    ConsoleBufferingDisable();
    ConsoleHideCursor();
    ConsoleClear();

    int64_t ballTick = now_ms(); // 50ms

    enum {
        P2,
        C,
        err
    } mode;

    Ball ball = {
        .pos = {W / 2, H / 2},
        .vel = {1, 1}
    };

    PaddlePlayer p1 = {
        .pos = {0, H / 2},
        .points = 0
    };

    PaddlePlayer p2 = {
        .pos = {W-1, H/2},
        .points = 0,
    };

    do {
        Array* ask = input("Select Against [0: Computer, 1: Player]: ");
        mode = ((char*)(ask->items))[ask->size-1] == '0' ?
            C : ((char*)(ask->items))[ask->size-1] == '1' ?
                P2 : err;
    } while (mode == err);

    while (true) {
        ConsoleGoToHome();
        draw(ball, p1, p2);

        if (keyPressed('W') && p1.pos.y > 1) {
            p1.pos.y--;
        }
        
        if (keyPressed('S') && p1.pos.y < H - 2) {
            p1.pos.y++;
        }
        
        if (mode == P2) {
            if (keyPressed('O') && p2.pos.y > 1) {
                p2.pos.y--;
            }
        
            if (keyPressed('L') && p2.pos.y < H - 2) {
                p2.pos.y++;
            }
        } else if (mode == C) {
            if (ball.vel.x > 0 && ball.pos.x > W / 2) {
                if (ball.pos.y < p2.pos.y && p2.pos.y > 2) {
                    p2.pos.y--;
        
                } else if (ball.pos.y > p2.pos.y && p2.pos.y < H - 3) {
                    p2.pos.y++;
                }
            }
        }

        if ((now_ms() - ballTick) > 50) {
            if (ball.pos.y <= 0 || ball.pos.y >= H - 2) {
                ball.vel.y *= -1;
            }

            ivec2 next = {
                ball.pos.x + ball.vel.x,
                ball.pos.y + ball.vel.y
            };

            if (
                (
                    next.x == p1.pos.x &&
                    abs(next.y - p1.pos.y) <= 1
                ) ||
                (
                    next.x == p2.pos.x &&
                    abs(next.y - p2.pos.y) <= 1
                )
            ) {
                ball.vel.x *= -1;
            }

            ball.pos.x += ball.vel.x;
            ball.pos.y += ball.vel.y;

            if (ball.pos.x < 0) {
                p2.points++;
                ball.pos = (ivec2){W / 2, H / 2};
                ball.vel = (ivec2){1, randint(0, 1) ? 1 : -1};
            }

            if (ball.pos.x > W - 1) {
                p1.points++;
                ball.pos = (ivec2){W / 2, H / 2};
                ball.vel = (ivec2){-1, randint(0, 1) ? 1 : -1};
            }

            ballTick = now_ms();
        }
    }
}
