#pragma once

struct IScanObject
{
	virtual uintptr_t TryFind() = 0;
};

struct PatternScanObject : public IScanObject
{
	PatternScanObject(
		const char* sig,
		int resultOffset = 0,
		bool relative = false,
		int relativeAddressOffset = 0)
		: 
		Sig(sig), 
		ResultOffset(resultOffset), 
		bRelative(relative), 
		RelativeAddressOffset(relativeAddressOffset)
	{
	}

	const char* Sig;
	bool bRelative;
	int RelativeAddressOffset;
	int ResultOffset;

	uintptr_t TryFind() override;
};