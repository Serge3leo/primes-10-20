// Тестирование на простоту 128-битных чисел
#include <cstdint>
#include <stdexcept>

#include <array>

typedef unsigned __int128 uint128_t;

// 
// Редукция Монтгомери для 128-битных чисел.
//

/// 256-битное целое
struct uint256_t {
    uint128_t low;
    uint128_t high;

    uint256_t() : low(0), high(0) {}
    uint256_t(uint128_t l, uint128_t h) : low(l), high(h) {}
};

/// Умножение двух 128-битных чисел с получением 256-битного результата.
inline uint256_t mul_128(uint128_t a, uint128_t b) {
    uint128_t a_low  = (uint64_t)a;
    uint128_t a_high = a >> 64;
    uint128_t b_low  = (uint64_t)b;
    uint128_t b_high = b >> 64;

    uint128_t low_low = a_low * b_low;
    uint128_t low_high = a_low * b_high;
    uint128_t high_low = a_high * b_low;
    uint128_t high_high = a_high * b_high;

    uint128_t carry = (low_low >> 64) +
        (uint64_t)low_high +
        (uint64_t)high_low;

    uint128_t low =
        (low_low & (((uint128_t)1 << 64) - 1)) |
        ((carry & (((uint128_t)1 << 64) - 1)) << 64);

    uint128_t high = high_high +
        (low_high >> 64) +
        (high_low >> 64) +
        (carry >> 64);

    return uint256_t(low, high);
}

/// Возвращает (a + b) % mod, корректно обрабатывая возможные переполнения.
/// Требует, чтобы a, b < mod, иначе результат может быть некорректным.
inline uint128_t add_mod_128(uint128_t a, uint128_t b, uint128_t mod) {
    // Возвращает (a + b) % mod
    uint128_t r = a + b;

    // Если произошло переполнение, то r < a -- в этом случае нужно вычесть mod, так как r mod m == (r - m) mod m.
    // Если r >= mod, нужно вычесть mod.
    if (r < a || r >= mod) {
        r -= mod;
    }

    return r;
}

/// Возвращает 2*x % mod.
/// Требует, чтобы x < mod, иначе результат может быть некорректным.
inline uint128_t double_mod_128(uint128_t x, uint128_t mod) {
    if (x >= mod - x) {
        return x - (mod - x);
    } else {
        return x + x;
    }
}

/// Возвращает 2^bits % mod.
inline uint128_t pow2_mod_128(unsigned bits, uint128_t mod) {
    uint128_t r = 1;

    for (unsigned i = 0; i < bits; ++i) {
        r = double_mod_128(r, mod);
    }

    return r;
}

/// Вычисляет обратное число по модулю 2^128 для нечётного числа m.
inline uint128_t montgomery_inverse(uint128_t m) {
    // Возвращает -(m^{-1}) mod 2^128.
    // Работает только для нечётного m.
    uint128_t x = 1;

    // Вычисление 2^128/m методом Ньютона.
    // 1/m получается как результат итераций x = x * (2 - m * x) mod 2^128, 
    // который сходится к 1/m (по модулю 2^128) при любом нечётном m 
    // (требуется чтобы было взаимно просто с модулем, то есть 2^128).
    // После каждой итерации число корректных бит примерно удваивается.
    for (int i = 0; i < 8; ++i) {
        x *= 2 - m * x;
    }
    // x*m = 1 + A*2^128
    return -x;
}

// Умножение и возведение в степень по модулю для 128-битных чисел с использованием Montgomery reduction.
//
// Работает быстрее обычных арифметических операций по модулю при большом количестве операций с одним и тем же модулем
struct Montgomery128 {
    // Тип для чисел в Montgomery-представлении.
    // Используется для type safety, чтобы не смешивать обычные числа и числа в Montgomery-представлении.
    typedef struct montgomery_t { 
        uint128_t v;
        inline bool operator==(const montgomery_t& other) const {
            return v == other.v;
        }
    } montgomery_t;

