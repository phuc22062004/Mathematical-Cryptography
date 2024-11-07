#include <iostream>
#include <array>
#include <string>
#include <algorithm>
#include <random>

class BigUInt512 {
private:
    static const int SIZE = 512 / 64; // Number of 64-bit integers to store 512 bits
    std::array<uint64_t, SIZE> data;

public:
    BigUInt512() {
        data.fill(0);
    }

    BigUInt512(const std::string& number) {
        data.fill(0);
        fromString(number);
    }

    void fromString(const std::string& number) {
        std::string num = number;
        std::reverse(num.begin(), num.end());
        for (size_t i = 0; i < num.size(); ++i) {
            int digit = num[i] - '0';
            for (size_t j = 0; j < SIZE; ++j) {
                uint64_t carry = 0;
                for (size_t k = 0; k < SIZE; ++k) {
                    uint64_t temp = data[k] * 10 + carry;
                    if (k == j) {
                        temp += digit;
                    }
                    carry = temp >> 64;
                    data[k] = temp & 0xFFFFFFFFFFFFFFFF;
                }
            }
        }
    }

    std::string toString() const {
        std::string result;
        BigUInt512 temp = *this;
        while (!temp.isZero()) {
            uint64_t remainder = temp.divideBy10();
            result.push_back('0' + remainder);
        }
        if (result.empty()) {
            result = "0";
        } else {
            std::reverse(result.begin(), result.end());
        }
        return result;
    }

    bool isZero() const {
        for (const auto& part : data) {
            if (part != 0) {
                return false;
            }
        }
        return true;
    }

    uint64_t divideBy10() {
        uint64_t remainder = 0;
        for (int i = SIZE - 1; i >= 0; --i) {
            uint64_t temp = (remainder << 64) | data[i];
            data[i] = temp / 10;
            remainder = temp % 10;
        }
        return remainder;
    }

    void randomize(int bit_size) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFF);

        data.fill(0);
        int full_chunks = bit_size / 64;
        int remaining_bits = bit_size % 64;

        for (int i = 0; i < full_chunks; ++i) {
            data[i] = dis(gen);
        }

        if (remaining_bits > 0) {
            uint64_t mask = (1ULL << remaining_bits) - 1;
            data[full_chunks] = dis(gen) & mask;
        }
    }

    bool isEven() const {
        return (data[0] & 1) == 0;
    }

    BigUInt512 subtract(const BigUInt512& other) const {
        BigUInt512 result;
        uint64_t borrow = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            uint64_t temp = data[i] - other.data[i] - borrow;
            result.data[i] = temp;
            borrow = (temp > data[i]) ? 1 : 0;
        }
        return result;
    }

    BigUInt512 mod(const BigUInt512& divisor) const {
        BigUInt512 remainder;
        for (int i = SIZE - 1; i >= 0; --i) {
            remainder = (remainder << 64) + data[i];
            remainder = remainder - (divisor * (remainder / divisor));
        }
        return remainder;
    }

    BigUInt512 operator-(const BigUInt512& other) const {
        return subtract(other);
    }

    BigUInt512 operator%(const BigUInt512& divisor) const {
        return mod(divisor);
    }

    BigUInt512 operator*(const BigUInt512& other) const {
        BigUInt512 result;
        for (size_t i = 0; i < SIZE; ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < SIZE - i; ++j) {
                __uint128_t temp = static_cast<__uint128_t>(data[i]) * other.data[j] + result.data[i + j] + carry;
                result.data[i + j] = static_cast<uint64_t>(temp);
                carry = static_cast<uint64_t>(temp >> 64);
            }
        }
        return result;
    }

    BigUInt512 operator/(const BigUInt512& divisor) const {
        BigUInt512 quotient, remainder;
        for (int i = SIZE * 64 - 1; i >= 0; --i) {
            remainder = (remainder << 1) + ((data[i / 64] >> (i % 64)) & 1);
            if (remainder >= divisor) {
                remainder = remainder - divisor;
                quotient.data[i / 64] |= (1ULL << (i % 64));
            }
        }
        return quotient;
    }

    BigUInt512 operator<<(int shift) const {
        BigUInt512 result;
        int chunk_shift = shift / 64;
        int bit_shift = shift % 64;
        for (int i = SIZE - 1; i >= chunk_shift; --i) {
            result.data[i] = data[i - chunk_shift] << bit_shift;
            if (i > chunk_shift && bit_shift > 0) {
                result.data[i] |= data[i - chunk_shift - 1] >> (64 - bit_shift);
            }
        }
        return result;
    }

    BigUInt512 operator>>(int shift) const {
        BigUInt512 result;
        int chunk_shift = shift / 64;
        int bit_shift = shift % 64;
        for (int i = 0; i < SIZE - chunk_shift; ++i) {
            result.data[i] = data[i + chunk_shift] >> bit_shift;
            if (i < SIZE - chunk_shift - 1 && bit_shift > 0) {
                result.data[i] |= data[i + chunk_shift + 1] << (64 - bit_shift);
            }
        }
        return result;
    }

    bool operator>=(const BigUInt512& other) const {
        for (int i = SIZE - 1; i >= 0; --i) {
            if (data[i] > other.data[i]) return true;
            if (data[i] < other.data[i]) return false;
        }
        return true;
    }

    bool operator==(const BigUInt512& other) const {
        for (int i = 0; i < SIZE; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    bool operator!=(const BigUInt512& other) const {
        return !(*this == other);
    }

    // Add more arithmetic operations as needed
};

BigUInt512 modular_exponentiation(BigUInt512 base, BigUInt512 exponent, const BigUInt512& mod) {
    BigUInt512 result("1");
    base = base % mod;
    while (!exponent.isZero()) {
        if (exponent.data[0] & 1) {
            result = (result * base) % mod;
        }
        exponent = exponent >> 1;
        base = (base * base) % mod;
    }
    return result;
}

bool isPrime(const BigUInt512& number, int iterations = 5) {
    if (number.isZero() || number.isEven()) return false;

    BigUInt512 one("1");
    BigUInt512 two("2");
    BigUInt512 d = number - one;
    int s = 0;

    while (d.isEven()) {
        d = d >> 1;
        ++s;
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(2, 0xFFFFFFFFFFFFFFFF);

    for (int i = 0; i < iterations; ++i) {
        BigUInt512 a;
        a.randomize(512);
        a = a % (number - two) + two;

        BigUInt512 x = modular_exponentiation(a, d, number);
        if (x == one || x == number - one) continue;

        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = modular_exponentiation(x, two, number);
            if (x == number - one) {
                composite = false;
                break;
            }
        }

        if (composite) return false;
    }

    return true;
}

BigUInt512 generateRandomPrime(int bit_size) {
    BigUInt512 prime;
    do {
        prime.randomize(bit_size);
    } while (!isPrime(prime));
    return prime;
}

int main() {
    int bit_size = 512; // You can change this to any bit size you need
    BigUInt512 prime = generateRandomPrime(bit_size);
    std::cout << "Random Prime Number: " << prime.toString() << std::endl;
    return 0;
}