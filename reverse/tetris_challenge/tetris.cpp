// Tetris Reverse Challenge - AIS3 Pre-Exam
// Obfuscated with: Control Flow Flattening, Anti-Debug, Junk Code

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

// Target pattern hash (SHA-like simple hash)
unsigned char targetPatternHash[] = {
    0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe
};

// The winning pattern (specific block arrangement)
// Pattern: First 4 rows should spell "AIS3" using colored blocks
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},  // A shape start
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},  // A I S
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},  // continued
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}   // ends with 3
};

// Junk variables (obfuscation)
volatile int junk1 = 0xDEADBEEF;
volatile int junk2 = 0xCAFEBABE;
volatile long junk3 = 0xBAADF00D;
volatile char junkArr[64] = {0};

// Anti-debug: Check /proc/self/status for TracerPid
int __attribute__((always_inline)) checkTracerPid() {
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
    return 0;
}

// Anti-debug: SIGTRAP handler
void sigtrapHandler(int sig) {
    debugDetected = 1;
}

// Anti-debug: Timing check
volatile unsigned long startTime = 0;
int __attribute__((noinline)) timingCheck() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long current = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
    if (startTime == 0) {
        startTime = current;
        return 0;
    }
    // If more than 5 seconds between checks, likely debugging
    if (current - startTime > 5000000) {
        return 1;
    }
    startTime = current;
    return 0;
}

// Junk function 1 (never called with true condition)
void __attribute__((noinline)) junkFunc1() {
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
    for (int i = 0; i < len; i++) {
        data[i] ^= (unsigned char)(junk1 >> (i % 4) * 8);
        data[i] = (data[i] << 3) | (data[i] >> 5);
    }
}

// Control flow flattening dispatcher state
volatile int cfState = 0;
volatile int cfNext = 0;

