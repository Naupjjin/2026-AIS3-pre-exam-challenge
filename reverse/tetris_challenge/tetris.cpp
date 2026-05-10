#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define RESET   "\033[0m"
#define RED     "\033[41m  \033[0m"
#define GREEN   "\033[42m  \033[0m"
#define YELLOW  "\033[43m  \033[0m"
#define BLUE    "\033[44m  \033[0m"
#define MAGENTA "\033[45m  \033[0m"
#define CYAN    "\033[46m  \033[0m"
#define WHITE   "\033[47m  \033[0m"
#define EMPTY   "  "

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

// ============== ROGUE BYTES ANTI-DISASSEMBLER ==============
#define ROGUE_BYTES_1() __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory")
#define ROGUE_BYTES_2() __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory")
#define ROGUE_BYTES_3() __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory")
#define ROGUE_BYTES_4() __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory")

// ============== JUNK CODE MACROS ==============
#define JUNK_CODE_BLOCK_1() do { \
    volatile int _jk1 = 0x1337; \
    volatile int _jk2 = _jk1 ^ 0xDEAD; \
    (void)_jk2; \
} while(0)
#define JUNK_CODE_BLOCK_2() do { \
    volatile int _jk3 = __LINE__; \
    volatile int _jk4 = _jk3 * 0xBEEF; \
    (void)_jk4; \
} while(0)
#define JUNK_CODE_BLOCK_3() do { \
    volatile int _jk5 = 0xCAFE; \
    volatile int _jk6 = _jk5 - 0xBABE; \
    (void)_jk6; \
} while(0)
#define JUNK_CODE_BLOCK_4() do { \
    volatile int _jk7 = 0xFACE; \
    volatile int _jk8 = _jk7 + 0xFEED; \
    (void)_jk8; \
} while(0)

// Combined junk + rogue bytes
#define OBFUSCATE() \
    ROGUE_BYTES_1(); \
    JUNK_CODE_BLOCK_1(); \
    ROGUE_BYTES_2(); \
    JUNK_CODE_BLOCK_2(); \
    ROGUE_BYTES_3(); \
    JUNK_CODE_BLOCK_3(); \
    ROGUE_BYTES_4(); \
    JUNK_CODE_BLOCK_4()

// Tetromino shapes
const int TETROMINOS[7][4][4] = {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,2,2,0}, {0,2,2,0}, {0,0,0,0}},
    {{0,0,0,0}, {3,3,3,0}, {0,3,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,4,4,0}, {4,4,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {5,5,0,0}, {0,5,5,0}, {0,0,0,0}},
    {{0,0,0,0}, {6,6,6,0}, {0,0,6,0}, {0,0,0,0}},
    {{0,0,0,0}, {7,7,7,0}, {7,0,0,0}, {0,0,0,0}}
};

// Global game state
int board[BOARD_HEIGHT][BOARD_WIDTH];
int currentPiece[4][4];
int currentX, currentY;
int currentType;
int score = 0;
int linesCleared = 0;

// Encrypted flag
unsigned char encFlag[] = {
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64, 0x0
};

// Winning pattern
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}
};

// Terminal stuff
struct termios origTermios;

