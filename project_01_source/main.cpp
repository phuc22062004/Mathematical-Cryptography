#include <iostream>
#include <math.h>
#include <iostream>
#include <array>
#include <string>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <thread>
#include <future>
#include <vector>
using namespace std;

class BigUInt512
{
private:
    static const int SIZE = 512 / 64; // Number of 64-bit integers to store 512 bits
public:
    array<uint64_t, SIZE> data;

public:
    BigUInt512()
    {
        data.fill(0);
    }

    BigUInt512(const string &number)
    {
        data.fill(0);
        fromString(number);
    }

    void fromString(const string &number)
    {
        // Clear existing data
        data.fill(0);

        // Input validation
        if (number.empty())
        {
            return;
        }

        for (char c : number)
        {
            if (!isdigit(c))
            {
                throw invalid_argument("Invalid character in number string");
            }
        }

        // Process each digit from left to right
        for (char digit : number)
        {
            // Multiply current value by 10
            uint64_t carry = 0;
            for (int i = 0; i < SIZE; i++)
            {
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
            while (carry && i < SIZE)
            {
                uint64_t sum = data[i] + carry;
                carry = (sum < data[i]) ? 1 : 0;
                data[i] = sum;
                i++;
            }
        }
    }

    string toString() const
    {
        string result;
        BigUInt512 temp = *this;
        while (!temp.isZero())
        {
            uint64_t remainder = temp.divideBy10();
            result.push_back('0' + static_cast<char>(remainder));
        }
        if (result.empty())
        {
            result = "0";
        }
        else
        {
            reverse(result.begin(), result.end());
        }
        return result;
    }

    bool isZero() const
    {
        for (const auto &part : data)
        {
            if (part != 0)
            {
                return false;
            }
        }
        return true;
    }

    BigUInt512 operator+(const BigUInt512 &other) const
    {
        BigUInt512 result;
        uint64_t carry = 0;
        for (int i = 0; i < SIZE; ++i)
        {
            uint64_t temp = data[i] + other.data[i] + carry;
            carry = (temp < data[i]) ? 1 : 0;
            result.data[i] = temp;
        }
        return result;
    }

    uint64_t divideBy10()
    {
        uint64_t remainder = 0;
        for (int i = SIZE - 1; i >= 0; --i)
        {
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

    bool isEven() const
    {
        return (data[0] & 1) == 0;
    }

    BigUInt512 subtract(const BigUInt512 &other) const
    {
        BigUInt512 result;
        uint64_t borrow = 0;
        for (size_t i = 0; i < SIZE; ++i)
        {
            uint64_t temp = data[i] - other.data[i] - borrow;
            result.data[i] = temp;
            borrow = (data[i] < other.data[i] + borrow) ? 1 : 0;
        }
        return result;
    }

    BigUInt512 mod(const BigUInt512 &divisor) const
    {
        // Check for division by zero
        if (divisor.isZero())
        {
            throw invalid_argument("Modulo by zero");
        }

        BigUInt512 remainder;

        // Process each bit from most significant to least significant
        for (int i = SIZE * 64 - 1; i >= 0; --i)
        {
            // Shift remainder left by 1 and add next bit
            remainder = remainder << 1;
            remainder.data[0] |= (data[i / 64] >> (i % 64)) & 1;

            // If remainder >= divisor, subtract divisor
            if (remainder >= divisor)
            {
                remainder = remainder.subtract(divisor);
            }
        }

        return remainder;
    }

    BigUInt512 operator-(const BigUInt512 &other) const
    {
        return subtract(other);
    }

    BigUInt512 operator%(const BigUInt512 &divisor) const
    {
        return mod(divisor);
    }

    static void multiply_uint64(uint64_t a, uint64_t b, uint64_t &low, uint64_t &high)
    {
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

    BigUInt512 operator*(const BigUInt512 &other) const
    {
        BigUInt512 result;
        for (size_t i = 0; i < SIZE; ++i)
        {
            uint64_t carry = 0;
            for (size_t j = 0; j < SIZE - i; ++j)
            {
                uint64_t low, high;
                multiply_uint64(data[i], other.data[j], low, high);

                uint64_t sum = result.data[i + j] + low + carry;
                result.data[i + j] = sum;

                carry = high + (sum < low);
            }
        }
        return result;
    }

    BigUInt512 operator/(const BigUInt512 &divisor) const
    {
        // Check for division by zero
        if (divisor.isZero())
        {
            throw invalid_argument("Division by zero");
        }

        BigUInt512 quotient;
        BigUInt512 remainder;

        // Process each bit from most significant to least significant
        for (int i = SIZE * 64 - 1; i >= 0; --i)
        {
            // Shift remainder left by 1 and add next bit from dividend
            remainder = remainder << 1;
            remainder.data[0] |= (data[i / 64] >> (i % 64)) & 1;

            // If remainder >= divisor, subtract divisor and set quotient bit
            if (remainder >= divisor)
            {
                remainder = remainder.subtract(divisor);
                quotient.data[i / 64] |= (uint64_t)1 << (i % 64);
            }
        }

        return quotient;
    }

    BigUInt512 operator<<(int shift) const
    {
        BigUInt512 result;
        result.data.fill(0); // Initialize result array to zero
        int chunk_shift = shift / 64;
        int bit_shift = shift % 64;
        for (int i = SIZE - 1; i >= chunk_shift; --i)
        {
            result.data[i] = data[i - chunk_shift] << bit_shift;
            if (i > chunk_shift && bit_shift > 0)
            {
                result.data[i] |= data[i - chunk_shift - 1] >> (64 - bit_shift);
            }
        }
        return result;
    }

    BigUInt512 operator>>(int shift) const
    {
        BigUInt512 result;
        result.data.fill(0); // Initialize result array to zero
        int chunk_shift = shift / 64;
        int bit_shift = shift % 64;
        for (int i = 0; i < SIZE - chunk_shift; ++i)
        {
            result.data[i] = data[i + chunk_shift] >> bit_shift;
            if (i < SIZE - chunk_shift - 1 && bit_shift > 0)
            {
                result.data[i] |= data[i + chunk_shift + 1] << (64 - bit_shift);
            }
        }
        return result;
    }

    bool operator==(const BigUInt512 &other) const
    {
        for (int i = 0; i < SIZE; ++i)
        {
            if (data[i] != other.data[i])
                return false;
        }
        return true;
    }

    bool operator!=(const BigUInt512 &other) const
    {
        return !(*this == other);
    }

    bool operator>=(const BigUInt512 &other) const
    {
        for (int i = SIZE - 1; i >= 0; --i)
        {
            if (data[i] > other.data[i])
                return true;
            if (data[i] < other.data[i])
                return false;
        }
        return true;
    }

    bool operator>(const BigUInt512 &other) const
    {
        for (int i = SIZE - 1; i >= 0; --i)
        {
            if (data[i] > other.data[i])
                return true;
            if (data[i] < other.data[i])
                return false;
        }
        return false;
    }

    // Add more arithmetic operations as needed
    void randomize(int bit_size = 256)
    {
        bit_size = bit_size - 1;
        random_device rd;
        mt19937_64 gen(rd());
        uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFF);

        data.fill(0);
        int full_chunks = bit_size / 64;
        int remaining_bits = bit_size % 64;

        for (int i = 0; i < full_chunks; ++i)
        {
            data[i] = dis(gen);
        }

        if (remaining_bits > 0)
        {
            uint64_t mask = (1ULL << remaining_bits) - 1;
            data[full_chunks] = dis(gen) & mask;
        }
    }
};

BigUInt512 modular_exponentiation(BigUInt512 base, BigUInt512 exponent, const BigUInt512 &mod)
{
    BigUInt512 result("1");
    while (!exponent.isZero())
    {
        if (!exponent.isEven())
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent = exponent >> 1;
    }
    return result;
}
// Khởi tạo các hằng số zero, one và two, được dùng trong các phép toán với số nguyên lớn
const BigUInt512 zero("0");
const BigUInt512 one("1");
const BigUInt512 two("2");

// Danh sách các số nguyên tố nhỏ được dùng trong hàm getLowLevelPrime để kiểm tra tính nguyên tố sơ bộ
vector<string> first_primes = {"2", "3", "5", "7", "11", "13", "17", "19", "23", "29",
                               "31", "37", "41", "43", "47", "53", "59", "61", "67", "71",
                               "73", "79", "83", "89", "97", "101", "103",
                               "107", "109", "113", "127", "131", "137", "139",
                               "149", "151", "157", "163", "167", "173", "179",
                               "181", "191", "193", "197", "199", "211", "223",
                               "227", "229", "233", "239", "241", "251", "257",
                               "263", "269", "271", "277", "281", "283", "293",
                               "307", "311", "313", "317", "331", "337", "347", "349"};

// Hàm mulmod tính (a * b) % m một cách an toàn với các số lớn
BigUInt512 mulmod(BigUInt512 a, BigUInt512 b, BigUInt512 m)
{
    BigUInt512 result = zero;
    // Thực hiện phép nhân lũy thừa qua từng bit của b
    while (b > zero)
    {
        // Nếu bit cuối của b là 1, thêm a vào kết quả
        if (!b.isEven())
        {
            result = (result + a) % m;
        }
        // Nhân đôi a và dịch b sang phải
        a = (a * two) % m;
        b = b >> 1;
    }
    return result;
}

// Hàm getLowLevelPrime tạo số nguyên lớn có số bit chỉ định và kiểm tra tính nguyên tố sơ bộ
BigUInt512 getLowLevelPrime(int bit_size)
{
    while (true)
    {
        BigUInt512 candidate;
        candidate.randomize(bit_size); // Khởi tạo số ngẫu nhiên với bit_size

        bool is_prime = true;
        // Kiểm tra tính nguyên tố của candidate với các số nguyên tố nhỏ trong first_primes
        for (int i = 0; i < first_primes.size(); i++)
        {
            BigUInt512 prime(first_primes[i]);

            // Nếu candidate trùng với một số trong first_primes, trả về candidate
            if (candidate == prime)
                return candidate;

            // Nếu candidate chia hết cho bất kỳ số nào trong first_primes, không phải số nguyên tố
            if (candidate % prime == zero)
            {
                is_prime = false;
                break;
            }
        }
        // Nếu vượt qua kiểm tra, candidate có thể là số nguyên tố
        if (is_prime)
            return candidate;
    }
}

// Hàm trialComposite kiểm tra tính hợp lệ của một số khi thực hiện kiểm tra Miller-Rabin
bool trialComposite(BigUInt512 a, BigUInt512 d, BigUInt512 n, int s)
{
    BigUInt512 x = modular_exponentiation(a, d, n); // Tính a^d % n
    if (x == one || x == n - one)
    {
        return false; // Nếu x là 1 hoặc n - 1, không phải là hợp số
    }
    for (int r = 1; r < s; ++r)
    {
        x = mulmod(x, x, n); // Tính x^2 % n
        if (x == n - one)
            return false; // Nếu x trở thành n - 1, không phải là hợp số
    }
    return true; // Nếu không tìm thấy giá trị n - 1, n là hợp số
}

bool isProbablePrime(BigUInt512 n, int rounds = 5)
{
    if (!(n >= two))
        return false;
    if (n == two || n == BigUInt512("3"))
        return true;
    if (n.isEven())
        return false;

    BigUInt512 d = n - one;
    int s = 0;
    while (d.isEven())
    {
        d = d >> 1;
        s++;
    }

    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dist(2, 0xFFFFFFFFFFFFFFFF);

    for (int i = 0; i < rounds; ++i)
    {
        BigUInt512 a;
        a.randomize();
        a = a % (n - two) + two;

        BigUInt512 x = modular_exponentiation(a, d, n);
        if (x == one || x == n - one)
            continue;

        bool isComposite = true;
        for (int r = 1; r < s && isComposite; ++r)
        {
            x = (x * x) % n;
            if (x == n - one)
                isComposite = false;
        }
        if (isComposite)
            return false;
    }
    return true;
}

// Hàm tìm số nguyên tố tiếp theo lớn hơn hoặc bằng một số cho trước
BigUInt512 findNextPrime(const BigUInt512 &start)
{
    BigUInt512 candidate = start;

    // Nếu là số chẵn, tăng lên để chuyển sang số lẻ
    if (candidate.isEven())
    {
        candidate = candidate + BigUInt512("1");
    }

    // Tăng dần đến khi tìm thấy một số nguyên tố
    while (true)
    {
        if (isProbablePrime(candidate))
        {
            return candidate;
        }
        candidate = candidate + BigUInt512("2"); // Chỉ kiểm tra số lẻ
    }
}

BigUInt512 getBigPrime(int bit_size)
{
    while (true)
    {
        BigUInt512 candidate = getLowLevelPrime(bit_size);
        if (isProbablePrime(candidate))
            return candidate;
    }
}

BigUInt512 generate_safe_prime(int bit_size)
{
    BigUInt512 candidate;
    candidate.randomize(bit_size);

    // Tìm số nguyên tố lớn hơn hoặc bằng số ngẫu nhiên đã sinh ra
    BigUInt512 p = findNextPrime(candidate);

    // Kiểm tra điều kiện để là số nguyên tố an toàn
    while (true)
    {
        BigUInt512 q = (p - BigUInt512("1")) / BigUInt512("2");
        if (isProbablePrime(q))
        {
            return p;
        }
        p = p + BigUInt512("2"); // Tiếp tục kiểm tra số lẻ tiếp theo
        p = findNextPrime(p);    // Tìm số nguyên tố kế tiếp
    }
}

BigUInt512 gcd(const BigUInt512 &a, const BigUInt512 &b)
{
    if (b.isZero())
        return a;
    return gcd(b, a % b);
}
// Hàm tính toán giá trị tuần hoàn (x * x + 1) % n cho thuật toán Pollard's Rho
BigUInt512 pollardsRhoFunction(const BigUInt512 &x, const BigUInt512 &n)
{
    return (x * x + BigUInt512("1")) % n;
}
// Hàm sử dụng thuật toán Pollard's Rho để tìm một ước số nguyên tố của n
BigUInt512 pollardsRho(const BigUInt512 &n)
{
    if (n.isEven())
        return BigUInt512("2");

    BigUInt512 x("2"), y("2"), d("1");

    while (d == BigUInt512("1"))
    {

        x = pollardsRhoFunction(x, n);
        y = pollardsRhoFunction(pollardsRhoFunction(y, n), n);
        d = gcd((x > y ? x - y : y - x), n);
    }
    return d;
}

vector<BigUInt512> factorize(const BigUInt512 &n)
{
    vector<BigUInt512> factors;
    BigUInt512 num = n;

    BigUInt512 two("2");
    while (num.isEven())
    {
        factors.push_back(two);
        num = num / two;
    }

    if (num > BigUInt512("1"))
    {
        vector<BigUInt512> stack = {num};
        while (!stack.empty())
        {
            BigUInt512 curr = stack.back();
            stack.pop_back();

            if (isProbablePrime(curr))
            {
                factors.push_back(curr);
                // cout << curr.toString() << endl;
                continue;
            }

            BigUInt512 divisor = pollardsRho(curr);
            stack.push_back(divisor);
            stack.push_back(curr / divisor);
        }
    }

    return factors;
}

BigUInt512 findGenerator(const BigUInt512 &p)
{
    BigUInt512 one("1");
    BigUInt512 two("2");
    BigUInt512 p_minus_1 = p - one;

    vector<BigUInt512> factors = factorize(p_minus_1);

    while (true)
    {
        BigUInt512 x;
        x.randomize();
        x = x % (p - two) + two;

        bool isGenerator = true;
        for (const auto &factor : factors)
        {
            BigUInt512 z = p_minus_1 / factor;

            if (modular_exponentiation(x, z, p) == one)
            {
                isGenerator = false;
                break;
            }
        }

        if (isGenerator)
        {
            return x;
        }
    }
}

BigUInt512 generatePrivateKey(const BigUInt512 &p)
{
    BigUInt512 two("2");
    BigUInt512 x;
    x.randomize();
    x = x % (p - two) + two;
    return x;
}

int main()
{
    int bit_size = 128;
    BigUInt512 prime, g, a, b, A, B, AliceSecret, BobSecret;

    auto future_prime = async(launch::async, [&]()
                              { return generate_safe_prime(bit_size); });
    prime = future_prime.get();

    auto future_g = async(launch::async, [&]()
                          { return findGenerator(prime); });
    g = future_g.get();

    auto future_a = async(launch::async, [&]()
                          { return generatePrivateKey(prime); });
    auto future_b = async(launch::async, [&]()
                          { return generatePrivateKey(prime); });
    a = future_a.get();
    b = future_b.get();

    auto future_A = async(launch::async, [&]()
                          { return modular_exponentiation(g, a, prime); });
    auto future_B = async(launch::async, [&]()
                          { return modular_exponentiation(g, b, prime); });
    A = future_A.get();
    B = future_B.get();

    auto future_AliceSecret = async(launch::async, [&]()
                                    { return modular_exponentiation(B, a, prime); });
    auto future_BobSecret = async(launch::async, [&]()
                                  { return modular_exponentiation(A, b, prime); });
    AliceSecret = future_AliceSecret.get();
    BobSecret = future_BobSecret.get();

    cout << "Safe Prime: " << prime.toString() << endl;
    cout << "Generator (g): " << g.toString() << endl;
    cout << "Public Key of Alice (A): " << A.toString() << endl;
    cout << "Public Key of Bob (B): " << B.toString() << endl;
    cout << "Secret Key of Alice: " << AliceSecret.toString() << endl;
    cout << "Secret Key of Bob: " << BobSecret.toString() << endl;
    cout << "Are the secret keys equal? " << (AliceSecret == BobSecret ? "Yes" : "No") << endl;

    return 0;
}