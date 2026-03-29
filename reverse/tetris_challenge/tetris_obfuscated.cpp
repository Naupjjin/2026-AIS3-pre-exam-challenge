// Tetris Reverse Challenge - AIS3 Pre-Exam
// Obfuscated with: Control Flow Flattening (Big Jump Table), Anti-Disassembler, Anti-Debug

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <sys/ptrace.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// Color codes for terminal
#define RESET   "\033[0m"
#define RED     "\033[41m  \033[0m"
#define GREEN   "\033[42m  \033[0m"
#define YELLOW  "\033[43m  \033[0m"
#define BLUE    "\033[44m  \033[0m"
#define MAGENTA "\033[45m  \033[0m"
#define CYAN    "\033[46m  \033[0m"
#define WHITE   "\033[47m  \033[0m"
#define EMPTY   "  "

// Board dimensions
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

// Anti-disassembler macros - insert fake conditional jumps
// These use opaque predicates that always evaluate to true/false but confuse disassemblers
#define ANTI_DISASM_1() \
    __asm__ __volatile__ ( \
        "xor %%eax, %%eax\n\t" \
        "jz 1f\n\t" \
        ".byte 0xe8\n\t" \
        "1:\n\t" \
        : : : "eax" \
    )

#define ANTI_DISASM_2() \
    __asm__ __volatile__ ( \
        "push %%rax\n\t" \
        "xor %%eax, %%eax\n\t" \
        "test %%eax, %%eax\n\t" \
        "jnz 1f\n\t" \
        "jmp 2f\n\t" \
        "1:\n\t" \
        ".byte 0xff, 0x25\n\t" \
        "2:\n\t" \
        "pop %%rax\n\t" \
        : : : "rax" \
    )

#define ANTI_DISASM_3() \
    __asm__ __volatile__ ( \
        "call 1f\n\t" \
        ".byte 0x48\n\t" \
        "1: add $1, (%%rsp)\n\t" \
        "ret\n\t" \
        : : : \
    )

// Fake call instruction that's never executed
#define ANTI_DISASM_FAKE_CALL() \
    __asm__ __volatile__ ( \
        "xor %%eax, %%eax\n\t" \
        "test %%eax, %%eax\n\t" \
        "jz 1f\n\t" \
        ".byte 0xe8, 0x00, 0x00, 0x00, 0x00\n\t" \
        "1:\n\t" \
        : : : "eax" \
    )

// Overlapping instruction trick
#define ANTI_DISASM_OVERLAP() \
    __asm__ __volatile__ ( \
        "jmp 1f\n\t" \
        ".byte 0x48, 0xb8\n\t" \
        "1:\n\t" \
        : : : \
    )

// Create confusion with function pointer-like bytes
#define ANTI_DISASM_PTR() \
    __asm__ __volatile__ ( \
        "push %%rax\n\t" \
        "mov $0, %%eax\n\t" \
        "test %%eax, %%eax\n\t" \
        "jnz 1f\n\t" \
        "jmp 2f\n\t" \
        "1:\n\t" \
        ".byte 0x48, 0x8b, 0x05, 0x00, 0x00, 0x00, 0x00\n\t" \
        ".byte 0xff, 0xd0\n\t" \
        "2:\n\t" \
        "pop %%rax\n\t" \
        : : : "rax" \
    )

// Tetromino shapes (I, O, T, S, Z, J, L)
const int TETROMINOS[7][4][4] = {
    // I - Cyan (1)
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    // O - Yellow (2)
    {{0,0,0,0}, {0,2,2,0}, {0,2,2,0}, {0,0,0,0}},
    // T - Magenta (3)
    {{0,0,0,0}, {3,3,3,0}, {0,3,0,0}, {0,0,0,0}},
    // S - Green (4)
    {{0,0,0,0}, {0,4,4,0}, {4,4,0,0}, {0,0,0,0}},
    // Z - Red (5)
    {{0,0,0,0}, {5,5,0,0}, {0,5,5,0}, {0,0,0,0}},
    // J - Blue (6)
    {{0,0,0,0}, {6,6,6,0}, {0,0,6,0}, {0,0,0,0}},
    // L - White/Orange (7)
    {{0,0,0,0}, {7,7,7,0}, {7,0,0,0}, {0,0,0,0}}
};

// Global game state
int board[BOARD_HEIGHT][BOARD_WIDTH];
int currentPiece[4][4];
int currentX, currentY;
int currentType;
int score = 0;
int linesCleared = 0;
volatile int debugDetected = 0;

