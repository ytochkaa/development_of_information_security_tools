# UML диаграммы проекта

## 1. Диаграмма классов

```mermaid
classDiagram
    class main_cpp {
        +int main(argc, argv)
    }

    class CryptoManager {
        -CryptoManager()
        +static CryptoManager& instance()
        +bool encrypt(filePath, password)
        +bool decrypt(filePath, password)
        +bool encryptDirectory(dirPath, password)
        +bool decryptDirectory(dirPath, password)
        +static bool isValidFileForEncryption(filePath)
        +static bool isValidFileForDecryption(filePath)
        +static bool isValidDirectory(dirPath)
        -static bool isFileEncrypted(filePath)
        -bool processFile(filePath, password, operation)
        -bool encryptFile(filePath, password)
        -bool decryptFile(filePath, password)
    }

    class PasswordKeyDerivation {
        +static QByteArray deriveKeyFromPassword(password, salt)
        +static bool validatePassword(password)
    }

    class TestMenu {
        +void run()
    }

    class ITest {
        <<interface>>
        +virtual ~ITest()
        +virtual string name() const
        +virtual void run()
    }

    class TestUtils {
        <<namespace>>
        +QString testsBasePath()
        +bool copyDirectory(src, dst)
        +string getPassword(prompt)
        +void clearPassword(password)
    }

    class crypto_constants_h {
        <<constants>>
        +MAGIC
        +FORMAT_VERSION
        +SALT_SIZE
        +NONCE_SIZE
        +TAG_SIZE
        +KEY_SIZE
        +BUFFER_SIZE
        +PBKDF2_ITERATIONS
        +MIN_PASSWORD_LENGTH
        +MAX_PASSWORD_LENGTH
    }

    main_cpp ..> CryptoManager : uses
    main_cpp ..> PasswordKeyDerivation : uses
    main_cpp ..> TestMenu : uses
    main_cpp ..> crypto_constants_h : uses

    CryptoManager ..> PasswordKeyDerivation : uses
    CryptoManager ..> crypto_constants_h : uses

    TestMenu o-- ITest : contains
    TestUtils ..> PasswordKeyDerivation : uses
    TestUtils ..> crypto_constants_h : uses
```

## 2. Диаграмма последовательности (шифрование файла)

```mermaid
sequenceDiagram
    participant User
    participant main
    participant CryptoManager
    participant PasswordKeyDerivation
    participant FileSystem

    User->>main: вводит путь и пароль
    main->>CryptoManager: encrypt(filePath, password)
    CryptoManager->>CryptoManager: isValidFileForEncryption()
    CryptoManager->>CryptoManager: isFileEncrypted()
    CryptoManager->>PasswordKeyDerivation: deriveKeyFromPassword()
    PasswordKeyDerivation-->>CryptoManager: key
    CryptoManager->>FileSystem: прочитать исходный файл
    CryptoManager->>FileSystem: записать зашифрованный файл
    CryptoManager-->>main: true/false
    main-->>User: результат операции
```

## 3. Краткое описание связей

- `main.cpp` запускает интерфейс приложения и вызывает операции шифрования/дешифрования.
- `CryptoManager` является центральным управляющим классом и реализует логику работы с файлами и директориями.
- `PasswordKeyDerivation` отвечает за формирование ключа из пароля.
- `TestMenu` и `ITest` предназначены для тестового режима.
- `crypto_constants.h` хранит параметры криптографических операций.
