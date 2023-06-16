#ifndef BASETYPES_H
#define BASETYPES_H

#include <locale>
#include <stdio.h>  // for printf()
#include <stdlib.h> // for strtol()
#include <errno.h>  // for errno
#include <limits.h> // for INT_MIN and INT_MAX
#include <string>
#include <vector>
#include <array>
#include <list> 
using namespace std;
#define TRUE 1
#define FALSE 0
#define VBMIN(a,b) (((a)<(b))?(a):(b))
#define VBMAX(a,b) (((a)>(b))?(a):(b))

#define lvoid [this]()

#if defined(_MSC_VER)
//  Microsoft 
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
//  do nothing and hope for the best?
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

static inline void BREAKPOINT(void) {};
#ifndef WIN32
#ifndef ZeroMemory
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif
typedef unsigned char BOOL;				// 8 bit unsigned
typedef unsigned char BYTE;				// 8 bit unsigned
typedef unsigned short int WORD;		//16 bit unsigned
typedef unsigned long int DWORD;		//32 bit unsigned
#endif

typedef unsigned long long int QWORD;	//64 bit unsigned
typedef unsigned int SWORD;				//16 bit signed
typedef unsigned long int SDWORD;		//32 bit signed
typedef unsigned long long int SQWORD;	//64 bit signed
typedef float FLOAT;

typedef std::vector<std::vector<BYTE>> t_grid;

typedef std::string vbString;

#endif // !INCLUDES_H