    uint128_t mod;
    uint128_t inv;
    uint128_t r2;

    explicit Montgomery128(uint128_t m)
        : mod(m),
          inv(montgomery_inverse(m)),
          r2(pow2_mod_128(256, m)) {}

    // Редукция Монтгомери
    inline uint128_t reduce(const uint256_t& t) const {
        uint128_t q = t.low * inv;

        uint256_t qm = mul_128(q, mod);

        uint128_t low_sum = t.low + qm.low;
        uint128_t carry = (low_sum < t.low) ? 1 : 0;

        uint128_t res = t.high + qm.high + carry;

        if (res >= mod) {
            res -= mod;
        }

        return res;
    }

    // Преобразует обычное число в Montgomery-представление.
    inline montgomery_t to_montgomery(uint128_t x) const {
        return montgomery_t{reduce(mul_128(x, r2))};
    }


    // Преобразует число из Montgomery-представления обратно в обычное представление.
    inline uint128_t from_montgomery(const montgomery_t& x) const {
        return reduce(uint256_t(x.v, 0));
    }

    // Умножение двух чисел в Montgomery-представлении.
    inline montgomery_t mul(const montgomery_t& a, const montgomery_t& b) const {
        return montgomery_t{reduce(mul_128(a.v, b.v))};
    }

    // Возвращает (base^e) % mod.
    uint128_t exp_mod(uint128_t base, uint128_t e) const {
        auto result = exp_to_montgomery(base, e);
        return from_montgomery(result);
    }

    // Возвращает (base^e) % mod, где результат остаётся в Montgomery-представлении.
    montgomery_t exp_to_montgomery(uint128_t base, uint128_t e) const;
};


Montgomery128::montgomery_t 
Montgomery128::exp_to_montgomery(uint128_t base, uint128_t e) const {
    // Возвращает результат в Montgomery-представлении.
    montgomery_t result = to_montgomery(1);
    montgomery_t x = to_montgomery(base % mod);

    while (e > 0) {
        if (e & 1) {
            result = mul(result, x);
        }

        e >>= 1;

        if (e) {
            x = mul(x, x);
        }
    }

    return result;
}

// Числа для теста Миллера-Рабина, достаточные для проверки простоты всех 128-битных чисел 
// с вероятностью ошибки не более 4^{-k} при k итерациях.
const std::array<uint128_t, 100> MILLER_RABIN_BASES_128 = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
};

const unsigned int NUM_BASES_128 = MILLER_RABIN_BASES_128.size();

/// Класс для проверки простоты 128-битных чисел с помощью теста Миллера-Рабина.
class MillerRabin128 {
    uint128_t prime; // Число, для которого мы проверяем простоту, модуль для всех операций.
    
    uint128_t _minus_one; // prime - 1, используется для оптимизации.
    uint128_t d; // Максимальный нечётный делитель prime - 1
    int r; // prime - 1 == d * 2^r 

    Montgomery128 mont; // Montgomery-конвертор для модуля prime.
    Montgomery128::montgomery_t one_mont; // Montgomery-представление единицы для данного модуля.
    Montgomery128::montgomery_t minus_one_mont; // Montgomery-представление -1 для данного модуля.


    MillerRabin128(uint128_t n)
        : prime(n),
          _minus_one(n - 1),
          d(n - 1),
          r(0),
          mont(n),
            one_mont(mont.to_montgomery(1)),
            minus_one_mont(mont.to_montgomery(n - 1))
    {
        if (n <= 1) {
            throw std::invalid_argument("n must be greater than 1");
        }

        while ((d & 1) == 0) {
            d >>= 1;
            r++;
        }
    }

    /// Возвращает true, если a -- свидетель простоты для числа prime.
    /// Если false, то prime -- составное число.
    bool is_primality_witness(uint128_t a) const;

