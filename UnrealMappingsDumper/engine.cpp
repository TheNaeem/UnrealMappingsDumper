#include "pch.h"

#include "engine.h"
#include "unrealFunctions.h"

std::wstring_view FNameBase::AsString()
{
	FString Ret;
	FNameToString(this, Ret);

	if (Ret.Data() != nullptr)
	{
		return std::wstring_view(Ret.Data());
	}

	return {};
}

std::string FNameBase::ToString()
{
	auto Ret = AsString();

	return std::string(Ret.begin(), Ret.end());
}