// Encrypted flag (will be decrypted with correct pattern)
unsigned char encFlag[] = {
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64, 0x0
};

// The winning pattern (specific block arrangement)
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}
};

// Junk variables (obfuscation)
volatile int junk1 = 0xDEADBEEF;
volatile int junk2 = 0xCAFEBABE;
volatile long junk3 = 0xBAADF00D;
volatile char junkArr[64] = {0};

// Big jump table for control flow flattening
// 256 entries to confuse analysis
typedef void (*dispatch_func_t)(void);
volatile int dispatchState = 0;
volatile int dispatchNext = 0;
volatile int dispatchTemp = 0;

// Forward declarations
static void dispatchHandler();

// Anti-debug: Check /proc/self/status for TracerPid
int __attribute__((always_inline)) checkTracerPid() {
    ANTI_DISASM_1();
    char buf[4096];
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) return 1;
    
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return 1;
    buf[n] = '\0';
    
    char* tracer = strstr(buf, "TracerPid:");
    if (tracer) {
        tracer += 10;
        while (*tracer == ' ' || *tracer == '\t') tracer++;
        if (*tracer != '0' || (*(tracer+1) >= '0' && *(tracer+1) <= '9')) {
            return 1;
        }
    }
    ANTI_DISASM_FAKE_CALL();
    return 0;
}

// Anti-debug: SIGTRAP handler
void sigtrapHandler(int sig) {
    debugDetected = 1;
}

// Anti-debug: Timing check
volatile unsigned long startTime = 0;
int __attribute__((noinline)) timingCheck() {
    ANTI_DISASM_2();
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long current = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    if (startTime == 0) {
        startTime = current;
        return 0;
    }
    if (current - startTime > 5000000) {
        return 1;
    }
    startTime = current;
    return 0;
}

// Junk function 1 (never called with true condition)
void __attribute__((noinline)) junkFunc1() {
    ANTI_DISASM_OVERLAP();
    for (int i = 0; i < 1000; i++) {
        junk1 ^= (i * 0x1337);
        junk2 += junk1;
        junkArr[i % 64] = (char)(junk1 ^ junk2);
    }
    if (junk1 == 0) {
        exit(1);
    }
}

// Junk function 2
int __attribute__((noinline)) junkFunc2(int x) {
    ANTI_DISASM_PTR();
    int result = x;
    for (int i = 0; i < 50; i++) {
        result = ((result << 3) | (result >> 29)) ^ 0xABCDEF;
        result += junk2;
        result ^= junk3;
    }
    return result;
}

// Junk function 3 - fake decryption
void __attribute__((noinline)) junkDecrypt(unsigned char* data, int len) {
    ANTI_DISASM_1();
    for (int i = 0; i < len; i++) {
        data[i] ^= (unsigned char)(junk1 >> (i % 4) * 8);
        data[i] = (data[i] << 3) | (data[i] >> 5);
    }
}

// Real key derivation from pattern
void __attribute__((noinline)) deriveKeyFromPattern(const int pattern[4][BOARD_WIDTH], unsigned char* key) {
    ANTI_DISASM_2();
    
    if (junk1 == 0xFFFFFFFF) junkFunc1();
    
    unsigned int hash = 0x811c9dc5;  // FNV-1a
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            hash ^= pattern[i][j];
            hash *= 0x01000193;
            junk2 ^= hash;
        }
    }
    
    ANTI_DISASM_FAKE_CALL();
    
    // Generate 24-byte key from hash
    for (int i = 0; i < 24; i++) {
        key[i] = (unsigned char)((hash >> ((i % 4) * 8)) & 0xFF);
        hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

// Real decryption function (RC4-like)
void __attribute__((noinline)) realDecrypt(unsigned char* data, int len, unsigned char* key, int keyLen) {
    ANTI_DISASM_OVERLAP();
    
    unsigned char S[256];
    int i, j = 0;
    
    // KSA
    for (i = 0; i < 256; i++) S[i] = i;
    for (i = 0; i < 256; i++) {
        j = (j + S[i] + key[i % keyLen]) % 256;
        unsigned char tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
    }
    
    ANTI_DISASM_1();
    
    // PRGA
    i = j = 0;
    for (int k = 0; k < len; k++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        unsigned char tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
        data[k] ^= S[(S[i] + S[j]) % 256];
    }
}

// Terminal handling
struct termios origTermios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
    printf("\033[?25h");
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &origTermios);
    atexit(disableRawMode);
    
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\033[?25l");
}

