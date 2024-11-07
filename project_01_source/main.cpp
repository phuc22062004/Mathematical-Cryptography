

#include <iostream>
#include <cstdint>
#include <vector>
#include <random>
#include <bitset>
#include <chrono>

// A: Trien khai ham luy thua mo-dun
//  <...> modular_exponentiation(<..> base, <..> exponent, <..> mod){

// }

uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t res = 0;
    a %= m;
    while (b > 0) {
        if (b & 1)
            res = (res + a) % m;
        a = (a * 2) % m;
        b >>= 1;
    }
    return res;
}

uint64_t powMod(uint64_t a, uint64_t b, uint64_t n) {
    uint64_t result = 1;
    a %= n;
    while (b > 0) {
        if (b & 1)
            result = mulmod(result, a, n);
        a = mulmod(a, a, n);
        b >>= 1;
    }
    return result;
}
std::vector<int> first_primes = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
                                    31, 37, 41, 43, 47, 53, 59, 61, 67,
                                    71, 73, 79, 83, 89, 97, 101, 103,
                                    107, 109, 113, 127, 131, 137, 139,
                                    149, 151, 157, 163, 167, 173, 179,
                                    181, 191, 193, 197, 199, 211, 223,
                                    227, 229, 233, 239, 241, 251, 257,
                                    263, 269, 271, 277, 281, 283, 293,
                                    307, 311, 313, 317, 331, 337, 347, 349 };

bool trialComposite(uint64_t a, uint64_t evenC, uint64_t to_test, int max_div_2) {
    if (powMod(a, evenC, to_test) == 1)
        return false;
 
    for (int i = 0; i < max_div_2; i++) {
        uint64_t temp = static_cast<uint64_t>(1) << i;
        if (powMod(a, temp * evenC, to_test) == to_test - 1)
            return false;
    }
 
    return true;
}

bool MillerRabinTest(uint64_t to_test) {
    constexpr int accuracy = 20;
 
    int max_div_2 = 0;
    uint64_t evenC = to_test - 1;
    while (evenC % 2 == 0) {
        evenC >>= 1;
        max_div_2++;
    }
 
    // random numbers init
    auto now = std::chrono::system_clock::now();
    auto seed = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<uint64_t> dist(2, to_test - 2);
 
    for (int i = 0; i < accuracy; i++) {
        uint64_t a = dist(gen);
 
        if (trialComposite(a, evenC, to_test, max_div_2)) {
            return false;
        }
    }
 
    return true;
}

// B: Trien khai ham sinh so nguyen to ngau nhien
//<..> generate_safe_prime(int bit_size){

// }

uint64_t generate_random_odd(int bit_size) {
    auto now = std::chrono::system_clock::now();
    auto seed = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    uint64_t num = dist(gen);

    // Điều chỉnh số bit cho đúng với bit_size
    num |= (1ULL << (bit_size - 1)); // Đảm bảo bit cao nhất là 1
    num |= 1; // Đảm bảo số lẻ
    return num;
}

uint64_t getLowLevelPrime(int bit_size) {
    while (true) {
        uint64_t candidate = generate_random_odd(bit_size);
        bool is_prime = true;
        for (int i = 0; i < first_primes.size(); i++) {
            if (candidate == first_primes[i])
                return candidate;
 
            if (candidate % first_primes[i] == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime)
            return candidate;
    }
}

uint64_t getBigPrime(int bit_size) {
    while (true) {
        uint64_t candidate = getLowLevelPrime(bit_size);
        if (MillerRabinTest(candidate))
            return candidate;
    }
}

uint64_t generate_safe_prime(int bit_size) {
    while (true) {
        uint64_t q = getBigPrime(bit_size - 1); // Tạo số nguyên tố q với bit_size - 1
        uint64_t p = 2 * q + 1;
        if (MillerRabinTest(p)) {
            return p;
        }
    }
}




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
int main() {
    int bit_size;
    std::cout << "Nhập số bit cho safe prime: ";
    std::cin >> bit_size;

    uint64_t safe_prime = generate_safe_prime(bit_size);
    std::cout << "Safe prime (" << bit_size << " bit): " << safe_prime << std::endl;

    return 0;
}