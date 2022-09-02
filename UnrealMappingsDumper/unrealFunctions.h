#pragma once

#include "unrealContainers.h"

typedef void (*_FNameToString)(void* pThis, FString& Out);
inline _FNameToString FNameToString;