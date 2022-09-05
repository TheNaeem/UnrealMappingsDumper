#pragma once

#ifndef UE_FNAME_OUTLINE_NUMBER
	#define UE_FNAME_OUTLINE_NUMBER 1
#endif 

typedef uint32_t FNameEntryId;

class FNameBase
{
public:

	std::wstring_view AsString() const;
	std::string ToString() const;
};

class UObjectDependency
{
public:

	struct FName : public FNameBase
	{
		FNameEntryId ComparisonIndex;
		uint32_t Number;

		bool operator== (FName n) const
		{
			return ComparisonIndex == n.ComparisonIndex;
		}

		friend size_t hash_value(const FName& p)
		{
			return phmap::HashState().combine(0, p.ComparisonIndex, p.ComparisonIndex / 3);
		}
	};

protected:

	UObjectDependency()
	{
	}
};

class FortniteUObjectBase : public UObjectDependency
{
public:

	struct FName : public FNameBase
	{
		FNameEntryId ComparisonIndex;

#if !UE_FNAME_OUTLINE_NUMBER
		uint32_t Number;
#endif

		bool operator== (FName n) const
		{
			return ComparisonIndex == n.ComparisonIndex;
		}

		friend size_t hash_value(const FName& p)
		{
			return phmap::HashState().combine(0, p.ComparisonIndex, p.ComparisonIndex / 3);
		}
	};

protected:

	FortniteUObjectBase()
	{
	}
};