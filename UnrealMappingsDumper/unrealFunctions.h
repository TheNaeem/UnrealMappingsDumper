#pragma once

#include "unrealContainers.h"

typedef void (*_FNameToString)(const void* pThis, FString& Out);
inline _FNameToString FNameToString;