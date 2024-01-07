//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
// 
//=========================================================================================================

#pragma once

#include "tchar.h"
#include <string>

typedef  long long			 Z_INT;
typedef  unsigned long long Z_UINT;
                            
typedef          long		 Z_INT32;
typedef unsigned long		 Z_UINT32;
                            
typedef          long long  Z_INT64;
typedef unsigned long long  Z_UINT64;
                            
typedef          double     Z_DOUBLE;
typedef          float      Z_FLOAT;
typedef          char       Z_INT8;
typedef unsigned char       Z_UINT8;
typedef          short      Z_INT16;
typedef unsigned short      Z_UINT16;

typedef wchar_t             Z_TEXT_CHAR;

typedef std::wstring Z_STRING;

#define Z_TEXT(text) L##text

#define tcerr wcerr
#define tcout wcout
#define tstrlen wcslen