    public:
    static inline MillerRabin128 from_odd(uint128_t n) {
        // Представление Монтгомери требует, чтобы модуль был нечётным, так что мы гарантируем это на входе.
        if (n <= 1) {
            throw std::invalid_argument("n должно быть больше 1");
        }
        if ((n&1) == 0) {
            throw std::invalid_argument("n должно быть нечётным");
        }
        return MillerRabin128(n);
    }

    // Проверяет, является ли prime простым числом с вероятностью ошибки не более 4^{-k}.
    bool is_prime(unsigned int k) const ;
};

bool MillerRabin128::is_primality_witness(uint128_t a) const {
    a %= prime;

    if (a == 0) {
        return true;
    }

    
    // x находится в Montgomery-представлении.
    auto x = mont.exp_to_montgomery(a, d);

    if (x == one_mont || x == minus_one_mont) {
        return true;
    }

    for (int i = 0; i < r - 1; i++) {
        // Перемножение в Montgomery-представлении.
        x = mont.mul(x, x);

        if (x == minus_one_mont) {
            return true;
        }

        if (x == one_mont) {
            // нашли нетривиальный квадратный корень единицы
            return false;
        }
    }
    // по теореме Ферма, a^(n-1) == 1, то есть x^2 == 1, но при этом x != -1
    // следовательно, x - нетривиальный квадратный корень единицы.
    return false;
}

bool  MillerRabin128::is_prime(unsigned int k) const {
    if (prime <= 1) {
        return false;
    }

    if (prime <= 3) {
        return true;
    }

    if ((prime & 1) == 0) {
        return false;
    }

    if (k > NUM_BASES_128) {
        throw std::invalid_argument("Слишком большое k для количества доступных баз: " + std::to_string(NUM_BASES_128));
    }

    for (unsigned int i = 0; i < k; i++) {
        uint128_t a = MILLER_RABIN_BASES_128[i];

        if (a >= prime) {
            a %= prime;
        }

        if (a == 0) {
            continue;
        }

        if (!is_primality_witness(a)) {
            return false;
        }
    }

    return true;
}

// Малые простые числа для быстрой проверки делимости 128-битного числа 
// перед применением теста Миллера-Рабина.
const std::array<uint64_t, 25> SMALL_PRIMES = {
    3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101
};

// Предвычисляем остатки от деления 2^32 на малые простые числа
static constexpr std::array<uint64_t, 25> _mk_rems(){
    std::array<uint64_t, 25> rems{};
    for (size_t i = 0; i < SMALL_PRIMES.size(); ++i) {
        rems[i] = (1ULL << 32) % SMALL_PRIMES[i];
    }
    return rems;
}

// Предвычисленные остатки от деления 2^32 на малые простые числа для оптимизации 
// проверки делимости 128-битного числа на эти простые числа.
const std::array<uint64_t, 25> SMALL_PRIME_REMS = _mk_rems();

// Функция проверки делимости __uint128_t на малое простое p
// pos -- позиция в массиве SMALL_PRIMES, для которой мы проверяем делимость
bool is_divisible_by_small_prime(unsigned __int128 n, size_t pos) {
    uint64_t p = SMALL_PRIMES[pos];
    uint64_t R = SMALL_PRIME_REMS[pos];

    // Разбиваем 128-битное число на четыре 32-битных куска
    uint32_t a3 = (uint32_t)(n >> 96);
    uint32_t a2 = (uint32_t)(n >> 64);
    uint32_t a1 = (uint32_t)(n >> 32);
    uint32_t a0 = (uint32_t)(n);

    // Шаги схемы Горнера
    // Используем uint64_t для rem, чтобы избежать переполнения при (rem * R + ai)
    uint64_t rem = a3 % p;
    
    rem = (rem * R + a2) % p;
    rem = (rem * R + a1) % p;
    rem = (rem * R + a0) % p;

    return rem == 0;
}

