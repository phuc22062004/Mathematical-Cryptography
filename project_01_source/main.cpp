#include <iostream>
#include <random>
#include <math.h>
#include <cstdint>

using namespace std;

bool isPrime(uint64_t n, int k = 20)
{
    if (n <= 1 || n % 2 == 0)
        return false;
    if (n <= 3)
        return true;
    uint64_t d = n - 1, r = 0;
    while (d % 2 == 0)
    {
        d /= 2;
        ++r;
    }

    mt19937_64 rng(random_device{}());
    uniform_int_distribution<uint64_t> dist(2, n - 2);

    for (int i = 0; i < k; i++)
    {
        uint64_t a = dist(rng);
        uint64_t x = 1, base = a, exponent = d;
        while (exponent > 0)
        {
            if (exponent % 2 == 1)
            {
                x = (x * base) % n;
            }
            base = (base * base) % n;
            exponent /= 2;
        }
        if (x == 1 || x == n - 1)
        {
            continue;
        }
        bool composite = true;
        for (int j = 0; j < r - 1; j++)
        {
            x = (x * x) % n;
            if (x == n - 1)
            {
                composite = false;
                break;
            }
        }
        if (composite)
            return false;
    }
    return true;
}

uint64_t generate_safe_prime(int bit)
{
    mt19937_64 rng(random_device{}());
    uniform_int_distribution<uint64_t> dist(1ULL << (bit - 2), (1ULL << (bit - 1) - 1));
    while (true)
    {
        uint64_t q = dist(rng);
        if (q % 2 == 0)
            q++;
        if (isPrime(q))
        {
            uint64_t p = 2 * q + 1;
            if (isPrime(p))
            {
                return p;
            }
        }
    }
}

uint64_t generate_private_key(uint64_t p)
{
    mt19937_64 rng(random_device{}());
    uniform_int_distribution<uint64_t> dist(2, p - 2);
    return dist(rng);
}

int main()
{
    int bit = 512;
    uint64_t p = generate_safe_prime(bit);
    cout << "Safe prime: " << p << endl;
    uint64_t key = generate_private_key(bit);
    cout << "Private key: " << key << endl;
}