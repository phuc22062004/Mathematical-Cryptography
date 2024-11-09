#include <iostream>
#include <math.h>

using namespace std;

// A: Trien khai ham luy thua mo-dun
//  <...> modular_exponentiation(<..> base, <..> exponent, <..> mod){

// }

// B: Trien khai ham sinh so nguyen to ngau nhien
//<..> generate_safe_prime(int bit_size){

// }

// C: Trien khai ham sinh khoa ngau nhien
//<..> generate_private_key(<..> p){
//  }

// D: Hoan thanh logic trao doi khoa Diffie-Hellman
// int main(){
// 1. Sinh so nguyen to lon hon p va phan tu sinh g
//  int bit_size = 512;

// 2. Sinh khoa rieng cua Alice va Bob

// 3. Tinh gia tri cong khai cua Alice va Bob

// 4. Tinh bi mat chung

// 5. Hien thi ket qua va xac minh rang bi mat trung khop

//}

#include <iostream>
#include <array>
#include <string>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <cstdint>

class BigUInt512 {
private:
    static const int SIZE = 512 / 64; // Number of 64-bit integers to store 512 bits
public: 
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
        // Clear existing data
        data.fill(0);
        
        // Input validation
        if (number.empty()) {
            return;
        }
        
        for (char c : number) {
            if (!std::isdigit(c)) {
                throw std::invalid_argument("Invalid character in number string");
            }
        }

        // Process each digit from left to right
        for (char digit : number) {
            // Multiply current value by 10
            uint64_t carry = 0;
            for (int i = 0; i < SIZE; i++) {
                // Split current number into high and low 32-bits for multiplication
                uint64_t low = data[i] & 0xFFFFFFFF;
                uint64_t high = data[i] >> 32;
                
                // Multiply parts by 10 and combine with previous carry
                uint64_t result_low = low * 10;
                uint64_t result_high = high * 10;
                
                // Add carry from previous iteration
                result_low += carry;
                
                // Calculate new carry and combine results
                carry = result_high + (result_low >> 32);
                data[i] = ((result_low & 0xFFFFFFFF) | ((carry & 0xFFFFFFFF) << 32));
                carry >>= 32;
            }
            
            // Add current digit
            carry = digit - '0';
            int i = 0;
            while (carry && i < SIZE) {
                uint64_t sum = data[i] + carry;
                carry = (sum < data[i]) ? 1 : 0;
                data[i] = sum;
                i++;
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

    uint64_t divideBy10() {
        uint64_t remainder = 0;
        for (int i = SIZE - 1; i >= 0; --i) {
            // Handle high 32 bits
            uint64_t high_part = remainder << 32;
            uint64_t temp_high = high_part | (data[i] >> 32);
            uint64_t q_high = temp_high / 10;
            remainder = temp_high % 10;

            // Handle low 32 bits
            uint64_t low_part = remainder << 32;
            uint64_t temp_low = low_part | (data[i] & 0xFFFFFFFF);
            uint64_t q_low = temp_low / 10;
            remainder = temp_low % 10;

            // Combine results
            data[i] = (q_high << 32) | q_low;
        }
        return remainder;
    }

    void randomize(int bit_size) {

        auto now = std::chrono::system_clock::now();
        auto seed = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::mt19937_64 gen(seed);

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
            borrow = (data[i] < other.data[i] + borrow) ? 1 : 0;
        }
        return result;
    }

    BigUInt512 mod(const BigUInt512& divisor) const {
        // Check for division by zero
        if (divisor.isZero()) {
            throw std::invalid_argument("Modulo by zero");
        }

        BigUInt512 remainder;
        
        // Process each bit from most significant to least significant
        for (int i = SIZE * 64 - 1; i >= 0; --i) {
            // Shift remainder left by 1 and add next bit
            remainder = remainder << 1;
            remainder.data[0] |= (data[i / 64] >> (i % 64)) & 1;
            
            // If remainder >= divisor, subtract divisor
            if (remainder >= divisor) {
                remainder = remainder.subtract(divisor);
            }
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
        result.data.fill(0);

        for (int i = 0; i < SIZE; i++) {
            if (data[i] == 0) continue;

            uint64_t carry = 0;
            for (int j = 0; j < SIZE - i; j++) {
                // Perform 64-bit multiplication
                uint64_t a = data[i];
                uint64_t b = other.data[j];

                // Split into 32-bit parts
                uint64_t a_low = a & 0xFFFFFFFF;
                uint64_t a_high = a >> 32;
                uint64_t b_low = b & 0xFFFFFFFF;
                uint64_t b_high = b >> 32;

                // Calculate partial products
                uint64_t low = a_low * b_low;
                uint64_t mid1 = a_low * b_high;
                uint64_t mid2 = a_high * b_low;
                uint64_t high = a_high * b_high;

                // Combine results
                uint64_t temp = result.data[i + j] + low + (mid1 << 32) + (mid2 << 32) + carry;
                carry = high + (mid1 >> 32) + (mid2 >> 32) + (temp < result.data[i + j]);

                result.data[i + j] = temp;
            }
        }
        return result;
    }

    BigUInt512 operator/(const BigUInt512& divisor) const {
        // Check for division by zero
        if (divisor.isZero()) {
            throw std::invalid_argument("Division by zero");
        }

        BigUInt512 quotient;
        BigUInt512 remainder;

        // Process each bit from most significant to least significant
        for (int i = SIZE * 64 - 1; i >= 0; --i) {
            // Shift remainder left by 1 and add next bit from dividend
            remainder = remainder << 1;
            remainder.data[0] |= (data[i / 64] >> (i % 64)) & 1;

            // If remainder >= divisor, subtract divisor and set quotient bit
            if (remainder >= divisor) {
                remainder = remainder.subtract(divisor);
                quotient.data[i / 64] |= (uint64_t)1 << (i % 64);
            }
        }

        return quotient;
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
    int bit_size = 128; // You can change this to any bit size you need
    BigUInt512 prime = generateRandomPrime(bit_size);
    //BigUInt512 g.randomize() ;
    //std::cout << "Random Number: " << g.toString() << std::endl;
    std::cout << "Random Prime Number: " << prime.toString() << std::endl;
    return 0;
}