#pragma once

#include "app.h"
#include "unrealTypes.h"
#include "scanning.h"

/*
* The idea here is to make something that can easily be overriden for engine or game versions with different types
* that need to be overriden or handled differently.
*/

struct IUnrealVersion
{
private:
	static bool TryDynamicOffsets();

public:

	template <typename Version>
	static bool InitTypes()
	{
		uintptr_t GObjectsAddy = 0;

		for (auto Scan : Version::GetGObjectsPatterns())
		{
			GObjectsAddy = Scan->TryFind();

			if (GObjectsAddy)
				break;
		}

		if (!GObjectsAddy)
		{
			UE_LOG("Could not find the address for GObjects. Try overriding it or adding the correct sig for it.");
			return false;
		}

		ObjObjects::SetInstance(GObjectsAddy);

		uintptr_t FNameStringAddy = 0;

		for (auto Scan : Version::GetFNameStringPatterns())
		{
			FNameStringAddy = Scan->TryFind();

			if (FNameStringAddy)
				break;
		}

		if (!FNameStringAddy)
		{
			UE_LOG("Could not find the address for FNameToString. Try overriding it or adding the correct sig for it.");
			return false;
		}

		FNameToString = (_FNameToString)FNameStringAddy;

		using UObjectImpl = Version::Offsets::UObject;
		using UStructImpl = Version::Offsets::UStruct;

		UObject::NameOffset = UObjectImpl::NameOffset;
		FName::IsOptimized = Version::HasOptimizedFName;
		FProperty::FPropertySize = Version::FPropertySize;

		if (!TryDynamicOffsets())
		{
			UE_LOG("Could not grab dynamic offsets. Just gonna use the hardcoded ones.");

			UObject::ClassOffset = UObjectImpl::ClassOffset;
			UObject::OuterOffset = UObjectImpl::OuterOffset;

			UStruct::SuperOffset = UStructImpl::SuperOffset;
			UStruct::ChildPropertiesOffset = UStructImpl::ChildPropertiesOffset;
		}

		return true;
	}
};

/*
* Yes, it's true, UObject should pretty much always be the same across all games,
* however there are some games that use a custom UObject, so should they ever need mappings,
* this design pattern makes it easier to override the offsets.
*/

struct UnrealVersionBase : IUnrealVersion
{
	static constexpr int FPropertySize = 0x78;
	static constexpr bool HasOptimizedFName = false;

	struct Offsets
	{
		struct UObject
		{
			static constexpr int NameOffset = 0x18;
			static constexpr int ClassOffset = 0x10;
			static constexpr int OuterOffset = 0x20;
		};

		struct UStruct
		{
			static constexpr int SuperOffset = 0x40;
			static constexpr int ChildPropertiesOffset = 0x50;
		};
	};

	static std::vector<std::shared_ptr<IScanObject>> GetFNameStringPatterns()
	{
		return
		{
			std::make_shared<PatternScanObject>("E8 ? ? ? ? 83 7D C8 00 48 8D 15 ? ? ? ? 0F 5A DE", 1, true),
			std::make_shared<StringRefScanObject<std::wstring>>(
				L"%s %s SetTimer passed a negative or zero time. The associated timer may fail to be created/fire! If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay).",
				true, 1, true, 0xE8)
		};
	}


	static std::vector<std::shared_ptr<IScanObject>> GetGObjectsPatterns()
	{
		return
		{
			std::make_shared<PatternScanObject>("48 89 05 ? ? ? ? E8 ? ? ? ? ? ? ? 0F 84", 3, true),
			std::make_shared<PatternScanObject>("48 8B 05 ? ? ? ? 48 8B 0C 07 48 85 C9 74 20", 3, true),
			std::make_shared<PatternScanObject>("48 8B 05 ? ? ? ? 48 8B 0C", 3, true),
			std::make_shared<PatternScanObject>("48 03 ? ? ? ? ? ? ? ? ? ? 48 8B 10 48 85 D2 74 07", 3, true)

		};
	}
};

/*
* Use this if the games engine has UE_FNAME_OUTLINE_NUMBER defined as 1
*/
struct Version_OptimizedFName : UnrealVersionBase
{
	static constexpr int FPropertySize = 0x70;
	static constexpr bool HasOptimizedFName = true;
};

struct Version_FortniteLatest : Version_OptimizedFName
{
	static std::vector<std::shared_ptr<IScanObject>> GetGObjectsPatterns()
	{
		return
		{
			std::make_shared<PatternScanObject>("48 8B 05 ? ? ? ? 48 8B 0C C8", 3, true)
		};
	}
};