// Real key derivation from pattern
void deriveKeyFromPattern(int pattern[4][BOARD_WIDTH], unsigned char* key) {
    // Junk insertion
    if (junk1 == 0xFFFFFFFF) junkFunc1();
    
    unsigned int hash = 0x811c9dc5;  // FNV-1a
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            hash ^= pattern[i][j];
            hash *= 0x01000193;
            
            // More junk
            junk2 ^= hash;
        }
    }
    
    // Generate 24-byte key from hash
    for (int i = 0; i < 24; i++) {
        key[i] = (unsigned char)((hash >> ((i % 4) * 8)) & 0xFF);
        hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

// Real decryption function (RC4-like)
void realDecrypt(unsigned char* data, int len, unsigned char* key, int keyLen) {
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
    printf("\033[?25h");  // Show cursor
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &origTermios);
    atexit(disableRawMode);
    
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\033[?25l");  // Hide cursor
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

// Clear screen
void clearScreen() {
    printf("\033[2J\033[H");
}

// Draw the game board - FLATTENED CONTROL FLOW
void drawBoard() {
    int flatState = 0;
    int i = 0, j = 0;
    int tempBoard[BOARD_HEIGHT][BOARD_WIDTH];
    
    // Anti-debug check embedded
    if (checkTracerPid()) {
        debugDetected = 1;
    }
    
    while (1) {
        // Junk state transitions
        if (flatState == 999 && junk1 == 0) {
            junkFunc1();
            flatState = 0;
        }
        
        switch (flatState) {
            case 0:  // Initialize temp board
                memcpy(tempBoard, board, sizeof(board));
                flatState = 1;
                i = 0;
                break;
                
            case 1:  // Overlay current piece - outer loop
                if (i >= 4) {
                    flatState = 4;
                } else {
                    j = 0;
                    flatState = 2;
                }
                break;
                
            case 2:  // Inner loop
                if (j >= 4) {
                    i++;
                    flatState = 1;
                } else {
                    flatState = 3;
                }
                break;
                
            case 3:  // Process cell
                if (currentPiece[i][j] != 0) {
                    int boardY = currentY + i;
                    int boardX = currentX + j;
                    if (boardY >= 0 && boardY < BOARD_HEIGHT && 
                        boardX >= 0 && boardX < BOARD_WIDTH) {
                        tempBoard[boardY][boardX] = currentPiece[i][j];
                    }
                }
                j++;
                flatState = 2;
                // Junk
                junk3 ^= (i * j);
                break;
                
            case 4:  // Clear screen and draw header
                clearScreen();
                printf("╔════════════════════╗  Score: %d\n", score);
                i = 0;
                flatState = 5;
                break;
                
            case 5:  // Draw board rows - outer
                if (i >= BOARD_HEIGHT) {
                    flatState = 8;
                } else {
                    printf("║");
                    j = 0;
                    flatState = 6;
                }
                break;
                
            case 6:  // Draw cells - inner
                if (j >= BOARD_WIDTH) {
                    flatState = 7;
                } else {
                    printCell(tempBoard[i][j]);
                    j++;
                }
                break;
                
            case 7:  // End row
                printf("║");
                if (i == 0) printf("  Lines: %d", linesCleared);
                else if (i == 2) printf("  [W/A/S/D] Move");
                else if (i == 3) printf("  [Q] Rotate");
                else if (i == 4) printf("  [P] Check Pattern");
                else if (i == 5) printf("  [ESC] Quit");
                printf("\n");
                i++;
                flatState = 5;
                break;
                
            case 8:  // Draw footer
                printf("╚════════════════════╝\n");
                flatState = 100;
                break;
                
            case 100:  // Exit
                return;
                
            default:
                flatState = 0;
                break;
        }
    }
}

// Initialize game board
void initBoard() {
    memset(board, 0, sizeof(board));
    score = 0;
    linesCleared = 0;
}

// Spawn new piece - FLATTENED
void spawnPiece() {
    int state = 0;
    int pieceType = 0;
    int i = 0, j = 0;
    
    while (1) {
        switch (state) {
            case 0:
                // Anti-debug
                if (timingCheck()) {
                    debugDetected = 1;
                }
                pieceType = rand() % 7;
                currentType = pieceType;
                currentX = BOARD_WIDTH / 2 - 2;
                currentY = 0;
                i = 0;
                state = 1;
                break;
                
            case 1:
                if (i >= 4) {
                    state = 4;
                } else {
                    j = 0;
                    state = 2;
                }
                break;
                
            case 2:
                if (j >= 4) {
                    i++;
                    state = 1;
                } else {
                    state = 3;
                }
                break;
                
            case 3:
                currentPiece[i][j] = TETROMINOS[pieceType][i][j];
                j++;
                state = 2;
                // Junk
                if (junk2 == 0xDEAD) {
                    junkFunc2(junk1);
                }
                break;
                
            case 4:
                return;
                
            default:
                state = 0;
                break;
        }
    }
}

// Check collision
int checkCollision(int offsetX, int offsetY) {
    int state = 0;
    int i = 0, j = 0;
    int newX, newY;
    
    while (1) {
        switch (state) {
            case 0:
                i = 0;
                state = 1;
                break;
                
            case 1:
                if (i >= 4) {
                    state = 6;  // No collision
                } else {
                    j = 0;
                    state = 2;
                }
                break;
                
            case 2:
                if (j >= 4) {
                    i++;
                    state = 1;
                } else {
                    state = 3;
                }
                break;
                
            case 3:
                if (currentPiece[i][j] != 0) {
                    newY = currentY + i + offsetY;
                    newX = currentX + j + offsetX;
                    state = 4;
                } else {
                    j++;
                    state = 2;
                }
                break;
                
            case 4:  // Check bounds
                if (newX < 0 || newX >= BOARD_WIDTH || newY >= BOARD_HEIGHT) {
                    state = 5;  // Collision
                } else if (newY >= 0 && board[newY][newX] != 0) {
                    state = 5;  // Collision
                } else {
                    j++;
                    state = 2;
                }
                break;
                
            case 5:
                return 1;  // Collision detected
                
            case 6:
                return 0;  // No collision
                
            default:
                state = 0;
                break;
        }
    }
}

// Lock piece to board
void lockPiece() {
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
    
    // Junk
    junk1 = junkFunc2(junk1);
}

// Clear completed lines - FLATTENED
int clearLines() {
    int state = 0;
    int i = 0, j = 0;
    int cleared = 0;
    int full;
    
    while (1) {
        switch (state) {
            case 0:
                i = BOARD_HEIGHT - 1;
                cleared = 0;
                state = 1;
                break;
                
            case 1:
                if (i < 0) {
                    state = 7;
                } else {
                    j = 0;
                    full = 1;
                    state = 2;
                }
                break;
                
            case 2:  // Check if line is full
                if (j >= BOARD_WIDTH) {
                    state = 3;
                } else {
                    if (board[i][j] == 0) {
                        full = 0;
                    }
                    j++;
                }
                break;
                
            case 3:  // Process line
                if (full) {
                    state = 4;  // Clear line
                } else {
                    i--;
                    state = 1;
                }
                break;
                
            case 4:  // Shift lines down
                for (int k = i; k > 0; k--) {
                    for (int l = 0; l < BOARD_WIDTH; l++) {
                        board[k][l] = board[k-1][l];
                    }
                }
                for (int l = 0; l < BOARD_WIDTH; l++) {
                    board[0][l] = 0;
                }
                cleared++;
                state = 1;  // Check same row again
                break;
                
            case 7:
                linesCleared += cleared;
                score += cleared * 100;
                return cleared;
                
            default:
                state = 0;
                break;
        }
    }
}

// Rotate piece
void rotatePiece() {
    int temp[4][4];
    int i, j;
    
    // Junk anti-debug
    if (debugDetected) {
        junkDecrypt(encFlag, 30);
    }
    
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            temp[j][3-i] = currentPiece[i][j];
        }
    }
    
    int backup[4][4];
    memcpy(backup, currentPiece, sizeof(currentPiece));
    memcpy(currentPiece, temp, sizeof(temp));
    
    if (checkCollision(0, 0)) {
        memcpy(currentPiece, backup, sizeof(backup));
    }
}

