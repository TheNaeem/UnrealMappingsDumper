#include "pch.h"

#include "unrealVersion.h"

#pragma comment(lib, "Version.lib")

#define SCAN_LIMIT 0x300

#define SCAN_FOR_MEMBER_OFFSET(obj, member, outOffset) \
	for (uint8_t* i = (uint8_t*)obj; ; i++)\
	{\
		auto Count = i - (uint8_t*)obj;\
		if (Count >= SCAN_LIMIT)\
			return false;\
		\
		if (*(uintptr_t*)i == (uintptr_t)member)\
		{\
			outOffset = Count;\
			break;\
		}\
	}\


//this is super unsafe but hopefully stackoverflow comes in clutch https://stackoverflow.com/a/42389638
bool IUnrealVersion::TryDynamicOffsets()
{
	try
	{
		auto UClassPtr = ObjObjects::FindObjectByName<UClass>(L"Class");
		auto UObjectPtr = ObjObjects::FindObjectByName<UClass>(L"Object");
		auto ActorPtr = ObjObjects::FindObjectByName<UClass>(L"Actor");
		auto EnginePtr = ObjObjects::FindObjectByName(L"/Script/Engine");

		if (!UClassPtr or !UObjectPtr or !ActorPtr or !EnginePtr)
			return false;

		SCAN_FOR_MEMBER_OFFSET(UObjectPtr, UClassPtr, UObject::ClassOffset);

		if (!UObject::ClassOffset)
			return false;

		SCAN_FOR_MEMBER_OFFSET(ActorPtr, UObjectPtr, UStruct::SuperOffset);

		if (!UStruct::SuperOffset)
			return false;

		UStruct::ChildPropertiesOffset = UStruct::SuperOffset + (sizeof(void*) * 2);

		SCAN_FOR_MEMBER_OFFSET(ActorPtr, EnginePtr, UObject::OuterOffset);

		if (!UObject::OuterOffset)
			return false;
	}
	catch (...)
	{
		return false;
	}

	return true;
}