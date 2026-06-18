#ifndef CRYPTO_CONSTANTS_H
#define CRYPTO_CONSTANTS_H

// Размер ключа AES-256
static constexpr int KEY_SIZE = 32;
// Размер nonce для GCM
static constexpr int NONCE_SIZE = 12;
// Размер тега аутентификации
static constexpr int TAG_SIZE = 16;
// Размер соли
static constexpr int SALT_SIZE = 16;

// Параметры KDF (PBKDF2 по умолчанию)
static constexpr int PBKDF2_ITERATIONS = 100000;

// Размер и сигнатура магии формата файла
static constexpr char MAGIC[] = "ENCFILE";
static constexpr int MAGIC_SIZE = 7;

// Полный размер заголовка зашифрованного файла
static constexpr int HEADER_SIZE = MAGIC_SIZE + SALT_SIZE + NONCE_SIZE;

// Размер буфера для потокового шифрования/дешифрования
static constexpr int BUFFER_SIZE = 4096;

#endif // CRYPTO_CONSTANTS_H