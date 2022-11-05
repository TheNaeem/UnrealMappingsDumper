#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
// Windows Header Files
#include <windows.h>
#include <stdio.h>
#include <cstdint>
#include <string>
#include <functional>
#include <sstream>
#include <vector>
#include <filesystem>
#include "../Dependencies/parallel_hashmap/phmap.h"