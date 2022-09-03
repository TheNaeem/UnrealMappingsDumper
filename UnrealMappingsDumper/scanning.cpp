#include "pch.h"

#include "scanning.h"

uintptr_t PatternScanObject::TryFind()
{
	auto Addy = Memcury::Scanner::FindPattern(Sig.c_str());
	
	if (!Addy.IsValid())
		return 0;

	if (bRelative)
	{
		Addy.RelativeOffset(RelativeAddressOffset);
	}

	Addy.AbsoluteOffset(ResultOffset);

	return Addy.Get();
}

template <typename T>
uintptr_t StringRefScanObject<T>::TryFind()
{
	Memcury::Scanner Addy = Memcury::Scanner::FindStringRef(StringRef);

	if (!Addy.IsValid())
		return 0;
	
	if (OpcodeToFind)
	{
		auto BeforeOpcodeScan = Addy.Get();

		Addy.ScanFor({ OpcodeFindsToSkip }, !bScanBackwards, OpcodeFindsToSkip);

		if (Addy.Get() == BeforeOpcodeScan)
			return 0;
	}

	if (bRelative)
		Addy.RelativeOffset(Offset);
	else Addy.AbsoluteOffset(Offset);

	return Addy.Get();
}