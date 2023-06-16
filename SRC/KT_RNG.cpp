#include "KT_RNG.h"
std::mutex mtMutex;
std::thread controlThread;
bool initialized = false;

#define CYCLE_MILLISECONDS 500
#define RESET_DAYS 45
#define CYCLE_RESET (((DWORD)RESET_DAYS * (DWORD)86400000)/((DWORD)CYCLE_MILLISECONDS))

static unsigned long long mt[NN];
static int mti = NN + 1;

int clzll(unsigned long long x) {
    // Initialize the count to the size of the long long type in bits
    int count = CHAR_BIT * sizeof(unsigned long long);
    // Shift the bits of x right until a set bit is found
    while ((x & (1ull << (count - 1))) == 0 && count > 0) {
        --count;
    }
    return count;
}

void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti=1; mti<NN; mti++) 
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

void init_by_array64(unsigned long long init_key[],
    unsigned long long key_length)
{
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i = 1; j = 0;
    k = (NN > key_length ? NN : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * 3935559000370003845ULL))
            + init_key[j] + j; 
        i++; j++;
        if (i >= NN) { mt[0] = mt[NN - 1]; i = 1; }
        if (j >= key_length) j = 0;
    }
    for (k = NN - 1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 62)) * 2862933555777941757ULL))
            - i;
        i++;
        if (i >= NN) { mt[0] = mt[NN - 1]; i = 1; }
    }

    mt[0] = 1ULL << 63; 
}

unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2] = { 0ULL, MATRIX_A };

    if (mti >= NN) { 

        if (mti == NN + 1)
            init_genrand64(5489ULL);

        for (i = 0; i < NN - MM; i++) {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + MM] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        for (; i < NN - 1; i++) {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + (MM - NN)] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        x = (mt[NN - 1] & UM) | (mt[0] & LM);
        mt[NN - 1] = mt[MM - 1] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];

        mti = 0;
    }

    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}


void KT_RNG_discardNumbers(unsigned int howMany)
{
    mtMutex.lock();
    for (size_t i = 0; i < howMany; i++)
    {
        genrand64_int64();
    }
    mtMutex.unlock();
}

/// <summary>
/// Matrix initialization
/// </summary>
/// <param name="seed">pRNG seed</param>
void KT_RNG_init(unsigned long long seed)
{
    // Virtual memory address depends on unpredictable factors depending on the OS kernel
    // and memory usage by other processes
    unsigned int* giveMeRandomAddress = (unsigned int*)malloc(sizeof(unsigned int));
    unsigned long long memory_entropy = (unsigned long long)giveMeRandomAddress;
    free(giveMeRandomAddress);

    // Monotonic time that is NOT related to the wall clock system time, it is related to unknown factors such as the last machine reboot
    // https://en.cppreference.com/w/cpp/chrono/steady_clock
    unsigned long long time_entropy = (unsigned long long)std::chrono::steady_clock::now().time_since_epoch().count();
    
    // Using also the machine's random device own implementation, 
    // which is not required to be nondeterministic but still adds unknown bits
    std::random_device rd;

    unsigned long long seeds_array[8] = {seed, memory_entropy, time_entropy, rd(), rd(), rd(), rd(), rd()};

    mtMutex.lock();
    init_by_array64(seeds_array, 8);
    mtMutex.unlock();
}

/// <summary>
/// Every n milliseconds consumes one random number
/// re-seed after fixed amount of time using a random number from the actual state of the RNG and re-applying entropy
/// </summary>
/// <param name="_RNG"></param>
void KT_RNG_controlCycle()
{
    QWORD reset = 0;
    while (true)
    {
        KT_RNG_discardNumbers(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(CYCLE_MILLISECONDS));
        reset++;
        if (reset >= CYCLE_RESET)
        {
            KT_RNG_init(genrand64_int64());
            reset = 0;
        }
    }
}

/// <summary>
/// Generate a random number between 0 and (2^32)-1
/// </summary>
/// <returns>A random number</returns>
unsigned int KT_RNG_getRandom(unsigned int limit)
{
    unsigned int* giveMeRandomAddress = (unsigned int*)malloc(sizeof(unsigned int));
    unsigned long long memory_entropy = (unsigned long long)giveMeRandomAddress;
    free(giveMeRandomAddress);
    BYTE discards = 1 + (memory_entropy % 3); //self-discard some amount of numbers

    KT_RNG_discardNumbers(discards);

    mtMutex.lock();

    unsigned int r = 0;
    int remaining_bits = CHAR_BIT * sizeof(uint32_t);
    while (remaining_bits > 0) {
        uint64_t t = genrand64_int64();
        r = (r << remaining_bits) | (t >> (64 - remaining_bits));
        int bits_used = 64 - clzll(t);
        remaining_bits -= bits_used;
    }

    while (r >= limit) {
        r = r % limit;
    }

    mtMutex.unlock();
    return r;
}

extern "C"
{
    void startRNG(unsigned int seed)
    {
        if (!initialized)
        {
            KT_RNG_init(seed);
            controlThread = std::thread(KT_RNG_controlCycle);
            initialized = true;
        }
    }

    unsigned int getRandom(unsigned int limit)
    {
        return KT_RNG_getRandom(limit);
    }
}

int main(int argc, char* argv[])
{
    startRNG(time(NULL));
    for (size_t i = 0; i < 100000000000; i++)
    {
        printf(to_string(KT_RNG_getRandom(0xFFFFFFFF)).c_str());
        printf("\n");

    }
    return 0;
}