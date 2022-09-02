#pragma once

template <class T>
struct TArray
{
	T* Data;
	int32_t ArrayNum;
	int32_t ArrayMax;

	TArray()
	{
		Data = nullptr;
		ArrayNum = 0;
		ArrayMax = 0;
	}

	FORCEINLINE int Num()
	{
		return ArrayNum;
	}

	FORCEINLINE T& operator[](int i)
	{
		return Data[i];
	};
};

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

	explicit FORCEINLINE TEnumAsByte(int32 InValue)
		: Value(static_cast<uint8>(InValue))
	{ }

	explicit FORCEINLINE TEnumAsByte(uint8 InValue)
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