/// Проверяет, является ли n простым числом с помощью быстрой проверки делимости на малые простые числа и теста Миллера-Рабина.
/// mr_iterations -- количество итераций теста Миллера-Рабина.
/// small_primes_count -- количество малых простых чисел для проверки делимости (по умолчанию -- все из SMALL_PRIMES). 
///   Если 0, то проверка делимости на малые простые числа не выполняется.
bool is_prime(uint128_t n, unsigned int mr_iterations, unsigned int small_primes_count = SMALL_PRIMES.size()) {
    if (n <= 1) {
        return false;
    }
    unsigned int count = std::min(small_primes_count, (unsigned int)SMALL_PRIMES.size());
    for (unsigned int i = 0; i < count; ++i) {
        uint32_t p = SMALL_PRIMES[i];
        if (n == p) {
            return true;
        }
        if (is_divisible_by_small_prime(n, i)) {
            return false;
        }
    }
    MillerRabin128 mr = MillerRabin128::from_odd(n);
    return mr.is_prime(mr_iterations);

}

#include <cstring>

uint128_t hex_str_to_uint128(const char* s) {
    uint128_t result = 0;
    if (strlen(s) == 0) {
        throw std::invalid_argument("Input string is empty");
    }
    for (const char* p = s; *p; p++) {
        char c = *p;
        uint8_t value;
        if (c >= '0' && c <= '9') {
            value = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            value = c - 'A' + 10;
        } else {
            throw std::invalid_argument("Invalid character in hex string");
        }
        auto _result = (result << 4) | value;
        if (_result < result) {
            throw std::overflow_error("Hex string value exceeds 128 bits");
        }
        result = _result;
    }
    return result;
}

uint128_t str_to_uint128(const char* s) {
    uint128_t result = 0;
    if (strlen(s) == 0) {
        throw std::invalid_argument("Input string is empty");
    }
    if (strlen(s) == 1) {
        if (isdigit(s[0])) {
            return s[0] - '0';
        }
        throw std::invalid_argument("Invalid character in input string");
    }
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        // Шестнадцатеричная строка
        return hex_str_to_uint128(s + 2);
    } else {
        // Десятичная строка
        for (const char* p = s; *p; p++) {
            char c = *p;
            if (c < '0' || c > '9') {
                throw std::invalid_argument("Invalid character in decimal string");
            }
            uint8_t value = c - '0';
            auto _result = result * 10 + value;
            if (_result < result) {
                throw std::overflow_error("Decimal string value exceeds 128 bits");
            }
            result = _result;
        }
    }
    return result;
}

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const uint128_t& value) {
    if (value == 0) {
        return os << '0';
    }

    // Проверим флаг hex, чтобы вывести в нужном формате
    std::ios_base::fmtflags flags = os.flags();
    if (flags & std::ios_base::hex) {
        char buffer[33]; // 128 бит = 32 шестнадцатеричных символа + 1 для нуля
        char* ptr = buffer + sizeof(buffer);
        uint128_t temp = value;
        *(--ptr) = '\0';
        while (temp > 0) {
            uint8_t digit = temp & 0xF;
            *(--ptr) = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            temp >>= 4;
        }
        return os << ptr;
    }

    char buffer[40]; // 2^128 - 1 имеет 39 цифр в десятичной записи
    char* ptr = buffer + sizeof(buffer);
    uint128_t temp = value;
    *(--ptr) = '\0';
    while (temp > 0) {
        *(--ptr) = '0' + (temp % 10);
        temp /= 10;
    }
    return os << ptr;
}

#include <getopt.h>

void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " n1 n2 [--dec|--hex]" << std::endl;
    std::cerr << "  n1   : start of the interval (inclusive)" << std::endl;
    std::cerr << "  n2   : end of the interval (exclusive)" << std::endl;
    std::cerr << "  --dec: output in decimal format (default)" << std::endl;
    std::cerr << "  --hex: output in hexadecimal format" << std::endl;
    std::cerr << "Examples:" << std::endl;
    std::cerr << "  " << prog << " 10 50" << std::endl;
    std::cerr << "  " << prog << " 0xFF 0x200 --hex" << std::endl;
    std::cerr << "  " << prog << " 10000000000000000000 10000000000000001000" << std::endl;
}

