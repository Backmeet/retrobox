#include "utils/utils.c"
#include "utils/vec.c"
#include <stdio.h>
#include <stdlib.h>

#define W 10
#define H 20

typedef enum {
    EMPTY  = 232,
    RED    = 196,
    GREEN  = 46,
    BLUE   = 21,
    ORANGE = 208,
    CYAN   = 51,
    PURPLE = 93,
    YELLOW = 226,
    GHOST  = 240
} Colour;

Colour randColour() {
    static const Colour Colours[] = {
        RED,
        GREEN,
        BLUE,
        ORANGE,
        CYAN,
        PURPLE,
        YELLOW
    };

    return Colours[randint(0, 6)];
}

typedef struct {
    uint16_t rotations[4];
    char name;
} Tetromino;

typedef struct {
    Tetromino* type;
    Colour colour;
    int rot;
    ivec2 pos;
} Piece;

static const Tetromino PiecesDef[] = {
    {
        {
            0b0100010001100000,
            0b0010111000000000,
            0b1100010001000000,
            0b0000111010000000
        },
        'L'
    },
    {
        {
            0b0100010011000000,
            0b0000111000100000,
            0b0110010001000000,
            0b1000111000000000
        },
        'J'
    },
    {
        {
            0b0000011001100000,
            0b0000011001100000,
            0b0000011001100000,
            0b0000011001100000
        },
        'O'
    },
    {
        {
            0b0100111000000000,
            0b0100110001000000,
            0b0000111001000000,
            0b0100011001000000
        },
        'T'
    },
    {
        {
            0b0010011001000000,
            0b1100011000000000,
            0b0010011001000000,
            0b1100011000000000
        },
        'S'
    },
    {
        {
            0b0100011000100000,
            0b0110110000000000,
            0b0100011000100000,
            0b0110110000000000
        },
        'Z'
    },
    {
        {
            0b0000111100000000,
            0b0100010001000100,
            0b0000111100000000,
            0b0100010001000100
        },
        'I'
    }
};

static Colour Grid[W][H];

static int Bag[7];
static int BagIndex = 7;

static Piece Current;
static Tetromino* Next;
static Colour NextColour;

static int Score = 0;
static int Lines = 0;
static int Combo = 0;
static bool GameOver = false;

void shuffleBag() {
    for (int i = 0; i != 7; i++) {
        Bag[i] = i;
    }

    for (int i = 0; i != 7; i++) {
        int j = randint(0, 6);

        int t = Bag[i];
        Bag[i] = Bag[j];
        Bag[j] = t;
    }

    BagIndex = 0;
}

Tetromino* nextFromBag() {
    if (BagIndex >= 7) {
        shuffleBag();
    }

    return (Tetromino*)&PiecesDef[Bag[BagIndex++]];
}

bool pieceCell(Piece* p, int x, int y) {
    return (p->type->rotations[p->rot] >> ((y << 2) + x)) & 1;
}

bool collides(Piece* p) {
    for (int y = 0; y != 4; y++) {
        for (int x = 0; x != 4; x++) {
            if (!pieceCell(p, x, y)) {
                continue;
            }

            int gx = p->pos.x + x;
            int gy = p->pos.y + y;

            if (gx < 0 || gx >= W || gy >= H) {
                return true;
            }

            if (gy >= 0 && Grid[gx][gy] != EMPTY) {
                return true;
            }
        }
    }

    return false;
}

void lockPiece() {
    for (int y = 0; y != 4; y++) {
        for (int x = 0; x != 4; x++) {
            if (!pieceCell(&Current, x, y)) {
                continue;
            }

            int gx = Current.pos.x + x;
            int gy = Current.pos.y + y;

            if (gy >= 0) {
                Grid[gx][gy] = Current.colour;
            }
        }
    }
}

void clearLines() {
    int cleared = 0;

    for (int y = H - 1; y >= 0; y--) {
        bool full = true;

        for (int x = 0; x != W; x++) {
            if (Grid[x][y] == EMPTY) {
                full = false;
                break;
            }
        }

        if (!full) {
            continue;
        }

        cleared++;

        for (int py = y; py > 0; py--) {
            for (int x = 0; x != W; x++) {
                Grid[x][py] = Grid[x][py - 1];
            }
        }

        for (int x = 0; x != W; x++) {
            Grid[x][0] = EMPTY;
        }

        y++;
    }

    if (cleared) {
        Combo++;

        static const int ScoreTable[] = {
            0,
            100,
            300,
            500,
            800
        };

        Score += ScoreTable[cleared] + (Combo * 50);
        Lines += cleared;
    } else {
        Combo = 0;
    }
}

void spawnPiece() {
    Current.type = Next;
    Current.rot = 0;
    Current.colour = randColour();
    Current.pos = (ivec2){3, -2};

    Next = nextFromBag();

    if (collides(&Current)) {
        GameOver = true;
    }
}

