#include "test_menu.h"
#include "itest.h"

#include <iostream>
#include <vector>
#include <memory>

void TestMenu::run() {
    std::vector<std::unique_ptr<ITest>> tests;

    if (tests.empty()) {
        std::cout << "Тесты не добавлены." << std::endl;
        return;
    }

    while (true) {
        std::cout << "\nВыберите тест:" << std::endl;
        for (size_t i = 0; i < tests.size(); i++) {
            std::cout << i + 1 << ". " << tests[i]->name() << std::endl;
        }
        std::cout << "0. Выход" << std::endl;
        std::cout << "Ваш выбор: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "0") {
            std::cout << "Выход из режима тестирования." << std::endl;
            return;
        }

        try {
            int idx = std::stoi(choice) - 1;
            if (idx >= 0 && idx < static_cast<int>(tests.size())) {
                tests[idx]->run();
            } else {
                std::cout << "Неверный выбор!" << std::endl;
            }
        } catch (...) {
            std::cout << "Неверный выбор!" << std::endl;
        }
    }
}
