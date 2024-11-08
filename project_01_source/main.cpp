

#include <iostream>
#include <cstdint>
#include <vector>
#include <random>
#include <bitset>
#include <chrono>
#include <array>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>

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
    static void multiply_uint64(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
        uint64_t a_low = static_cast<uint32_t>(a);
        uint64_t a_high = a >> 32;
        uint64_t b_low = static_cast<uint32_t>(b);
        uint64_t b_high = b >> 32;

        uint64_t low_low = a_low * b_low;
        uint64_t low_high = a_low * b_high;
        uint64_t high_low = a_high * b_low;
        uint64_t high_high = a_high * b_high;

        uint64_t carry = (low_low >> 32) + static_cast<uint32_t>(low_high) + static_cast<uint32_t>(high_low);
        low = (low_low & 0xFFFFFFFF) | (carry << 32);
        high = high_high + (low_high >> 32) + (high_low >> 32) + (carry >> 32);
    }

    BigUInt512 operator*(const BigUInt512& other) const {
        BigUInt512 result;
        for (size_t i = 0; i < SIZE; ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < SIZE - i; ++j) {
                uint64_t low, high;
                multiply_uint64(data[i], other.data[j], low, high);

                // Cộng low vào kết quả hiện tại và carry
                uint64_t sum = result.data[i + j] + low + carry;
                result.data[i + j] = sum;

                // Xác định carry mới
                carry = high + (sum < low);  // Xử lý carry do tràn khi cộng
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
    bool isOdd() const {
        return (data[0] & 1) == 1;  // Kiểm tra bit thấp nhất
    }
    void randomize(int bit_size=512) { 
        bit_size = bit_size - 1;
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
};

const BigUInt512 zero("0");
const BigUInt512 one("1");
const BigUInt512 two("2");

std::vector<std::string> first_primes = { "2", "3", "5", "7", "11", "13", "17", "19", "23", "29", 
                                          "31", "37", "41", "43", "47", "53", "59", "61", "67", "71",
                                          "73", "79", "83", "89", "97", "101", "103",
                                          "107", "109", "113", "127", "131", "137", "139",
                                          "149", "151", "157", "163", "167", "173", "179",
                                          "181", "191", "193", "197", "199", "211", "223",
                                          "227", "229", "233", "239", "241", "251", "257",
                                          "263", "269", "271", "277", "281", "283", "293",
                                          "307", "311", "313", "317", "331", "337", "347", "349" };
BigUInt512 mulmod(BigUInt512 a, BigUInt512 b, BigUInt512 m){
    BigUInt512 res = zero;
    while (a != zero)
    {
        if(a.isOdd()){
            res = (res + b) % m;
        }
        a = a >> 1;
        b = (b*two) % m;
    }
    return res;
}

BigUInt512 powMod(BigUInt512 a, BigUInt512 b, BigUInt512 n){
    BigUInt512 x = one;
    a = a % n;
    while (b >= one)
    {
        if (b%two == one)
        {
            x = mulmod(x, a, n);
            b = b - one;
        }
        a = mulmod(a, a, n);
        b = b >> 1;
    }
    return x;
}

BigUInt512 getLowLevelPrime(int bits_size){
    while (true)
    {
        BigUInt512 candidate;
        candidate.randomize(bits_size);
        bool is_prime = true;
        for (int i = 0; i < first_primes.size(); i++)
        {
            BigUInt512 prime(first_primes[i]);
            if (candidate == prime)
                return candidate;

            if(candidate % prime == zero){
                is_prime = false;
                break;
            } 
        }
        if (is_prime)
            return candidate;
    }
}

bool trialComposite(BigUInt512 a, BigUInt512 d, BigUInt512 n, int s) {
    BigUInt512 x = powMod(a, d, n);
    if (x == one || x == n - one) {
        return false; // Not composite
    }
    for (int r = 1; r < s; ++r) {
        x = mulmod(x, x, n);
        if (x == n - one) return false; // Not composite
    }
    return true; // Composite
}

bool isProbablePrime(BigUInt512 n, int rounds = 5) {
    if (!(n >= two)) return false;
    if (n == two || n == BigUInt512("3")) return true;
    if (n.isEven()) return false;

    // Write (n - 1) as d * 2^s
    BigUInt512 d = n - one;
    int s = 0;
    while (d.isEven()) {
        d = d >> 1;
        s++;
    }

    // Perform rounds of Miller-Rabin primality test
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(2, 0xFFFFFFFFFFFFFFFF);

    for (int i = 0; i < rounds; ++i) {
        BigUInt512 a;
        a.randomize();
        a = a % (n-two) + two;
        if (trialComposite(a, d, n, s)) {
            return false; // Composite
        }
    }
    return true; // Probably prime
}


BigUInt512 getBigPrime(int bits_size){
    while (true)
    {
        BigUInt512 candidate = getLowLevelPrime(bits_size);
        if(isProbablePrime(candidate))
            return candidate;
    }
}



int main(){
    int bits_size = 512;
    BigUInt512 temp = getBigPrime(bits_size);
    std::cout<< temp.toString();
}


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
