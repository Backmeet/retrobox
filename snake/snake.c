#include "utils/utils.c"
#include "utils/vec.c"
#include <stdio.h>
#include <stdlib.h>

#define GRIDSIZE 10
#define APPLECOUNT 3

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct {
    ivec2 pos;
    Direction dir;
} bodytile;

typedef struct {
    ivec2 head;
    Array* body;
    Direction dir;
} Snake;

void draw(Snake* p, Array* a) {
    if (a->itemsize != sizeof(ivec2)) {
        panic("Error: in draw, apple position array itemsize != ivec2");
    }
    enum {
        EMPTY,
        SNAKE,
        APPLE
    } grid[GRIDSIZE][GRIDSIZE] = {0};

    static const char* enumToText[] = {
        [SNAKE] = "\033[48;5;46m  \033[0m",
        [APPLE] = "\033[48;5;196m  \033[0m",
        [EMPTY] = "\033[48;5;232m  \033[0m",
    };

    grid[p->head.x][p->head.y] = SNAKE;
    for (size_t i = 0; i != p->body->size; i++) {
        ivec2 pos = (*(bodytile**)Arrayindex(p->body, i))->pos;
        grid[pos.x][pos.y] = SNAKE;
    }

    for (size_t i = 0; i != a->size; i++) {
        ivec2* pos = *(ivec2**)Arrayindex(a, i);
        grid[pos->x][pos->y] = APPLE;
    }

    for (int y = 0; y != GRIDSIZE; y++) {
        for (int x = 0; x != GRIDSIZE; x++) {
            write(1, enumToText[grid[x][y]], strlen(enumToText[grid[x][y]]));
        }
        switch (y) {
            case 0:
                printf("   Score %d", (int)p->body->size);
            break;
            case 2:
                printf(keyPressed('W') ? "    \033[30;47mW\033[0m" : "    W");
            break;
            case 3:
                printf(keyPressed('A') ? "   \033[30;47mA\033[0m" : "   A");
                printf(keyPressed('S') ? "\033[30;47mS\033[0m" : "S");
                printf(keyPressed('D') ? "\033[30;47mD\033[0m" : "D");
            break;
        }
        putchar('\n');
    }
}

void movePlayer(Snake* player) {
    ivec2 oldHead = player->head;

    switch (player->dir) {
        case UP:    player->head.y -= 1; break;
        case DOWN:  player->head.y += 1; break;
        case LEFT:  player->head.x -= 1; break;
        case RIGHT: player->head.x += 1; break;
    }

    if (player->head.x < 0) player->head.x = GRIDSIZE - 1;
    if (player->head.x >= GRIDSIZE) player->head.x = 0;
    if (player->head.y < 0) player->head.y = GRIDSIZE - 1;
    if (player->head.y >= GRIDSIZE) player->head.y = 0;

    if (player->body->size == 0) return;

    bodytile* tail = *(bodytile**)Arrayindex(player->body, player->body->size - 1);

    Arrayremove(player->body, player->body->size - 1);

    tail->pos = oldHead;
    tail->dir = player->dir;

    Arrayinsert(player->body, 0, &tail);
}


void eat(Snake* player) {
    bodytile* tile = malloc(sizeof(bodytile));

    ivec2 base;
    Direction dir;

    if (player->body->size == 0) {
        base = player->head;
        dir = player->dir;
    } else {
        bodytile* tail = *(bodytile**)Arrayindex(player->body, player->body->size - 1);
        base = tail->pos;
        dir = tail->dir;
    }

    tile->pos = base;
    tile->dir = dir;

    switch (dir) {
        case UP:    tile->pos.y += 1; break;
        case DOWN:  tile->pos.y -= 1; break;
        case LEFT:  tile->pos.x -= 1; break;
        case RIGHT: tile->pos.x += 1; break;
    }

    if (tile->pos.x < 0) tile->pos.x = GRIDSIZE - 1;
    if (tile->pos.x >= GRIDSIZE) tile->pos.x = 0;
    if (tile->pos.y < 0) tile->pos.y = GRIDSIZE - 1;
    if (tile->pos.y >= GRIDSIZE) tile->pos.y = 0;

    Arrayadd(player->body, &tile);
}

void eatIfCan(Snake* player, Array* apples) {
    for (size_t i = 0; i != apples->size; i++) {
        ivec2* a = *(ivec2**)Arrayindex(apples, i);
        if (player->head.x == a->x && player->head.y == a->y) {
            eat(player);
            free(a);
            Arrayremove(apples, i);
            return;
        }
    }
}

static Snake player;
static Array* apples;
static TickFrame* f = NULL;

void atEnd() {
    Arrayfree(player.body);
    for (size_t i = 0; i != apples->size; i++) {
        free(*(ivec2**)Arrayindex(apples, i));
    }
    Arrayfree(apples);
    free(f);
}
//                                bodytile ivec2
bool BodyTilePosEqual(void* self, void* t, void* p) {
    return ((bodytile*)t)->pos.x == ((ivec2*)p)->x and
           ((bodytile*)t)->pos.y == ((ivec2*)p)->y;
}

int main() {
    seed(now_ms() ^ (uintptr_t)&main); // randomness final boss
    atexit(atEnd);

    ConsoleBufferingDisable();
    ConsoleHideCursor();
    ConsoleClear();

    player.head.x = randint(0, GRIDSIZE-1);
    player.head.y = randint(0, GRIDSIZE-1);
    player.dir = randint(0, RIGHT);
    player.body = Arrayinit(sizeof(bodytile*), 0);
    player.body->eq = BodyTilePosEqual;

    for (size_t i = 0; i != 3; i++) {
        eat(&player);
    }

    apples = Arrayinit(sizeof(ivec2*), 0);
    apples->eq = Array_EqualAsDerefPtr;
    int64_t lasttick = now_ms();
    Direction nextDir = player.dir;

    while (1) {
        if (keyPressed('W') && player.dir != DOWN) {
            nextDir = UP;
        } else if (keyPressed('A') && player.dir != RIGHT) {
            nextDir = LEFT;
        } else if (keyPressed('S') && player.dir != UP) {
            nextDir  = DOWN;
        } else if (keyPressed('D') && player.dir != LEFT) {
            nextDir = RIGHT;
        }

        if ((now_ms() - lasttick) > 500) {
            player.dir = nextDir;
            lasttick = now_ms();

            if (Arrayfind(player.body, &player.head) != -1) {
                ConsoleGoToHome();
                puts("Game Over!");
                for (int i = 0; i != GRIDSIZE; i++) {
                    putchar('\n');
                }
                return 0;
            }

            if (!apples->size) {
                for (size_t i = 0; i != APPLECOUNT; i++) {
                    ivec2* apple = malloc(sizeof(ivec2));
                    do {
                        apple->x = randint(0, GRIDSIZE-1);
                        apple->y = randint(0, GRIDSIZE-1);
                    } while (
                        Arrayfind(apples, apple) != -1 or       // apples cant be on top of eachother
                        Arrayfind(player.body, apple) != -1 or  // apples cant be on the snake's body
                        apple->x == player.head.x or            // apples cant be on the head of the snake
                        apple->y == player.head.y
                    );

                    Arrayadd(apples, &apple);
                }
            }

            movePlayer(&player);
            eatIfCan(&player, apples);
        }

        ConsoleGoToHome();
        draw(&player, apples);

        f = tick(60, f);
    }
    return 0;
}
