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

    uint64_t divideBy10() {
        uint64_t remainder = 0;
        for (int i = SIZE - 1; i >= 0; --i) {
            uint64_t temp = (remainder << 64) | data[i];
            data[i] = temp / 10;
            remainder = temp % 10;
        }
        return remainder;
    }

    BigUInt512 operator+(const BigUInt512& other) const {
        BigUInt512 result;
        uint64_t carry = 0;
        for (int i = 0; i < SIZE; ++i) {
            uint64_t temp = data[i] + other.data[i] + carry;
            carry = (temp < data[i]) ? 1 : 0;
            result.data[i] = temp;
        }
        return result;
    }

    BigUInt512 operator+(uint64_t value) const {
        BigUInt512 result = *this;
        uint64_t carry = value;
        for (int i = 0; i < SIZE; ++i) {
            uint64_t temp = result.data[i] + carry;
            carry = (temp < result.data[i]) ? 1 : 0;
            result.data[i] = temp;
            if (carry == 0) break;
        }
        return result;
    }

    BigUInt512 subtract(const BigUInt512& other) const {
        BigUInt512 result;
        uint64_t borrow = 0;
        for (size_t i = 0; i < SIZE; ++i) {
            uint64_t temp = data[i] - other.data[i] - borrow;
            result.data[i] = temp;
            borrow = (data[i] < other.data[i] + borrow) ? 1 : 0;
        }
        return result;
    }

    BigUInt512 operator<<(int shift) const {
        BigUInt512 result;
        result.data.fill(0); // Initialize result array to zero
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
        result.data.fill(0); // Initialize result array to zero
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

    bool operator==(const BigUInt512& other) const {
        for (int i = 0; i < SIZE; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    bool operator!=(const BigUInt512& other) const {
        return !(*this == other);
    }

    bool operator>=(const BigUInt512& other) const {
        for (int i = SIZE - 1; i >= 0; --i) {
            if (data[i] > other.data[i]) return true;
            if (data[i] < other.data[i]) return false;
        }
        return true;
    }

    BigUInt512 operator*(const BigUInt512& other) const {
        BigUInt512 result;
        for (int i = 0; i < SIZE; ++i) {
            uint64_t carry = 0;
            for (int j = 0; j < SIZE - i; ++j) {
                uint64_t temp = (uint64_t)data[i] * other.data[j] + result.data[i + j] + carry;
                result.data[i + j] = (uint64_t)temp;
                carry = (uint64_t)(temp >> 64);
            }
        }
        return result;
    }

    BigUInt512 operator%(const BigUInt512& other) const {
        BigUInt512 result = *this;
        BigUInt512 divisor = other;
        BigUInt512 quotient;
        BigUInt512 remainder;

        for (int i = SIZE * 64 - 1; i >= 0; --i) {
            remainder = remainder << 1;
            remainder.data[0] |= (result.data[i / 64] >> (i % 64)) & 1;
            if (remainder >= divisor) {
                remainder = remainder.subtract(divisor);
                quotient.data[i / 64] |= (uint64_t)1 << (i % 64);
            }
        }
        return remainder;
    }

    BigUInt512 operator/(const BigUInt512& other) const {
        BigUInt512 result;
        BigUInt512 divisor = other;
        BigUInt512 quotient;
        BigUInt512 remainder;

        for (int i = SIZE * 64 - 1; i >= 0; --i) {
            remainder = remainder << 1;
            remainder.data[0] |= (data[i / 64] >> (i % 64)) & 1;
            if (remainder >= divisor) {
                remainder = remainder.subtract(divisor);
                quotient.data[i / 64] |= (uint64_t)1 << (i % 64);
            }
        }
        return quotient;
    }

    bool isZero() const {
        for (int i = 0; i < SIZE; ++i) {
            if (data[i] != 0) return false;
        }
        return true;
    }

    bool getLeastSignificantBit() const {
        return data[0] & 1;
    }
    // Add more arithmetic operations as needed

    std::string toString() const {
        std::string result;
        BigUInt512 temp = *this;
        BigUInt512 zero("0");
        BigUInt512 ten("10");

        if (temp == zero) {
            return "0";
        }

        while (temp != zero) {
            BigUInt512 remainder = temp % ten;
            result += std::to_string(remainder.data[0]);
            temp = temp / ten;
        }

        std::reverse(result.begin(), result.end());
        return result;
    }
};

BigUInt512 modular_exponentiation(BigUInt512 base, BigUInt512 exponent, const BigUInt512& mod) {
    BigUInt512 result("1");
    base = base % mod;
    while (!exponent.isZero()) {
        if (exponent.getLeastSignificantBit()) {
            result = (result * base) % mod;
        }
        exponent = exponent >> 1;
        base = (base * base) % mod;
    }
    return result;
}

// Example usage
int main() {
    BigUInt512 num1("1234567890123456789012345678901234567890");
    BigUInt512 num2("9876543210987654321098765432109876543210");
    BigUInt512 sum = num1 + num2;
    std::cout << "Sum: " << sum.toString() << std::endl;

    BigUInt512 mod("10000000000000000000000000000000000000000");
    BigUInt512 base("2");
    BigUInt512 exponent("10");
    BigUInt512 result = modular_exponentiation(base, exponent, mod);
    std::cout << "Modular Exponentiation Result: " << result.toString() << std::endl;

    return 0;
}