int kbhit() {
    int byteswaiting;
    ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

// Print colored cell
void printCell(int color) {
    switch(color) {
        case 0: printf(EMPTY); break;
        case 1: printf(CYAN); break;
        case 2: printf(YELLOW); break;
        case 3: printf(MAGENTA); break;
        case 4: printf(GREEN); break;
        case 5: printf(RED); break;
        case 6: printf(BLUE); break;
        case 7: printf(WHITE); break;
        default: printf(EMPTY); break;
    }
}

void clearScreen() {
    printf("\033[2J\033[H");
}

// ============================================================================
// BIG JUMP TABLE BASED CONTROL FLOW FLATTENING
// ============================================================================

// Global state machine variables for the flattened control flow
volatile int gState = 0;
volatile int gSubState = 0;
volatile int gLoopVar1 = 0;
volatile int gLoopVar2 = 0;
volatile int gReturnVal = 0;
volatile int gTempBoard[BOARD_HEIGHT][BOARD_WIDTH];
volatile int gMatch = 0;
volatile int gFull = 0;
volatile int gCleared = 0;
volatile int gGameOver = 0;
volatile char gInput = 0;
volatile int gDropCounter = 0;
volatile int gPieceType = 0;
volatile unsigned char gKey[24];
volatile unsigned char gFlag[32];

// Opaque predicate - always returns 0 but hard to analyze
__attribute__((noinline)) int opaqueZero(int x) {
    return ((x * x) % 2) - ((x * x) % 2);
}

// Opaque predicate - always returns 1
__attribute__((noinline)) int opaqueOne(int x) {
    return ((x | (~x)) & 1);
}

// Compute next state with obfuscation
__attribute__((noinline)) int computeNextState(int current, int offset) {
    ANTI_DISASM_PTR();
    int opaque = opaqueZero(current);
    return (current + offset + opaque) & 0xFF;
}

// The massive state machine with 256-entry jump table
// Each function handles one state transition
// State machine layout:
//   0-31:   drawBoard states
//   32-63:  spawnPiece states
//   64-95:  checkCollision states
//   96-127: clearLines states
//   128-159: checkPattern states
//   160-191: gameLoop states
//   192-223: reserved/junk states
//   224-255: junk/trap states

// State handlers array - big jump table
typedef void (*state_handler_t)(void);

// Forward declare all state handlers
static void state_init(void);
static void state_draw_init(void);
static void state_draw_copy(void);
static void state_draw_overlay_start(void);
static void state_draw_overlay_inner(void);
static void state_draw_overlay_cell(void);
static void state_draw_clear(void);
static void state_draw_header(void);
static void state_draw_row_start(void);
static void state_draw_cell(void);
static void state_draw_row_end(void);
static void state_draw_footer(void);
static void state_draw_done(void);
static void state_spawn_init(void);
static void state_spawn_copy_outer(void);
static void state_spawn_copy_inner(void);
static void state_spawn_copy_cell(void);
static void state_spawn_done(void);
static void state_collision_init(void);
static void state_collision_outer(void);
static void state_collision_inner(void);
static void state_collision_check(void);
static void state_collision_bounds(void);
static void state_collision_yes(void);
static void state_collision_no(void);
static void state_clear_init(void);
static void state_clear_row_check(void);
static void state_clear_cell_check(void);
static void state_clear_process(void);
static void state_clear_shift(void);
static void state_clear_done(void);
static void state_pattern_init(void);
static void state_pattern_outer(void);
static void state_pattern_inner(void);
static void state_pattern_compare(void);
static void state_pattern_result(void);
static void state_pattern_decrypt(void);
static void state_pattern_show(void);
static void state_pattern_fail(void);
static void state_pattern_done(void);
static void state_game_init(void);
static void state_game_check(void);
static void state_game_draw(void);
static void state_game_input(void);
static void state_game_process(void);
static void state_game_drop(void);
static void state_game_lock(void);
static void state_game_over(void);
static void state_junk1(void);
static void state_junk2(void);
static void state_junk3(void);
static void state_trap(void);

// The big 256-entry jump table
// Many entries are junk or duplicated to confuse analysis
static state_handler_t jump_table[256] = {
    // 0-15: Draw states
    state_draw_init,        // 0
    state_draw_copy,        // 1
    state_draw_overlay_start, // 2
    state_draw_overlay_inner, // 3
    state_draw_overlay_cell,  // 4
    state_draw_clear,       // 5
    state_draw_header,      // 6
    state_draw_row_start,   // 7
    state_draw_cell,        // 8
    state_draw_row_end,     // 9
    state_draw_footer,      // 10
    state_draw_done,        // 11
    state_junk1,            // 12
    state_junk2,            // 13
    state_junk3,            // 14
    state_trap,             // 15
    
    // 16-31: Spawn states
    state_spawn_init,       // 16
    state_spawn_copy_outer, // 17
    state_spawn_copy_inner, // 18
    state_spawn_copy_cell,  // 19
    state_spawn_done,       // 20
    state_junk1,            // 21
    state_junk2,            // 22
    state_junk3,            // 23
    state_trap,             // 24
    state_junk1,            // 25
    state_junk2,            // 26
    state_junk3,            // 27
    state_trap,             // 28
    state_junk1,            // 29
    state_junk2,            // 30
    state_trap,             // 31
    
    // 32-47: Collision states
    state_collision_init,   // 32
    state_collision_outer,  // 33
    state_collision_inner,  // 34
    state_collision_check,  // 35
    state_collision_bounds, // 36
    state_collision_yes,    // 37
    state_collision_no,     // 38
    state_junk1,            // 39
    state_junk2,            // 40
    state_junk3,            // 41
    state_trap,             // 42
    state_junk1,            // 43
    state_junk2,            // 44
    state_junk3,            // 45
    state_trap,             // 46
    state_junk1,            // 47
    
    // 48-63: Clear line states
    state_clear_init,       // 48
    state_clear_row_check,  // 49
    state_clear_cell_check, // 50
    state_clear_process,    // 51
    state_clear_shift,      // 52
    state_clear_done,       // 53
    state_junk1,            // 54
    state_junk2,            // 55
    state_junk3,            // 56
    state_trap,             // 57
    state_junk1,            // 58
    state_junk2,            // 59
    state_junk3,            // 60
    state_trap,             // 61
    state_junk1,            // 62
    state_trap,             // 63
    
    // 64-79: Pattern check states
    state_pattern_init,     // 64
    state_pattern_outer,    // 65
    state_pattern_inner,    // 66
    state_pattern_compare,  // 67
    state_pattern_result,   // 68
    state_pattern_decrypt,  // 69
    state_pattern_show,     // 70
    state_pattern_fail,     // 71
    state_pattern_done,     // 72
    state_junk1,            // 73
    state_junk2,            // 74
    state_junk3,            // 75
    state_trap,             // 76
    state_junk1,            // 77
    state_junk2,            // 78
    state_trap,             // 79
    
    // 80-95: Game loop states
    state_game_init,        // 80
    state_game_check,       // 81
    state_game_draw,        // 82
    state_game_input,       // 83
    state_game_process,     // 84
    state_game_drop,        // 85
    state_game_lock,        // 86
    state_game_over,        // 87
    state_junk1,            // 88
    state_junk2,            // 89
    state_junk3,            // 90
    state_trap,             // 91
    state_junk1,            // 92
    state_junk2,            // 93
    state_junk3,            // 94
    state_trap,             // 95
    
    // 96-255: Junk and trap states to confuse disassemblers
    state_junk1, state_junk2, state_junk3, state_trap,  // 96-99
    state_junk1, state_junk2, state_junk3, state_trap,  // 100-103
    state_junk1, state_junk2, state_junk3, state_trap,  // 104-107
    state_junk1, state_junk2, state_junk3, state_trap,  // 108-111
    state_junk1, state_junk2, state_junk3, state_trap,  // 112-115
    state_junk1, state_junk2, state_junk3, state_trap,  // 116-119
    state_junk1, state_junk2, state_junk3, state_trap,  // 120-123
    state_junk1, state_junk2, state_junk3, state_trap,  // 124-127
    state_junk1, state_junk2, state_junk3, state_trap,  // 128-131
    state_junk1, state_junk2, state_junk3, state_trap,  // 132-135
    state_junk1, state_junk2, state_junk3, state_trap,  // 136-139
    state_junk1, state_junk2, state_junk3, state_trap,  // 140-143
    state_junk1, state_junk2, state_junk3, state_trap,  // 144-147
    state_junk1, state_junk2, state_junk3, state_trap,  // 148-151
    state_junk1, state_junk2, state_junk3, state_trap,  // 152-155
    state_junk1, state_junk2, state_junk3, state_trap,  // 156-159
    state_junk1, state_junk2, state_junk3, state_trap,  // 160-163
    state_junk1, state_junk2, state_junk3, state_trap,  // 164-167
    state_junk1, state_junk2, state_junk3, state_trap,  // 168-171
    state_junk1, state_junk2, state_junk3, state_trap,  // 172-175
    state_junk1, state_junk2, state_junk3, state_trap,  // 176-179
    state_junk1, state_junk2, state_junk3, state_trap,  // 180-183
    state_junk1, state_junk2, state_junk3, state_trap,  // 184-187
    state_junk1, state_junk2, state_junk3, state_trap,  // 188-191
    state_junk1, state_junk2, state_junk3, state_trap,  // 192-195
    state_junk1, state_junk2, state_junk3, state_trap,  // 196-199
    state_junk1, state_junk2, state_junk3, state_trap,  // 200-203
    state_junk1, state_junk2, state_junk3, state_trap,  // 204-207
    state_junk1, state_junk2, state_junk3, state_trap,  // 208-211
    state_junk1, state_junk2, state_junk3, state_trap,  // 212-215
    state_junk1, state_junk2, state_junk3, state_trap,  // 216-219
    state_junk1, state_junk2, state_junk3, state_trap,  // 220-223
    state_junk1, state_junk2, state_junk3, state_trap,  // 224-227
    state_junk1, state_junk2, state_junk3, state_trap,  // 228-231
    state_junk1, state_junk2, state_junk3, state_trap,  // 232-235
    state_junk1, state_junk2, state_junk3, state_trap,  // 236-239
    state_junk1, state_junk2, state_junk3, state_trap,  // 240-243
    state_junk1, state_junk2, state_junk3, state_trap,  // 244-247
    state_junk1, state_junk2, state_junk3, state_trap,  // 248-251
    state_junk1, state_junk2, state_junk3, state_trap   // 252-255
};

// ============================================================================
// STATE HANDLER IMPLEMENTATIONS
// ============================================================================

// Junk states - never actually reached with valid state transitions
static void state_junk1(void) {
    ANTI_DISASM_1();
    junk1 ^= 0xDEADC0DE;
    junk2 += junk1;
    gState = 255;  // Trap
}

static void state_junk2(void) {
    ANTI_DISASM_2();
    for (int i = 0; i < 10; i++) {
        junkArr[i] ^= junk1;
    }
    gState = 255;
}

static void state_junk3(void) {
    ANTI_DISASM_FAKE_CALL();
    junk3 = junk1 * junk2;
    gState = 255;
}

static void state_trap(void) {
    ANTI_DISASM_OVERLAP();
    // This should never be reached
    debugDetected = 1;
    exit(1);
}

// Draw board states
static void state_draw_init(void) {
    ANTI_DISASM_1();
    if (checkTracerPid()) {
        debugDetected = 1;
    }
    gState = 1;
}

static void state_draw_copy(void) {
    ANTI_DISASM_FAKE_CALL();
    memcpy((void*)gTempBoard, board, sizeof(board));
    gLoopVar1 = 0;
    gState = 2;
}

static void state_draw_overlay_start(void) {
    if (gLoopVar1 >= 4) {
        gState = 5;
    } else {
        gLoopVar2 = 0;
        gState = 3;
    }
}

static void state_draw_overlay_inner(void) {
    ANTI_DISASM_2();
    if (gLoopVar2 >= 4) {
        gLoopVar1++;
        gState = 2;
    } else {
        gState = 4;
    }
}

static void state_draw_overlay_cell(void) {
    if (currentPiece[gLoopVar1][gLoopVar2] != 0) {
        int boardY = currentY + gLoopVar1;
        int boardX = currentX + gLoopVar2;
        if (boardY >= 0 && boardY < BOARD_HEIGHT && 
            boardX >= 0 && boardX < BOARD_WIDTH) {
            gTempBoard[boardY][boardX] = currentPiece[gLoopVar1][gLoopVar2];
        }
    }
    gLoopVar2++;
    junk3 ^= (gLoopVar1 * gLoopVar2);
    gState = 3;
}

static void state_draw_clear(void) {
    ANTI_DISASM_OVERLAP();
    clearScreen();
    gState = 6;
}

static void state_draw_header(void) {
    printf("╔════════════════════╗  Score: %d\n", score);
    gLoopVar1 = 0;
    gState = 7;
}

static void state_draw_row_start(void) {
    ANTI_DISASM_PTR();
    if (gLoopVar1 >= BOARD_HEIGHT) {
        gState = 10;
    } else {
        printf("║");
        gLoopVar2 = 0;
        gState = 8;
    }
}

static void state_draw_cell(void) {
    if (gLoopVar2 >= BOARD_WIDTH) {
        gState = 9;
    } else {
        printCell(gTempBoard[gLoopVar1][gLoopVar2]);
        gLoopVar2++;
    }
}

static void state_draw_row_end(void) {
    printf("║");
    if (gLoopVar1 == 0) printf("  Lines: %d", linesCleared);
    else if (gLoopVar1 == 2) printf("  [W/A/S/D] Move");
    else if (gLoopVar1 == 3) printf("  [Q] Rotate");
    else if (gLoopVar1 == 4) printf("  [P] Check Pattern");
    else if (gLoopVar1 == 5) printf("  [ESC] Quit");
    printf("\n");
    gLoopVar1++;
    gState = 7;
}

static void state_draw_footer(void) {
    ANTI_DISASM_1();
    printf("╚════════════════════╝\n");
    gState = 11;
}

static void state_draw_done(void) {
    gState = 255;  // Signal completion
}

// Spawn piece states
static void state_spawn_init(void) {
    ANTI_DISASM_2();
    if (timingCheck()) {
        debugDetected = 1;
    }
    gPieceType = rand() % 7;
    currentType = gPieceType;
    currentX = BOARD_WIDTH / 2 - 2;
    currentY = 0;
    gLoopVar1 = 0;
    gState = 17;
}

static void state_spawn_copy_outer(void) {
    if (gLoopVar1 >= 4) {
        gState = 20;
    } else {
        gLoopVar2 = 0;
        gState = 18;
    }
}

static void state_spawn_copy_inner(void) {
    ANTI_DISASM_FAKE_CALL();
    if (gLoopVar2 >= 4) {
        gLoopVar1++;
        gState = 17;
    } else {
        gState = 19;
    }
}

static void state_spawn_copy_cell(void) {
    currentPiece[gLoopVar1][gLoopVar2] = TETROMINOS[gPieceType][gLoopVar1][gLoopVar2];
    gLoopVar2++;
    if (junk2 == 0xDEAD) {
        junkFunc2(junk1);
    }
    gState = 18;
}

static void state_spawn_done(void) {
    gState = 255;
}

// Collision check states (uses gLoopVar1, gLoopVar2 as i, j)
volatile int collisionOffsetX = 0;
volatile int collisionOffsetY = 0;
volatile int collisionNewX = 0;
volatile int collisionNewY = 0;

static void state_collision_init(void) {
    ANTI_DISASM_OVERLAP();
    gLoopVar1 = 0;
    gState = 33;
}

static void state_collision_outer(void) {
    if (gLoopVar1 >= 4) {
        gState = 38;  // No collision
    } else {
        gLoopVar2 = 0;
        gState = 34;
    }
}

static void state_collision_inner(void) {
    ANTI_DISASM_1();
    if (gLoopVar2 >= 4) {
        gLoopVar1++;
        gState = 33;
    } else {
        gState = 35;
    }
}

static void state_collision_check(void) {
    if (currentPiece[gLoopVar1][gLoopVar2] != 0) {
        collisionNewY = currentY + gLoopVar1 + collisionOffsetY;
        collisionNewX = currentX + gLoopVar2 + collisionOffsetX;
        gState = 36;
    } else {
        gLoopVar2++;
        gState = 34;
    }
}

static void state_collision_bounds(void) {
    ANTI_DISASM_PTR();
    if (collisionNewX < 0 || collisionNewX >= BOARD_WIDTH || collisionNewY >= BOARD_HEIGHT) {
        gState = 37;  // Collision
    } else if (collisionNewY >= 0 && board[collisionNewY][collisionNewX] != 0) {
        gState = 37;  // Collision
    } else {
        gLoopVar2++;
        gState = 34;
    }
}

static void state_collision_yes(void) {
    gReturnVal = 1;
    gState = 255;
}

static void state_collision_no(void) {
    gReturnVal = 0;
    gState = 255;
}

// Clear lines states
static void state_clear_init(void) {
    ANTI_DISASM_2();
    gLoopVar1 = BOARD_HEIGHT - 1;
    gCleared = 0;
    gState = 49;
}

static void state_clear_row_check(void) {
    if (gLoopVar1 < 0) {
        gState = 53;
    } else {
        gLoopVar2 = 0;
        gFull = 1;
        gState = 50;
    }
}

static void state_clear_cell_check(void) {
    ANTI_DISASM_FAKE_CALL();
    if (gLoopVar2 >= BOARD_WIDTH) {
        gState = 51;
    } else {
        if (board[gLoopVar1][gLoopVar2] == 0) {
            gFull = 0;
        }
        gLoopVar2++;
    }
}

static void state_clear_process(void) {
    if (gFull) {
        gState = 52;
    } else {
        gLoopVar1--;
        gState = 49;
    }
}

static void state_clear_shift(void) {
    ANTI_DISASM_OVERLAP();
    for (int k = gLoopVar1; k > 0; k--) {
        for (int l = 0; l < BOARD_WIDTH; l++) {
            board[k][l] = board[k-1][l];
        }
    }
    for (int l = 0; l < BOARD_WIDTH; l++) {
        board[0][l] = 0;
    }
    gCleared++;
    gState = 49;
}

static void state_clear_done(void) {
    linesCleared += gCleared;
    score += gCleared * 100;
    gReturnVal = gCleared;
    gState = 255;
}

// Pattern check states
static void state_pattern_init(void) {
    ANTI_DISASM_1();
    if (checkTracerPid() || debugDetected) {
        printf("\n[!] Nice try ;)\n");
        gState = 72;
        return;
    }
    gLoopVar1 = 0;
    gMatch = 1;
    gState = 65;
}

static void state_pattern_outer(void) {
    if (gLoopVar1 >= 4) {
        gState = 68;
    } else {
        gLoopVar2 = 0;
        gState = 66;
    }
}

static void state_pattern_inner(void) {
    ANTI_DISASM_PTR();
    if (gLoopVar2 >= BOARD_WIDTH) {
        gLoopVar1++;
        gState = 65;
    } else {
        gState = 67;
    }
}

static void state_pattern_compare(void) {
    if (junk1 == 0xDEADBEEF && junk2 == 0) {
        junkFunc1();
    }
    
    if (board[gLoopVar1][gLoopVar2] != WINNING_PATTERN[gLoopVar1][gLoopVar2]) {
        gMatch = 0;
    }
    gLoopVar2++;
    gState = 66;
}

static void state_pattern_result(void) {
    ANTI_DISASM_2();
    if (gMatch) {
        gState = 69;
    } else {
        gState = 71;
    }
}

static void state_pattern_decrypt(void) {
    ANTI_DISASM_FAKE_CALL();
    deriveKeyFromPattern(WINNING_PATTERN, (unsigned char*)gKey);
    memcpy((void*)gFlag, encFlag, 28);
    gState = 70;
}

static void state_pattern_show(void) {
    ANTI_DISASM_OVERLAP();
    realDecrypt((unsigned char*)gFlag, 28, (unsigned char*)gKey, 24);
    gFlag[30] = '\0';
    printf("\n\033[1;32m[*] Pattern matched! Flag: %s\033[0m\n", gFlag);
    printf("Press any key to continue...\n");
    getchar();
    gState = 72;
}

static void state_pattern_fail(void) {
    printf("\n\033[1;33m[!] Pattern not matched. Keep building!\033[0m\n");
    printf("Hint: Create a specific pattern in the first 4 rows.\n");
    usleep(1500000);
    gState = 72;
}

static void state_pattern_done(void) {
    gState = 255;
}

// Game loop states
static void state_game_init(void) {
    ANTI_DISASM_1();
    srand(time(NULL));
    memset(board, 0, sizeof(board));
    score = 0;
    linesCleared = 0;
    
    // Inline spawn piece
    gState = 16;
}

static void state_game_check(void) {
    if (gGameOver) {
        gState = 87;
    } else {
        gState = 82;
    }
}

static void state_game_draw(void) {
    ANTI_DISASM_2();
    // Run draw state machine
    gState = 0;
    while (gState != 255) {
        ANTI_DISASM_OVERLAP();
        jump_table[gState & 0xFF]();
    }
    gState = 83;
}

static void state_game_input(void) {
    ANTI_DISASM_PTR();
    gInput = 0;
    if (kbhit()) {
        gInput = getchar();
        if (gInput == 27) {
            if (kbhit()) {
                char seq1 = getchar();
                if (seq1 == '[' && kbhit()) {
                    char seq2 = getchar();
                    switch(seq2) {
                        case 'A': gInput = 'w'; break;
                        case 'B': gInput = 's'; break;
                        case 'C': gInput = 'd'; break;
                        case 'D': gInput = 'a'; break;
                    }
                }
            } else {
                gState = 87;
                return;
            }
        }
    }
    gState = 84;
}

// Helper for collision check in flattened context
int flatCheckCollision(int offsetX, int offsetY) {
    collisionOffsetX = offsetX;
    collisionOffsetY = offsetY;
    gState = 32;
    while (gState != 255) {
        jump_table[gState & 0xFF]();
    }
    return gReturnVal;
}

static void state_game_process(void) {
    ANTI_DISASM_FAKE_CALL();
    switch (gInput) {
        case 'a':
        case 'A':
            if (!flatCheckCollision(-1, 0)) currentX--;
            break;
        case 'd':
        case 'D':
            if (!flatCheckCollision(1, 0)) currentX++;
            break;
        case 's':
        case 'S':
            if (!flatCheckCollision(0, 1)) {
                currentY++;
                score++;
            }
            break;
        case 'w':
        case 'W':
            while (!flatCheckCollision(0, 1)) {
                currentY++;
                score += 2;
            }
            break;
        case 'q':
        case 'Q':
            {
                // Rotate piece
                int temp[4][4];
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        temp[j][3-i] = currentPiece[i][j];
                    }
                }
                int backup[4][4];
                memcpy(backup, currentPiece, sizeof(currentPiece));
                memcpy(currentPiece, temp, sizeof(temp));
                if (flatCheckCollision(0, 0)) {
                    memcpy(currentPiece, backup, sizeof(backup));
                }
            }
            break;
        case 'p':
        case 'P':
            {
                gState = 64;
                while (gState != 255) {
                    jump_table[gState & 0xFF]();
                }
            }
            break;
    }
    gState = 85;
}