// Check pattern and decrypt flag - HEAVILY FLATTENED
void checkPattern() {
    int state = 0;
    int i = 0, j = 0;
    int match = 1;
    unsigned char key[24];
    unsigned char flag[32];
    int currentPattern[4][BOARD_WIDTH];
    
    // Copy current board state for pattern
    memcpy(currentPattern, board, sizeof(currentPattern));
    
    while (1) {
        // Junk state
        if (state == 888 && junk1 == 0) {
            junkFunc1();
            state = 0;
        }
        
        switch (state) {
            case 0:
                // Anti-debug check
                if (checkTracerPid() || debugDetected) {
                    printf("\n[!] Nice try ;)\n");
                    return;
                }
                i = 0;
                match = 1;
                state = 1;
                break;
                
            case 1:
                if (i >= 4) {
                    state = 5;  // All rows checked
                } else {
                    j = 0;
                    state = 2;
                }
                break;
                
            case 2:
                if (j >= BOARD_WIDTH) {
                    i++;
                    state = 1;
                } else {
                    state = 3;
                }
                break;
                
            case 3:
                // Junk comparison
                if (junk1 == 0xDEADBEEF && junk2 == 0) {
                    junkFunc1();
                }
                
                // Real comparison
                if (board[i][j] != WINNING_PATTERN[i][j]) {
                    match = 0;
                }
                j++;
                state = 2;
                break;
                
            case 5:  // Check match result
                if (match) {
                    state = 10;  // Win!
                } else {
                    state = 20;  // Not yet
                }
                break;
                
            case 10:  // Derive key and decrypt
                deriveKeyFromPattern((int(*)[BOARD_WIDTH])WINNING_PATTERN, key);
                memcpy(flag, encFlag, 28);
                state = 11;
                break;
                
            case 11:
                realDecrypt(flag, 28, key, 24);
                flag[30] = '\0';
                printf("\n\033[1;32m[*] Pattern matched! Flag: %s\033[0m\n", flag);
                printf("Press any key to continue...\n");
                getchar();
                state = 100;
                break;
                
            case 20:  // Pattern not matched
                printf("\n\033[1;33m[!] Pattern not matched. Keep building!\033[0m\n");
                printf("Hint: Create a specific pattern in the first 4 rows.\n");
                usleep(1500000);
                state = 100;
                break;
                
            case 100:
                return;
                
            default:
                state = 0;
                break;
        }
    }
}

