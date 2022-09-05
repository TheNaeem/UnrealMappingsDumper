#pragma once

template <typename K, typename V>
struct TPair
{
	K Key;
	V Value;
};

template <class T>
class TArray
{
	T* Buffer;
	int32_t ArrayNum;
	int32_t ArrayMax;

public:

	TArray()
	{
		Buffer = nullptr;
		ArrayNum = 0;
		ArrayMax = 0;
	}

	FORCEINLINE int Num()
	{
		return ArrayNum;
	}

	FORCEINLINE T& operator[](int i)
	{
		return Buffer[i];
	};

	FORCEINLINE T* Data()
	{
		return Buffer;
	}
};

/// <summary>
/// When you don't care about the type of a TArray just use this.
/// </summary>
typedef TArray<uint8_t> TUndefinedArray;

template<class TEnum>
class TEnumAsByte
{
public:

	typedef TEnum EnumType;

	TEnumAsByte() = default;
	TEnumAsByte(const TEnumAsByte&) = default;
	TEnumAsByte& operator=(const TEnumAsByte&) = default;

	FORCEINLINE TEnumAsByte(TEnum InValue)
		: Value(static_cast<uint8_t>(InValue))
	{ }

	explicit FORCEINLINE TEnumAsByte(int32_t InValue)
		: Value(static_cast<uint8_t>(InValue))
	{ }

	explicit FORCEINLINE TEnumAsByte(uint8_t InValue)
		: Value(InValue)
	{ }

public:

	bool operator==(TEnum InValue) const
	{
		return static_cast<TEnum>(Value) == InValue;
	}

	bool operator==(TEnumAsByte InValue) const
	{
		return Value == InValue.Value;
	}

	operator TEnum() const
	{
		return (TEnum)Value;
	}

public:

	TEnum GetValue() const
	{
		return (TEnum)Value;
	}

private:

	uint8_t Value;
};

class FString : public TArray<wchar_t>
{
public:

	FORCEINLINE std::wstring_view AsString()
	{
		return Data();
	}
};