static void state_game_drop(void) {
    ANTI_DISASM_OVERLAP();
    gDropCounter++;
    if (gDropCounter >= 5) {
        gDropCounter = 0;
        if (!flatCheckCollision(0, 1)) {
            currentY++;
        } else {
            gState = 86;
            return;
        }
    }
    usleep(50000);
    gState = 81;
}

static void state_game_lock(void) {
    ANTI_DISASM_1();
    // Lock piece
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece[i][j] != 0) {
                int boardY = currentY + i;
                int boardX = currentX + j;
                if (boardY >= 0 && boardY < BOARD_HEIGHT &&
                    boardX >= 0 && boardX < BOARD_WIDTH) {
                    board[boardY][boardX] = currentPiece[i][j];
                }
            }
        }
    }
    junk1 = junkFunc2(junk1);
    
    // Clear lines
    gState = 48;
    while (gState != 255) {
        jump_table[gState & 0xFF]();
    }
    
    // Spawn new piece
    gState = 16;
    while (gState != 255) {
        jump_table[gState & 0xFF]();
    }
    
    // Check game over
    if (flatCheckCollision(0, 0)) {
        gGameOver = 1;
    }
    gState = 81;
}

static void state_game_over(void) {
    disableRawMode();
    clearScreen();
    printf("Game Over! Final Score: %d\n", score);
    printf("Lines Cleared: %d\n", linesCleared);
    gState = 255;
}