// Main game loop - FLATTENED
void gameLoop() {
    int state = 0;
    int gameOver = 0;
    char input;
    int dropCounter = 0;
    
    while (1) {
        // Junk
        if (state == 777 && junk3 == 0) {
            junkDecrypt((unsigned char*)junkArr, 64);
            state = 0;
        }
        
        switch (state) {
            case 0:  // Init
                srand(time(NULL));
                initBoard();
                spawnPiece();
                enableRawMode();
                state = 1;
                break;
                
            case 1:  // Main loop check
                if (gameOver) {
                    state = 100;
                } else {
                    state = 2;
                }
                break;
                
            case 2:  // Draw
                drawBoard();
                state = 3;
                break;
                
            case 3:  // Input handling
                input = 0;
                if (kbhit()) {
                    input = getchar();
                    // Handle escape sequences for arrow keys
                    if (input == 27) {  // ESC
                        if (kbhit()) {
                            char seq1 = getchar();
                            if (seq1 == '[' && kbhit()) {
                                char seq2 = getchar();
                                // Arrow keys: A=up, B=down, C=right, D=left
                                switch(seq2) {
                                    case 'A': input = 'w'; break;
                                    case 'B': input = 's'; break;
                                    case 'C': input = 'd'; break;
                                    case 'D': input = 'a'; break;
                                }
                            }
                        } else {
                            // Pure ESC key - quit
                            state = 100;
                            break;
                        }
                    }
                }
                state = 4;
                break;
                
            case 4:  // Process input
                switch (input) {
                    case 'a':
                    case 'A':
                        if (!checkCollision(-1, 0)) currentX--;
                        break;
                    case 'd':
                    case 'D':
                        if (!checkCollision(1, 0)) currentX++;
                        break;
                    case 's':
                    case 'S':
                        if (!checkCollision(0, 1)) {
                            currentY++;
                            score++;
                        }
                        break;
                    case 'w':
                    case 'W':
                        // Hard drop
                        while (!checkCollision(0, 1)) {
                            currentY++;
                            score += 2;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        rotatePiece();
                        break;
                    case 'p':
                    case 'P':
                        checkPattern();
                        break;
                }
                state = 5;
                break;
                
            case 5:  // Auto drop
                dropCounter++;
                if (dropCounter >= 5) {
                    dropCounter = 0;
                    if (!checkCollision(0, 1)) {
                        currentY++;
                    } else {
                        state = 6;
                        break;
                    }
                }
                usleep(50000);
                state = 1;
                break;
                
            case 6:  // Lock and spawn
                lockPiece();
                clearLines();
                spawnPiece();
                if (checkCollision(0, 0)) {
                    gameOver = 1;
                }
                state = 1;
                break;
                
            case 100:  // Game over / Exit
                disableRawMode();
                clearScreen();
                printf("Game Over! Final Score: %d\n", score);
                printf("Lines Cleared: %d\n", linesCleared);
                return;
                
            default:
                state = 0;
                break;
        }
    }
}

// Anti-debug: Check parent process
int __attribute__((constructor)) antiDebugInit() {
    // Set up SIGTRAP handler
    signal(SIGTRAP, sigtrapHandler);
    
    // Check ptrace
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
        debugDetected = 1;
    }
    
    // Initialize timing
    timingCheck();
    
    return 0;
}

// Junk constructor
void __attribute__((constructor)) junkInit() {
    junk1 = 0xDEADBEEF;
    junk2 = 0xCAFEBABE;
    junk3 = 0xBAADF00D;
    for (int i = 0; i < 64; i++) {
        junkArr[i] = (char)(i ^ 0x55);
    }
}

int main(int argc, char** argv) {
    // Junk
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
    
    gameLoop();
    
    return 0;
}