void placeAndSpawn() {
    lockPiece();
    clearLines();
    spawnPiece();
}

void movePiece(int dx) {
    Piece t = Current;
    t.pos.x += dx;

    if (!collides(&t)) {
        Current = t;
    }
}

void rotatePiece() {
    Piece t = Current;
    t.rot = (t.rot + 1) % 4;

    if (!collides(&t)) {
        Current = t;
        return;
    }

    t.pos.x--;

    if (!collides(&t)) {
        Current = t;
        return;
    }

    t.pos.x += 2;

    if (!collides(&t)) {
        Current = t;
    }
}

void gravity() {
    Piece t = Current;
    t.pos.y++;

    if (collides(&t)) {
        placeAndSpawn();
        return;
    }

    Current.pos.y++;
}

void drawMiniPiece(Tetromino* t, int ox, int oy, Colour colour) {
    for (int y = 0; y != 4; y++) {
        ConsoleMoveCursor(ox, oy + y);

        for (int x = 0; x != 4; x++) {
            if ((t->rotations[0] >> ((y << 2) + x)) & 1) {
                printf("\033[48;5;%dm  \033[0m", colour);
            } else {
                putchar(' ');
                putchar(' ');
            }
        }
    }
}

void draw() {
    Colour frame[W][H];

    for (int x = 0; x != W; x++) {
        for (int y = 0; y != H; y++) {
            frame[x][y] = Grid[x][y];
        }
    }

    Piece ghost = Current;

    while (1) {
        Piece t = ghost;
        t.pos.y++;

        if (collides(&t)) {
            break;
        }

        ghost.pos.y++;
    }

    for (int y = 0; y != 4; y++) {
        for (int x = 0; x != 4; x++) {
            if (!pieceCell(&ghost, x, y)) {
                continue;
            }

            int gx = ghost.pos.x + x;
            int gy = ghost.pos.y + y;

            if (gy >= 0 && frame[gx][gy] == EMPTY && gy < H) {
                frame[gx][gy] = GHOST;
            }
        }
    }

    for (int y = 0; y != 4; y++) {
        for (int x = 0; x != 4; x++) {
            if (!pieceCell(&Current, x, y)) {
                continue;
            }

            int gx = Current.pos.x + x;
            int gy = Current.pos.y + y;

            if (gy >= 0 && gy < H) {
                frame[gx][gy] = Current.colour;
            }
        }
    }

    for (int y = 0; y != H; y++) {
        for (int x = 0; x != W; x++) {
            if (frame[x][y] == EMPTY) {
                fputs("\033[48;5;232m  \033[0m", stdout);
            } else {
                printf("\033[48;5;%dm  \033[0m", frame[x][y]);
            }
        }

        switch (y) {
            case 1:
                puts("   NEXT");
            break;

            case 8:
                printf("   Score: %d\n", Score);
            break;

            case 10:
                printf("   Lines: %d\n", Lines);
            break;

            case 12:
                printf("   Combo: %d\n", Combo);
            break;

            case 15:
                puts(keyPressed('W') ? "    \033[30;47mW\033[0m" : "    W");
            break;

            case 16:
                fputs(keyPressed('A') ? "   \033[30;47mA\033[0m" : "   A", stdout);
                fputs(keyPressed('S') ? "\033[30;47mS\033[0m" : "S", stdout);
                puts(keyPressed('D') ? "\033[30;47mD\033[0m" : "D");
            break;

            default:
                putchar('\n');
            break;
        }
    }

    drawMiniPiece(Next, (W * 2) + 6, 3, NextColour);
}

int main() {
    seed(now_ms() ^ (uintptr_t)&main);

    ConsoleBufferingDisable();
    ConsoleHideCursor();
    ConsoleClear();

    for (int x = 0; x != W; x++) {
        for (int y = 0; y != H; y++) {
            Grid[x][y] = EMPTY;
        }
    }

    shuffleBag();

    Next = nextFromBag();
    NextColour = randColour();

    spawnPiece();

    int64_t gravityTick = now_ms();
    int64_t inputTick = now_ms();

    bool RotateHeld = false;

    while (!GameOver) {

        if ((now_ms() - inputTick) > 200) {
            inputTick = now_ms();

            if (keyPressed('A')) {
                movePiece(-1);
            }

            if (keyPressed('D')) {
                movePiece(1);
            }

            if (keyPressed('S')) {
                gravity();
            }

            if (keyPressed('W')) {
                rotatePiece();
            }
        }

        if ((now_ms() - gravityTick) > 1000) {
            gravityTick = now_ms();
            gravity();
        }

        ConsoleGoToHome();
        draw();
    }

    ConsoleGoToHome();

    puts("Game Over!");
    printf("Score: %d\n", Score);
    printf("Lines: %d\n", Lines);
    for (size_t i = 0; i != H-3; i++) putchar('\n');

    return 0;
}