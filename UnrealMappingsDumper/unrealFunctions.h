#pragma once

#include "unrealContainers.h"

#define DECLARE_UE_FUNC(name, returnVal, params) \
	typedef returnval (*_name)(params); \
	inline _name name; \

//DECLARE_UE_FUNC(FNameToString, void, (const void* pThis, FString& Out)); 

typedef void (*_FNameToString)(const void* pThis, FString& Out);
inline _FNameToString FNameToString;