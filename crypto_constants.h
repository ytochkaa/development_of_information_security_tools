#ifndef CRYPTO_CONSTANTS_H
#define CRYPTO_CONSTANTS_H

// Размер ключа AES-256
static constexpr int KEY_SIZE   = 32;
// Размер nonce для GCM
static constexpr int NONCE_SIZE = 12;
// Размер тега аутентификации
static constexpr int TAG_SIZE   = 16;

#endif // CRYPTO_CONSTANTS_H