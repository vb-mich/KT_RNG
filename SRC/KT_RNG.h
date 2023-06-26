#ifndef _RANDOM_GEN_H  
#define _RANDOM_GEN_H
#include "basetypes.h"
#include <stdint.h>
#include <climits>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include <stdio.h>
#include <fcntl.h>

#ifdef LINUX
#include <unistd.h>
#endif


/* Period parameters */
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */

extern "C"
{
	EXPORT void startRNG(unsigned int seed);
	EXPORT unsigned int getRandom(unsigned int limit);
	EXPORT void shuffleDeck(uint8_t* data, uint32_t length);
}


#endif 
