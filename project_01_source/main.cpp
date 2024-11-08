#include <iostream>
#include <math.h>
#include <cstdint>
using namespace std;

uint64_t modular_exponentiation(uint64_t base, uint64_t exponent, uint64_t mod) {
    uint64_t result = 1;
    base = base % mod;  

    while (exponent > 0) {
        // Nếu exponent là lẻ, nhân kết quả với base
        if (exponent % 2 == 1) {
            result = (result * base) % mod;
        }
        // Chia đôi exponent (shift left 1 bit)
        exponent = exponent >> 1;
        // Bình phương base và lấy mô-đun
        base = (base * base) % mod;
    }

    return result;
}

int main() {
    uint64_t base = 1234567890;
    uint64_t exponent = 9876543210;
    uint64_t mod = 1000000071;

    uint64_t result = modular_exponentiation(base, exponent, mod);
    cout << "Result: " << result << endl;

    return 0;
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
