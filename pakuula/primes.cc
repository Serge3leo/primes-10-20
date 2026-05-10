#include <iostream>
#include <iomanip>  // для std::fixed и std::setprecision
#include <chrono>   // для измерения времени
#include <string>
#include <cstring>
// Реализация больших чисел из OpenSSL, у них супер-оптимизированные функции проверки на простоту
#include <openssl/bn.h>

void print_usage(const char* prog) {
    std::cerr << "Использование: " << prog << " n1 n2 [--dec|--hex]" << std::endl;
    std::cerr << "  n1   : начальное число (включительно)" << std::endl;
    std::cerr << "  n2   : конечное число (не включается)" << std::endl;
    std::cerr << "  --dec: вывод в десятичном формате (по умолчанию)" << std::endl;
    std::cerr << "  --hex: вывод в шестнадцатеричном формате" << std::endl;
    std::cerr << "Примеры:" << std::endl;
    std::cerr << "  " << prog << " 10 50" << std::endl;
    std::cerr << "  " << prog << " 0xFF 0x200 --hex" << std::endl;
    std::cerr << "  " << prog << " 1000000000000000000 1000000000000000100" << std::endl;
}

/** Класс с автоматическим управлением памятью для BIGNUM */
struct BigNum {
    BIGNUM* bn;

    BigNum() : bn(BN_new()) {}
    BigNum(uint64_t value) : bn(BN_new()) { BN_set_word(bn, value); }
    
    ~BigNum() { BN_free(bn); }

    BigNum(const BigNum& other) = delete; // запрет копирования
    BigNum& operator=(const BigNum& other) = delete; // запрет присваивания
};

/** Автоматическое освобождение контекста BN_CTX */
struct ContextReleaser {
    BN_CTX* ctx;
    ContextReleaser(BN_CTX* c) : ctx(c) {}
    ~ContextReleaser() { BN_CTX_free(ctx); }

    ContextReleaser(const ContextReleaser& other) = delete; // запрет копирования
    ContextReleaser& operator=(const ContextReleaser& other) = delete; // запрет присваивания
};

// Создаёт BIGNUM из строки (поддерживает dec и hex с префиксом 0x)
bool str_to_bn(const char* s, BigNum& result) {
    if (strlen(s) == 0) {
        return false;
    }
    if (strlen(s) == 1) {
        if (isdigit(s[0])) {
            BN_set_word(result.bn, s[0] - '0');
            return true;
        }
        return false;
    }

    int ok;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        ok = BN_hex2bn(&result.bn, s + 2);
    } else {
        ok = BN_dec2bn(&result.bn, s);
    }

    return ok;
}

int main(int argc, char* argv[]) {
    // Разбор аргументов
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char* n1_str = nullptr;
    const char* n2_str = nullptr;
    bool hex_output = false;

    // Обработка позиционных аргументов и флагов
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--hex") == 0) {
            hex_output = true;
        } else if (std::strcmp(argv[i], "--dec") == 0) {
            hex_output = false;
        } else if (!n1_str) {
            n1_str = argv[i];
        } else if (!n2_str) {
            n2_str = argv[i];
        } else {
            std::cerr << "Ошибка: слишком много аргументов." << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!n1_str || !n2_str) {
        std::cerr << "Ошибка: не указаны границы интервала." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    // Инициализация
    BN_CTX* ctx = BN_CTX_new();
    if (!ctx) {
        std::cerr << "Ошибка: не удалось создать контекст OpenSSL." << std::endl;
        return 1;
    }
    ContextReleaser ctx_releaser(ctx); // гарантирует освобождение контекста

    BigNum n1, n2; // объекты для хранения больших чисел

    if (!str_to_bn(n1_str, n1) || !str_to_bn(n2_str, n2)) {
        std::cerr << "Ошибка: неверный формат числа." << std::endl;
        return 1;
    }

    if (BN_is_odd(n1.bn)) {
        // Если n1 нечётное, оставляем его как есть
    } else {
        // Если n1 чётное, делаем его нечётным (увеличиваем на 1)
        BN_add_word(n1.bn, 1);
    }

    BigNum two{2};
    BigNum current;
    BN_copy(current.bn, n1.bn);

    // Проверка: n1 < n2
    if (BN_cmp(n1.bn, n2.bn) >= 0) {
        std::cerr << "Ошибка: n1 должно быть меньше n2." << std::endl;
        return 1;
    }

    // Отключаем буферизацию вывода для мгновенной печати
    // std::cout.setf(std::ios::unitbuf);

    // Вывод заголовка
    std::cout << "# Интервал: [" << n1_str << ", " << n2_str << ")" << std::endl;
    std::cout << "# Формат: " << (hex_output ? "шестнадцатеричный" : "десятичный") << std::endl;

    int count = 0;

    auto start_time = std::chrono::steady_clock::now();

    // Основной цикл: current от n1 до n2 (не включая n2)
    while (BN_cmp(current.bn, n2.bn) < 0) {
        int is_prime = BN_is_prime_fasttest_ex(
            current.bn,           // проверяемое число
            BN_prime_checks,   // количество раундов Миллера-Рабина
            ctx,               // контекст
            1,                 // пробное деление включено
            nullptr            // callback не нужен
        );

        if (is_prime == 1) {
            // Нашли простое — преобразуем и выводим
            char* str = hex_output ? BN_bn2hex(current.bn) : BN_bn2dec(current.bn);
            if (str) {
                if (hex_output) {
                    std::cout << "0x" << str << std::endl;
                } else {
                    std::cout << str << std::endl;
                }
                OPENSSL_free(str);
                count++;
            }
        } else if (is_prime < 0) {
            std::cerr << "Ошибка при проверке числа." << std::endl;
            break;
        }

        // current += 2
        BN_add(current.bn, current.bn, two.bn); // переходим к следующему нечётному числу
    }
    auto end_time = std::chrono::steady_clock::now();


    // Статистика
    std::cout << "# Найдено простых чисел: " << count << std::endl;
    std::chrono::duration<float> elapsed = end_time - start_time;
    std::cout << "# Затраченное время: " << std::fixed << std::setprecision(6) 
          << elapsed.count() << " сек" << std::endl;
    // Очистка
    // Объекты BigNum автоматически освобождают память, а контекст освобождается через ContextReleaser

    return 0;
}
