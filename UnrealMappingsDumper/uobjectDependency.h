#pragma once

#ifndef UE_FNAME_OUTLINE_NUMBER
	#define UE_FNAME_OUTLINE_NUMBER 0
#endif 

typedef uint32_t FNameEntryId;

class UObjectDependency
{
public:

	class FName
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

	class FName
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