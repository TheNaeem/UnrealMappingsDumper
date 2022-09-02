#pragma once

#ifndef UE_FNAME_OUTLINE_NUMBER
	#define UE_FNAME_OUTLINE_NUMBER 1
#endif 

typedef uint32_t FNameEntryId;

class FNameBase
{
public:

	std::wstring_view AsString();
	std::string ToString();
};

class UObjectDependency
{
public:

	class FName : public FNameBase
	{
		FNameEntryId ComparisonIndex;
		uint32_t Number;
	};

protected:

	UObjectDependency()
	{
	}
};

class FortniteUObjectBase : public UObjectDependency
{
public:

	class FName : public FNameBase
	{
		FNameEntryId ComparisonIndex;

#if !UE_FNAME_OUTLINE_NUMBER
		uint32_t Number;
#endif
	};

protected:

	FortniteUObjectBase()
	{
	}
};