/// Структура для хранения значений опций, установленных через командную строку.
static struct option_values_t {
    bool no_output;
    bool hex_output;
    int small_primes_count;
    int miller_rabin_iterations;
} option_values = {true, false, (int)SMALL_PRIMES.size(), 64};

struct option long_options[] = {
    {"print", no_argument, nullptr, 'p'}, // печатать найденные простые числа (по умолчанию -- не печатать, только статистика)
    {"dec", no_argument, nullptr, 'd'},   // выводить найденные простые числа в десятичном формате (по умолчанию)
    {"hex", no_argument, nullptr, 'x'},   // выводить найденные простые числа в шестнадцатеричном формате
    // количество малых простых чисел для проверки делимости (по умолчанию -- все из SMALL_PRIMES)
    {"small-primes", required_argument, nullptr, 's'},
    // количество итераций теста Миллера-Рабина 
    {"iterations", required_argument, nullptr, 'i'}, 
    {nullptr, 0, nullptr, 0}
};

bool parse_arguments(int argc, char* argv[], uint128_t& n1, uint128_t& n2) {
    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "pdxs:i:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p': option_values.no_output = false; break;
            case 'd': option_values.hex_output = false; break;
            case 'x': option_values.hex_output = true; break;
            case 's': option_values.small_primes_count = std::stoi(optarg); break;
            case 'i': option_values.miller_rabin_iterations = std::stoi(optarg); break;
            case '?': break; // getopt_long already printed an error
            default: abort();
        }
    }
    const char* n1_str = nullptr;
    const char* n2_str = nullptr;
    if (optind < argc) {
        n1_str = argv[optind++];
    } else {
        std::cerr << "Error: missing n1 argument." << std::endl;
        return false;
    }
    if (optind < argc) {
        n2_str = argv[optind++];
    } else {
        std::cerr << "Error: missing n2 argument." << std::endl;
        return false;
    }
    if (!n1_str || !n2_str) {
        std::cerr << "Error: missing required arguments." << std::endl;
        return false;
    }

    try {
        n1 = str_to_uint128(n1_str);
        n2 = str_to_uint128(n2_str);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing numbers: " << e.what() << std::endl;
        return false;
    }
    if (n1 >= n2) {
        std::cerr << "Error: n1 must be less than n2." << std::endl;
        return false;
    }
    return true;
}

#include <chrono>

int main(int argc, char* argv[]) {
    uint128_t n1, n2;
    if (!parse_arguments(argc, argv, n1, n2)) {
        usage(argv[0]);
        return 1;
    }
    // Вывод заголовка
    std::cout << "# Интервал: [" << n1 << ", " << n2 << ")" << std::endl;
    if (option_values.no_output) {
        std::cout << "# Результаты не выводятся, только статистика." << std::endl;
    } else {
        std::cout << "# Формат: " << (option_values.hex_output ? "шестнадцатеричный" : "десятичный") << std::endl;
    }

    if (n1 & 1) {
        // n1 уже нечётное, можно начинать проверку с него
    } else {
        // n1 чётное, начинаем проверку со следующего нечетного числа
        n1 += 1;
    }
    unsigned int count = 0;
    auto start_time = std::chrono::steady_clock::now();
    while (n1 < n2) {
        if (is_prime(n1, option_values.miller_rabin_iterations, option_values.small_primes_count)) {
            count ++;
            if (!option_values.no_output) {
                if (option_values.hex_output) {
                    std::cout << "0x" << std::hex << n1 << std::dec << std::endl;
                } else {
                    std::cout << n1 << std::endl;
                }
            }
        }
        n1 += 2; // move to the next odd number
    }
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    // Статистика
    std::cout << "# Найдено простых чисел: " << count << std::endl;
    std::cout << "# Затраченное время: " << std::fixed << std::setprecision(6) 
          << elapsed_seconds.count() << " сек" << std::endl;
    return 0;
}
