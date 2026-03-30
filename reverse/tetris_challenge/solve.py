enc_flag = [
    0x2e, 0xa5, 0x56, 0x46, 0x0d, 0x7c, 0x8e, 0xdc,
    0x83, 0x6f, 0x30, 0x83, 0xff, 0xf8, 0xa5, 0x5c,
    0xd0, 0x76, 0xd8, 0xcd, 0x99, 0xdc, 0x3f, 0x39,
    0x9d, 0x65, 0x70, 0x64
]

pattern = [
    [5, 0, 5, 0, 1, 0, 4, 4, 4, 0],
    [5, 5, 5, 0, 1, 0, 4, 0, 0, 0],
    [5, 0, 5, 0, 1, 0, 0, 4, 4, 0],
    [5, 0, 5, 0, 1, 0, 4, 4, 0, 3],
]

def derive_key(pattern):
    hash_val = 0x811c9dc5

    for i in range(4):
        for j in range(10):
            hash_val ^= pattern[i][j]
            hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF

    key = []
    for i in range(24):
        key.append((hash_val >> ((i % 4) * 8)) & 0xFF)
        hash_val = (hash_val * 1103515245 + 12345) & 0x7FFFFFFF

    return key

def rc4_decrypt(data, key):
    S = list(range(256))
    j = 0

    for i in range(256):
        j = (j + S[i] + key[i % len(key)]) % 256
        S[i], S[j] = S[j], S[i]

    i = 0
    j = 0
    out = []

    for byte in data:
        i = (i + 1) % 256
        j = (j + S[i]) % 256
        S[i], S[j] = S[j], S[i]
        k = S[(S[i] + S[j]) % 256]
        out.append(byte ^ k)

    return bytes(out)

key = derive_key(pattern)
flag = rc4_decrypt(enc_flag, key)

print("FLAG:", flag.decode())