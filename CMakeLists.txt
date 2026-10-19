cmake_minimum_required(VERSION 3.5)
project(Encoder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES 
    "C:/msys64/mingw64/include/c++/15.2.0"
    "C:/msys64/mingw64/include/c++/15.2.0/x86_64-w64-mingw32"
    "C:/msys64/mingw64/include/c++/15.2.0/backward"
    "C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/15.2.0/include"
    "C:/msys64/mingw64/include"
    "C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/15.2.0/include-fixed"
    "C:/msys64/mingw64/x86_64-w64-mingw32/include"
    "C:/Users/darya/Desktop/Combez/8_semester/Development of information security tools/development_of_information_security_tools/other_libs/openssl-3.6.1"
)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/crypto/crypto_constants.h
    src/crypto/crypto_manager.h
    src/crypto/crypto_manager.cpp
    src/crypto/password_key_derivation.h
    src/crypto/password_key_derivation.cpp

    src/testing/itest.h
    src/testing/test_menu.h
    src/testing/test_menu.cpp
    src/testing/test_utils.h
    src/testing/test_utils.cpp
)

# Корень исходников: подключения вида "crypto/crypto_manager.h"
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        OpenSSL::SSL
        OpenSSL::Crypto
)