// Main game loop using big jump table
void gameLoop() {
    ANTI_DISASM_1();
    
    gGameOver = 0;
    gDropCounter = 0;
    
    // Initialize game
    gState = 80;
    jump_table[gState & 0xFF]();
    
    // Spawn first piece
    while (gState != 255) {
        ANTI_DISASM_2();
        jump_table[gState & 0xFF]();
    }
    
    enableRawMode();
    
    // Main loop
    gState = 81;
    while (gState != 255) {
        ANTI_DISASM_PTR();
        jump_table[gState & 0xFF]();
    }
}

// Anti-debug: Check parent process
int __attribute__((constructor)) antiDebugInit() {
    ANTI_DISASM_OVERLAP();
    signal(SIGTRAP, sigtrapHandler);
    
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
        debugDetected = 1;
    }
    
    timingCheck();
    
    return 0;
}

// Junk constructor
void __attribute__((constructor)) junkInit() {
    ANTI_DISASM_1();
    junk1 = 0xDEADBEEF;
    junk2 = 0xCAFEBABE;
    junk3 = 0xBAADF00D;
    for (int i = 0; i < 64; i++) {
        junkArr[i] = (char)(i ^ 0x55);
    }
}

int main(int argc, char** argv) {
    ANTI_DISASM_2();
    
    volatile int dummy = junkFunc2(argc);
    (void)dummy;
    
    printf("\033[1;36m");
    printf("╔════════════════════════════════════════╗\n");
    printf("║     TETRIS CHALLENGE - AIS3 2026       ║\n");
    printf("║                                        ║\n");
    printf("║  Create the secret pattern to win!    ║\n");
    printf("║                                        ║\n");
    printf("║  Controls:                             ║\n");
    printf("║  W/↑ - Hard Drop    A/← - Move Left    ║\n");
    printf("║  S/↓ - Soft Drop    D/→ - Move Right   ║\n");
    printf("║  Q   - Rotate       P   - Check Pattern║\n");
    printf("║  ESC - Quit                            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\033[0m");
    printf("\nPress ENTER to start...");
    getchar();
    
    ANTI_DISASM_FAKE_CALL();
    
    gameLoop();
    
    return 0;
}
