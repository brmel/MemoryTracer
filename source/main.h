//=========================================================================================================
// Author : Brahim Redouane Mellah
// Year   : 2023
//
//=========================================================================================================

#pragma once

#include <filesystem>
#include <algorithm>
#include "../include/os.h"

#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <chrono>
#include <thread>


// TODO: reference additional headers your program requires here
#include <string>
#include <list>
#include <vector>
#include <map>
#include <optional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#define DPSAPI_VERSION 1
#include <psapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "psapi.lib")

// From MSDN sample. Eww.
#define MAX_NAME 256