void disableRawMode() {
    OBFUSCATE();
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

void enableRawMode() {
    OBFUSCATE();
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int kbhit() {
    ROGUE_BYTES_1();
    int bytesWaiting;
    ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
    return bytesWaiting > 0;
}

void clearScreen() {
    ROGUE_BYTES_2();
    printf("\033[2J\033[H");
}

// ============== REALISTIC JUNK FUNCTIONS ==============
// These look like real game functions to confuse reverse engineers

#define JUNK_FUNC(n) \
    __attribute__((noinline)) void state_handler_##n(void) { \
        __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory"); \
        volatile int boardX = (__LINE__ * 0x1337) % BOARD_WIDTH; \
        volatile int boardY = (boardX ^ 0xDEAD) % BOARD_HEIGHT; \
        volatile int pieceType = (boardY + __LINE__) % 7; \
        volatile int collision = (pieceType * boardX) & 0xFF; \
        volatile int dropSpeed = collision ^ (boardY << 2); \
        volatile int lineScore = dropSpeed + (pieceType * 100); \
        (void)lineScore; \
    }

// Macro expansion helpers for generating 100000 junk functions
#define JUNK_10(base) \
    JUNK_FUNC(base##0) JUNK_FUNC(base##1) JUNK_FUNC(base##2) JUNK_FUNC(base##3) JUNK_FUNC(base##4) \
    JUNK_FUNC(base##5) JUNK_FUNC(base##6) JUNK_FUNC(base##7) JUNK_FUNC(base##8) JUNK_FUNC(base##9)

#define JUNK_100(base) \
    JUNK_10(base##0) JUNK_10(base##1) JUNK_10(base##2) JUNK_10(base##3) JUNK_10(base##4) \
    JUNK_10(base##5) JUNK_10(base##6) JUNK_10(base##7) JUNK_10(base##8) JUNK_10(base##9)

#define JUNK_1000(base) \
    JUNK_100(base##0) JUNK_100(base##1) JUNK_100(base##2) JUNK_100(base##3) JUNK_100(base##4) \
    JUNK_100(base##5) JUNK_100(base##6) JUNK_100(base##7) JUNK_100(base##8) JUNK_100(base##9)

#define JUNK_10000(base) \
    JUNK_1000(base##0) JUNK_1000(base##1) JUNK_1000(base##2) JUNK_1000(base##3) JUNK_1000(base##4) \
    JUNK_1000(base##5) JUNK_1000(base##6) JUNK_1000(base##7) JUNK_1000(base##8) JUNK_1000(base##9)

// Generate 100000 junk functions using non-zero prefix to avoid octal issues
JUNK_10000(1)
JUNK_10000(2)
JUNK_10000(3)
JUNK_10000(4)
JUNK_10000(5)
JUNK_10000(6)
JUNK_10000(7)
JUNK_10000(8)
JUNK_10000(9)
JUNK_10000(10)

// ============== 100000-ENTRY JUMP TABLE FOR CONTROL FLOW FLATTENING ==============
typedef void (*state_func_t)(void);
void state_dummy(void);
void state_draw_init(void);
void state_draw_header(void);
void state_draw_board(void);
void state_draw_score(void);
void state_spawn_init(void);
void state_spawn_copy(void);
void state_spawn_pos(void);
void state_collision_init(void);
void state_collision_check(void);
void state_collision_done(void);
void state_clear_init(void);
void state_clear_check(void);
void state_clear_shift(void);
void state_clear_done(void);
void state_pattern_init(void);
void state_pattern_check(void);
void state_pattern_decrypt(void);
void state_pattern_show(void);
void state_pattern_fail(void);
void state_game_init(void);
void state_game_loop(void);
void state_game_input(void);
void state_game_process(void);
void state_game_drop(void);
void state_game_lock(void);
void state_game_over(void);

// Global state machine variables
volatile int gState = 0;
volatile int gReturnVal = 0;
volatile int gLoopVar1 = 0;
volatile int gLoopVar2 = 0;
volatile int gMatch = 0;
volatile int gGameOver = 0;
volatile int gDropCounter = 0;
volatile int gInput = 0;
volatile int gCleared = 0;
volatile int gFull = 0;
volatile int collisionOffsetX = 0;
volatile int collisionOffsetY = 0;
volatile char gKey[32] = {0};
volatile char gFlag[64] = {0};

// The 100000-entry jump table for control flow flattening obfuscation
state_func_t jump_table[110000];

// Macros to populate jump table with realistic-looking junk functions
#define JUNK_TABLE_10(base) \
    jump_table[base##0] = state_handler_##base##0; \
    jump_table[base##1] = state_handler_##base##1; \
    jump_table[base##2] = state_handler_##base##2; \
    jump_table[base##3] = state_handler_##base##3; \
    jump_table[base##4] = state_handler_##base##4; \
    jump_table[base##5] = state_handler_##base##5; \
    jump_table[base##6] = state_handler_##base##6; \
    jump_table[base##7] = state_handler_##base##7; \
    jump_table[base##8] = state_handler_##base##8; \
    jump_table[base##9] = state_handler_##base##9;

#define JUNK_TABLE_100(base) \
    JUNK_TABLE_10(base##0) JUNK_TABLE_10(base##1) JUNK_TABLE_10(base##2) JUNK_TABLE_10(base##3) JUNK_TABLE_10(base##4) \
    JUNK_TABLE_10(base##5) JUNK_TABLE_10(base##6) JUNK_TABLE_10(base##7) JUNK_TABLE_10(base##8) JUNK_TABLE_10(base##9)

#define JUNK_TABLE_1000(base) \
    JUNK_TABLE_100(base##0) JUNK_TABLE_100(base##1) JUNK_TABLE_100(base##2) JUNK_TABLE_100(base##3) JUNK_TABLE_100(base##4) \
    JUNK_TABLE_100(base##5) JUNK_TABLE_100(base##6) JUNK_TABLE_100(base##7) JUNK_TABLE_100(base##8) JUNK_TABLE_100(base##9)

#define JUNK_TABLE_10000(base) \
    JUNK_TABLE_1000(base##0) JUNK_TABLE_1000(base##1) JUNK_TABLE_1000(base##2) JUNK_TABLE_1000(base##3) JUNK_TABLE_1000(base##4) \
    JUNK_TABLE_1000(base##5) JUNK_TABLE_1000(base##6) JUNK_TABLE_1000(base##7) JUNK_TABLE_1000(base##8) JUNK_TABLE_1000(base##9)

void init_jump_table() {
    // Initialize all entries to NULL first
    for (int i = 0; i < 110000; i++) {
        jump_table[i] = NULL;
    }
    
    // Fill entries 10000-109999 with realistic junk functions
    JUNK_TABLE_10000(1)
    JUNK_TABLE_10000(2)
    JUNK_TABLE_10000(3)
    JUNK_TABLE_10000(4)
    JUNK_TABLE_10000(5)
    JUNK_TABLE_10000(6)
    JUNK_TABLE_10000(7)
    JUNK_TABLE_10000(8)
    JUNK_TABLE_10000(9)
    JUNK_TABLE_10000(10)
    
    // Scatter meaningful state functions among junk entries
    // This makes it much harder to identify real functions
    jump_table[13579] = state_draw_init;        // 0 -> 13579
    jump_table[24680] = state_draw_header;      // 1 -> 24680
    jump_table[35791] = state_draw_board;       // 2 -> 35791
    jump_table[46802] = state_draw_score;       // 3 -> 46802
    jump_table[57913] = state_spawn_init;       // 4 -> 57913
    jump_table[68024] = state_spawn_copy;       // 5 -> 68024
    jump_table[79135] = state_spawn_pos;        // 6 -> 79135
    jump_table[80246] = state_collision_init;   // 7 -> 80246
    jump_table[91357] = state_collision_check;  // 8 -> 91357
    jump_table[12468] = state_collision_done;   // 9 -> 12468
    jump_table[23571] = state_clear_init;       // 10 -> 23571
    jump_table[34682] = state_clear_check;      // 11 -> 34682
    jump_table[45793] = state_clear_shift;      // 12 -> 45793
    jump_table[56804] = state_clear_done;       // 13 -> 56804
    jump_table[67915] = state_pattern_init;     // 14 -> 67915
    jump_table[78026] = state_pattern_check;    // 15 -> 78026
    jump_table[89137] = state_pattern_decrypt;  // 16 -> 89137
    jump_table[90248] = state_pattern_show;     // 17 -> 90248
    jump_table[11359] = state_pattern_fail;     // 18 -> 11359
    jump_table[22460] = state_game_init;        // 19 -> 22460
    jump_table[33571] = state_game_loop;        // 20 -> 33571
    jump_table[44682] = state_game_input;       // 21 -> 44682
    jump_table[55793] = state_game_process;     // 22 -> 55793
    jump_table[66804] = state_game_drop;        // 23 -> 66804
    jump_table[77915] = state_game_lock;        // 24 -> 77915
    jump_table[88026] = state_game_over;        // 25 -> 88026
}

// State index mapping (old -> new scattered indices)
#define ST_DRAW_INIT       13579
#define ST_DRAW_HEADER     24680
#define ST_DRAW_BOARD      35791
#define ST_DRAW_SCORE      46802
#define ST_SPAWN_INIT      57913
#define ST_SPAWN_COPY      68024
#define ST_SPAWN_POS       79135
#define ST_COLLISION_INIT  80246
#define ST_COLLISION_CHECK 91357
#define ST_COLLISION_DONE  12468
#define ST_CLEAR_INIT      23571
#define ST_CLEAR_CHECK     34682
#define ST_CLEAR_SHIFT     45793
#define ST_CLEAR_DONE      56804
#define ST_PATTERN_INIT    67915
#define ST_PATTERN_CHECK   78026
#define ST_PATTERN_DECRYPT 89137
#define ST_PATTERN_SHOW    90248
#define ST_PATTERN_FAIL    11359
#define ST_GAME_INIT       22460
#define ST_GAME_LOOP       33571
#define ST_GAME_INPUT      44682
#define ST_GAME_PROCESS    55793
#define ST_GAME_DROP       66804
#define ST_GAME_LOCK       77915
#define ST_GAME_OVER       88026
#define ST_END             99999

void state_dummy(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    JUNK_CODE_BLOCK_1();
    gState = ST_END;
}

// Key derivation using FNV-1a
void deriveKeyFromPattern(const int pattern[4][BOARD_WIDTH], unsigned char* key) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    unsigned int hash = 0x811c9dc5;
    JUNK_CODE_BLOCK_2();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            ROGUE_BYTES_1();
            hash ^= pattern[i][j];
            JUNK_CODE_BLOCK_3();
            hash *= 0x01000193;
            ROGUE_BYTES_2();
        }
    }
    JUNK_CODE_BLOCK_4();
    for (int i = 0; i < 24; i++) {
        ROGUE_BYTES_3();
        key[i] = (hash >> ((i % 4) * 8)) & 0xFF;
        JUNK_CODE_BLOCK_1();
        hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF;
        ROGUE_BYTES_4();
    }
    OBFUSCATE();
}

// RC4 decryption
void rc4Decrypt(unsigned char* data, int len, unsigned char* key, int keyLen) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    unsigned char S[256];
    JUNK_CODE_BLOCK_2();
    for (int i = 0; i < 256; i++) {
        ROGUE_BYTES_1();
        S[i] = i;
    }
    JUNK_CODE_BLOCK_3();
    int j = 0;
    for (int i = 0; i < 256; i++) {
        ROGUE_BYTES_2();
        j = (j + S[i] + key[i % keyLen]) % 256;
        JUNK_CODE_BLOCK_4();
        unsigned char tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
        ROGUE_BYTES_3();
    }
    JUNK_CODE_BLOCK_1();
    int i = 0;
    j = 0;
    for (int k = 0; k < len; k++) {
        ROGUE_BYTES_4();
        i = (i + 1) % 256;
        JUNK_CODE_BLOCK_2();
        j = (j + S[i]) % 256;
        unsigned char tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
        ROGUE_BYTES_1();
        data[k] ^= S[(S[i] + S[j]) % 256];
        JUNK_CODE_BLOCK_3();
    }
    OBFUSCATE();
}

// State implementations
void state_draw_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    clearScreen();
    gState = ST_DRAW_HEADER;
}

void state_draw_header(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    printf("\033[1;36mTETRIS - Score: %d | Lines: %d\033[0m\n", score, linesCleared);
    printf("Controls: WASD/Arrows, Q=Rotate, ESC=Quit\n\n");
    gState = ST_DRAW_BOARD;
}

void state_draw_board(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    const char* colors[] = {EMPTY, CYAN, YELLOW, MAGENTA, GREEN, RED, BLUE, WHITE};
    int tempBoard[BOARD_HEIGHT][BOARD_WIDTH];
    memcpy(tempBoard, board, sizeof(board));
    JUNK_CODE_BLOCK_1();
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ROGUE_BYTES_1();
            if (currentPiece[i][j] != 0) {
                int boardY = currentY + i;
                int boardX = currentX + j;
                if (boardY >= 0 && boardY < BOARD_HEIGHT && boardX >= 0 && boardX < BOARD_WIDTH) {
                    tempBoard[boardY][boardX] = currentPiece[i][j];
                }
            }
        }
    }
    JUNK_CODE_BLOCK_2();
    
    printf("╔");
    for (int i = 0; i < BOARD_WIDTH; i++) printf("══");
    printf("╗\n");
    
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        ROGUE_BYTES_2();
        printf("║");
        for (int j = 0; j < BOARD_WIDTH; j++) {
            int c = tempBoard[i][j];
            if (c >= 0 && c <= 7) printf("%s", colors[c]);
            else printf("%s", EMPTY);
        }
        printf("║\n");
    }
    
    printf("╚");
    for (int i = 0; i < BOARD_WIDTH; i++) printf("══");
    printf("╝\n");
    
    gState = ST_DRAW_SCORE;
}

void state_draw_score(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gState = ST_END;
}

void state_spawn_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    currentType = rand() % 7;
    gLoopVar1 = 0;
    gState = ST_SPAWN_COPY;
}

void state_spawn_copy(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    if (gLoopVar1 >= 4) {
        gState = ST_SPAWN_POS;
        return;
    }
    JUNK_CODE_BLOCK_1();
    for (int j = 0; j < 4; j++) {
        ROGUE_BYTES_1();
        currentPiece[gLoopVar1][j] = TETROMINOS[currentType][gLoopVar1][j];
    }
    gLoopVar1++;
    gState = ST_SPAWN_COPY;  
}

void state_spawn_pos(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    currentX = BOARD_WIDTH / 2 - 2;
    currentY = 0;
    gState = ST_END;
}

void state_collision_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gLoopVar1 = 0;
    gReturnVal = 0;
    gState = ST_COLLISION_CHECK;
}

void state_collision_check(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    if (gLoopVar1 >= 4) {
        gState = ST_COLLISION_DONE;
        return;
    }
    JUNK_CODE_BLOCK_2();
    for (int j = 0; j < 4; j++) {
        ROGUE_BYTES_2();
        if (currentPiece[gLoopVar1][j] != 0) {
            int newX = currentX + j + collisionOffsetX;
            int newY = currentY + gLoopVar1 + collisionOffsetY;
            JUNK_CODE_BLOCK_3();
            if (newX < 0 || newX >= BOARD_WIDTH || newY >= BOARD_HEIGHT) {
                gReturnVal = 1;
                gState = ST_COLLISION_DONE;
                return;
            }
            ROGUE_BYTES_3();
            if (newY >= 0 && board[newY][newX] != 0) {
                gReturnVal = 1;
                gState = ST_COLLISION_DONE;
                return;
            }
        }
    }
    gLoopVar1++;
    gState = ST_COLLISION_CHECK;  
}

void state_collision_done(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gState = ST_END;
}

void state_clear_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gLoopVar1 = BOARD_HEIGHT - 1;
    gCleared = 0;
    gState = ST_CLEAR_CHECK;
}

void state_clear_check(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    if (gLoopVar1 < 0) {
        gState = ST_CLEAR_DONE;
        return;
    }
    JUNK_CODE_BLOCK_1();
    gFull = 1;
    for (int j = 0; j < BOARD_WIDTH; j++) {
        ROGUE_BYTES_1();
        if (board[gLoopVar1][j] == 0) {
            gFull = 0;
            break;
        }
    }
    JUNK_CODE_BLOCK_2();
    if (gFull) {
        gState = ST_CLEAR_SHIFT;
    } else {
        gLoopVar1--;
        gState = ST_CLEAR_CHECK;
    }
}

void state_clear_shift(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    for (int k = gLoopVar1; k > 0; k--) {
        ROGUE_BYTES_2();
        for (int l = 0; l < BOARD_WIDTH; l++) {
            board[k][l] = board[k-1][l];
        }
    }
    JUNK_CODE_BLOCK_3();
    for (int l = 0; l < BOARD_WIDTH; l++) {
        board[0][l] = 0;
    }
    gCleared++;
    gLoopVar1--;
    gState = ST_CLEAR_CHECK;
}

void state_clear_done(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    linesCleared += gCleared;
    score += gCleared * 100;
    gState = ST_END;
}

void state_pattern_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    JUNK_CODE_BLOCK_4();
    gLoopVar1 = 0;
    gMatch = 1;
    gState = ST_PATTERN_CHECK;
}

void state_pattern_check(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    if (gLoopVar1 >= 4) {
        if (gMatch) {
            gState = ST_PATTERN_DECRYPT;
        } else {
            gState = ST_PATTERN_FAIL;
        }
        return;
    }
    JUNK_CODE_BLOCK_1();
    for (int j = 0; j < BOARD_WIDTH; j++) {
        ROGUE_BYTES_3();
        if (board[gLoopVar1][j] != WINNING_PATTERN[gLoopVar1][j]) {
            gMatch = 0;
        }
    }
    gLoopVar1++;
    gState = ST_PATTERN_CHECK;  
}

void state_pattern_decrypt(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    deriveKeyFromPattern(WINNING_PATTERN, (unsigned char*)gKey);
    JUNK_CODE_BLOCK_2();
    memcpy((void*)gFlag, encFlag, 28);
    gState = ST_PATTERN_SHOW;
}

void state_pattern_show(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    rc4Decrypt((unsigned char*)gFlag, 28, (unsigned char*)gKey, 24);
    gFlag[28] = '\0';
    JUNK_CODE_BLOCK_3();
    printf("\n\033%s\033[0m\n", gFlag);
    printf("Press any key to continue...\n");
    getchar();
    gState = ST_END;
}

void state_pattern_fail(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    printf("\n\033[1;33mexit\033[0m\n");
    usleep(1500000);
    gState = ST_END;
}

void state_game_init(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    srand(time(NULL));
    memset(board, 0, sizeof(board));
    score = 0;
    linesCleared = 0;
    gGameOver = 0;
    gDropCounter = 0;
    JUNK_CODE_BLOCK_4();
    
    // Spawn first piece
    gState = ST_SPAWN_INIT;
    while (gState != ST_END) {
        ROGUE_BYTES_4();
        jump_table[gState]();
    }
    gState = ST_GAME_LOOP;
}

void state_game_loop(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    if (gGameOver) {
        gState = ST_GAME_OVER;
        return;
    }
    JUNK_CODE_BLOCK_1();
    
    // Draw
    gState = ST_DRAW_INIT;
    while (gState != ST_END) {
        ROGUE_BYTES_1();
        jump_table[gState]();
    }
    gState = ST_GAME_INPUT;
}

void state_game_input(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gInput = 0;
    if (kbhit()) {
        JUNK_CODE_BLOCK_2();
        gInput = getchar();
        if (gInput == 27) {
            if (kbhit()) {
                ROGUE_BYTES_2();
                char seq1 = getchar();
                if (seq1 == '[' && kbhit()) {
                    char seq2 = getchar();
                    JUNK_CODE_BLOCK_3();
                    switch(seq2) {
                        case 'A': gInput = 'w'; break;
                        case 'B': gInput = 's'; break;
                        case 'C': gInput = 'd'; break;
                        case 'D': gInput = 'a'; break;
                    }
                }
            } else {
                gGameOver = 1;
                gState = ST_END;
                return;
            }
        }
    }
    gState = ST_GAME_PROCESS;
}

// Helper for collision check
int flatCheckCollision(int ox, int oy) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    ROGUE_BYTES_1();
    collisionOffsetX = ox;
    collisionOffsetY = oy;
    gState = ST_COLLISION_INIT;
    while (gState != ST_END) {
        JUNK_CODE_BLOCK_4();
        jump_table[gState]();
    }
    return gReturnVal;
}

void state_game_process(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    switch (gInput) {
        case 'a': case 'A':
            ROGUE_BYTES_1();
            if (!flatCheckCollision(-1, 0)) currentX--;
            break;
        case 'd': case 'D':
            ROGUE_BYTES_2();
            if (!flatCheckCollision(1, 0)) currentX++;
            break;
        case 's': case 'S':
            ROGUE_BYTES_3();
            if (!flatCheckCollision(0, 1)) { currentY++; score++; }
            break;
        case 'w': case 'W':
            JUNK_CODE_BLOCK_1();
            while (!flatCheckCollision(0, 1)) { currentY++; score += 2; }
            break;
        case 'q': case 'Q': {
            JUNK_CODE_BLOCK_2();
            int temp[4][4];
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    temp[j][3-i] = currentPiece[i][j];
            int backup[4][4];
            memcpy(backup, currentPiece, sizeof(currentPiece));
            memcpy(currentPiece, temp, sizeof(temp));
            ROGUE_BYTES_4();
            if (flatCheckCollision(0, 0)) memcpy(currentPiece, backup, sizeof(backup));
            break;
        }
        case 'p': case 'P':
            JUNK_CODE_BLOCK_3();
            gState = ST_PATTERN_INIT;
            while (gState != ST_END) {
                jump_table[gState]();
            }
            gState = ST_END;
            return;
    }
    gState = ST_GAME_DROP;
}

void state_game_drop(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gDropCounter++;
    if (gDropCounter >= 5) {
        JUNK_CODE_BLOCK_4();
        gDropCounter = 0;
        if (!flatCheckCollision(0, 1)) {
            currentY++;
        } else {
            gState = ST_GAME_LOCK;
            return;
        }
    }
    ROGUE_BYTES_1();
    usleep(50000);
    gState = ST_GAME_LOOP;
}

void state_game_lock(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ROGUE_BYTES_2();
            if (currentPiece[i][j] != 0) {
                int boardY = currentY + i;
                int boardX = currentX + j;
                JUNK_CODE_BLOCK_1();
                if (boardY >= 0 && boardY < BOARD_HEIGHT && boardX >= 0 && boardX < BOARD_WIDTH) {
                    board[boardY][boardX] = currentPiece[i][j];
                }
            }
        }
    }
    
    JUNK_CODE_BLOCK_2();
    // Clear lines
    gState = ST_CLEAR_INIT;
    while (gState != ST_END) {
        ROGUE_BYTES_3();
        jump_table[gState]();
    }
    
    // Spawn new piece
    gState = ST_SPAWN_INIT;
    while (gState != ST_END) {
        JUNK_CODE_BLOCK_3();
        jump_table[gState]();
    }
    
    ROGUE_BYTES_4();
    if (flatCheckCollision(0, 0)) {
        gGameOver = 1;
    }
    gState = ST_GAME_LOOP;
}

void state_game_over(void) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    disableRawMode();
    clearScreen();
    printf("Game Over! Final Score: %d\n", score);
    printf("Lines Cleared: %d\n", linesCleared);
    gState = ST_END;
}

void gameLoop() {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    gState = ST_GAME_INIT;
    jump_table[gState]();
    
    enableRawMode();
    
    while (!gGameOver) {
        JUNK_CODE_BLOCK_4();
        gState = ST_GAME_LOOP;
        while (gState != ST_END && !gGameOver) {
            ROGUE_BYTES_1();
            jump_table[gState]();
        }
    }
    
    state_game_over();
}

int main(int argc, char** argv) {
    __asm__ volatile(".byte 0xeb,0xff,0xc0" ::: "memory");
    OBFUSCATE();
    
    init_jump_table();
    
    printf("\033[1;36m");
    printf("╔════════════════════════════════════════╗\n");
    printf("║                TETRIS                  ║\n");
    printf("║                                        ║\n");
    printf("║  Controls:                             ║\n");
    printf("║  W/↑ - Hard Drop    A/← - Move Left    ║\n");
    printf("║  S/↓ - Soft Drop    D/→ - Move Right   ║\n");
    printf("║  Q   - Rotate                          ║\n");
    printf("║  ESC - Quit                            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\033[0m");
    printf("\nPress ENTER to start...");
    getchar();
    
    JUNK_CODE_BLOCK_4();
    ROGUE_BYTES_4();
    
    gameLoop();
    
    return 0;
}
