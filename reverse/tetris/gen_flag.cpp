// Flag generator - generates encrypted flag based on pattern
// This is NOT distributed with the challenge

#include <iostream>
#include <cstring>

#define BOARD_WIDTH 10

// The winning pattern
const int WINNING_PATTERN[4][BOARD_WIDTH] = {
    {5, 0, 5, 0, 1, 0, 4, 4, 4, 0},
    {5, 5, 5, 0, 1, 0, 4, 0, 0, 0},
    {5, 0, 5, 0, 1, 0, 0, 4, 4, 0},
    {5, 0, 5, 0, 1, 0, 4, 4, 0, 3}
};

// Real key derivation from pattern
void deriveKeyFromPattern(const int pattern[4][BOARD_WIDTH], unsigned char* key) {
    unsigned int hash = 0x811c9dc5;  // FNV-1a
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            hash ^= pattern[i][j];
            hash *= 0x01000193;
        }
    }
    
    // Generate 24-byte key from hash
    for (int i = 0; i < 24; i++) {
        key[i] = (unsigned char)((hash >> ((i % 4) * 8)) & 0xFF);
        hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

// RC4-like encryption
void encrypt(unsigned char* data, int len, unsigned char* key, int keyLen) {
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

int main() {
    const char* flag = "AIS3{T3tr1s_P4tt3rn_M4st3r!}";
    int len = strlen(flag);
    
    unsigned char key[24];
    deriveKeyFromPattern(WINNING_PATTERN, key);
    
    std::cout << "Key: ";
    for (int i = 0; i < 24; i++) {
        printf("0x%02x, ", key[i]);
    }
    std::cout << std::endl;
    
    unsigned char encrypted[64];
    memcpy(encrypted, flag, len);
    encrypt(encrypted, len, key, 24);
    
    std::cout << "Encrypted flag bytes:" << std::endl;
    std::cout << "unsigned char encFlag[] = {" << std::endl << "    ";
    for (int i = 0; i < len; i++) {
        printf("0x%02x", encrypted[i]);
        if (i < len - 1) printf(", ");
        if ((i + 1) % 8 == 0) printf("\n    ");
    }
    std::cout << ", 0" << std::endl << "};" << std::endl;
    
    std::cout << "\nFlag length: " << len << std::endl;
    
    // Verify decryption
    std::cout << "\nVerifying decryption..." << std::endl;
    encrypt(encrypted, len, key, 24);  // RC4 is symmetric
    encrypted[len] = '\0';
    std::cout << "Decrypted: " << encrypted << std::endl;
    
    return 0;
}
