#pragma once

#include <string>
#include <winnt.h>
#include <functional>

#include "unrealEnums.h"
#include "unrealFunctions.h"

#define QUICK_OFFSET(type, offset) *(type*)((uintptr_t)this + offset)

#define DECLARE_STATIC_CLASS(PATH) \
    static FORCEINLINE class UClass* StaticClass() \
	{ \
		static auto Inst = ObjObjects::FindObject<class UClass>(PATH); \
		return Inst; \
	} \

class FName
{
private:

	uint32_t Number = 0;
	uint32_t Padding;

public:

	static inline bool IsOptimized = false;

	__forceinline uint32_t GetNumber()
	{
		return Number;
	}

	bool operator== (FName n) const
	{
		return n.Number == n.Number;
	}

	friend size_t hash_value(const FName& p)
	{
		return size_t(p.Number);
	}

	std::wstring_view AsString() const
	{
		FString Ret;
		FNameToString(this, Ret);

		if (Ret.Data() != nullptr)
		{
			return std::wstring_view(Ret.Data());
		}

		return {};
	}

	std::string ToString() const
	{
		auto Ret = AsString();

		return std::string(Ret.begin(), Ret.end());
	}
};

class UObject
{
private:

	static inline int NameOffset = 0;
	static inline int ClassOffset = 0;
	static inline int OuterOffset = 0;

	friend struct IUnrealVersion;

public:

	void GetPathName(std::wstring& Result, UObject* StopOuter = nullptr)
	{
		if (this == StopOuter || this == NULL)
		{
			Result += L"None";
			return;
		}

		if (Outer() && Outer() != StopOuter)
		{
			Outer()->GetPathName(Result, StopOuter);
			Result += L".";
		}

		Result += GetFName().AsString();
	}

	FORCEINLINE std::wstring_view GetName()
	{
		auto& Name = QUICK_OFFSET(FName, NameOffset);
		return Name.AsString();
	}

	FORCEINLINE FName& GetFName()
	{
		return QUICK_OFFSET(FName, NameOffset);
	}

	FORCEINLINE std::wstring GetPath()
	{
		std::wstring Ret;

		GetPathName(Ret);

		return Ret;
	}

	FORCEINLINE class UClass* Class()
	{
		return QUICK_OFFSET(class UClass*, ClassOffset);
	}

	FORCEINLINE UObject* Outer()
	{
		return QUICK_OFFSET(UObject*, OuterOffset);
	}
};

class ObjObjects
{
	enum
	{
		NumElementsPerChunk = 64 * 1024,
	};

	static inline ObjObjects* Inst;

public:

	struct FUObjectItem
	{
		UObject* Object;
		int32_t Flags;
		int32_t ClusterRootIndex;
		int32_t SerialNumber;
	};

	ObjObjects& operator=(const ObjObjects&) = delete;

private:

	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;

public:

	static UObject* GetObjectByIndex(int Index)
	{
		int ChunkIndex = Index / NumElementsPerChunk;
		int WithinChunkIndex = Index % NumElementsPerChunk;

		if (
			Index < Inst->NumElements &&
			Index >= 0 &&
			ChunkIndex < Inst->NumChunks &&
			Index < Inst->MaxElements
			)
		{
			auto Chunk = Inst->Objects[ChunkIndex];

			if (Chunk)
				return (Chunk + WithinChunkIndex)->Object;
		}

		return nullptr;
	}

	static FORCEINLINE int Num()
	{
		return Inst->NumElements;
	}

	static void SetInstance(uintptr_t Val)
	{
		if (Val)
			Inst = (ObjObjects*)Val;
	}

	template <class T = UObject>
	static T* FindObjectByName(const wchar_t* ObjectName)
	{
		for (int i = 0; i < Num(); i++)
		{
			auto Obj = GetObjectByIndex(i);

			if (!Obj) continue;

			if (Obj->GetName() == ObjectName)
				return (T*)Obj;
		}

		return nullptr;
	}

	static void ForEach(std::function<void(UObject*&)> Action)
	{
		for (int i = 0; i < Num(); i++)
		{
			auto Obj = GetObjectByIndex(i);

			if (!Obj) continue;

			Action(Obj);
		}
	}

	template <class T>
	static T* FindObject(std::wstring FullName)
	{
		for (int i = 0; i < Num(); i++)
		{
			auto Obj = GetObjectByIndex(i);

			if (!Obj) continue;

			auto Path = Obj->GetPath();

			if (FullName.size() != Path.size())
				continue;

			bool Same = wcsncmp(FullName.c_str(), Path.c_str(), FullName.size()) == 0;

			if (Same)
				return (T*)Obj;
		}

		return nullptr;
	}
};

class UStruct : public UObject
{
private:

	static inline int SuperOffset = 0;
	static inline int ChildPropertiesOffset = 0;

	friend struct IUnrealVersion;

public:

	FORCEINLINE UStruct* Super()
	{
		return QUICK_OFFSET(UStruct*, SuperOffset);
	}

	FORCEINLINE int32_t PropertiesSize()
	{
		return QUICK_OFFSET(int32_t, ChildPropertiesOffset + sizeof(void*));
	}

	FORCEINLINE class FProperty* ChildProperties()
	{
		return QUICK_OFFSET(class FProperty*, ChildPropertiesOffset);
	}
};

class UClass : public UStruct
{
public:

	DECLARE_STATIC_CLASS(L"/Script/CoreUObject.Class");
};

class UScriptStruct : public UStruct
{
public:

	DECLARE_STATIC_CLASS(L"/Script/CoreUObject.ScriptStruct");
};

class FFieldClass
{
	FName Name;
	EClassCastFlags Id;

public:

	FORCEINLINE FName& GetFName()
	{
		return Name;
	}

	FORCEINLINE std::wstring_view GetName()
	{
		return Name.AsString();
	}

	FORCEINLINE EClassCastFlags GetId()
	{
		return Id;
	}
};

class FField
{
public:

	class Variant
	{
		union FFieldObjectUnion
		{
			FField* Field;
			UObject* Object;
		}Container;

		bool bIsUObject;
	};

private:

	void* Vtbl;
	FFieldClass* ClassPrivate;
	Variant Owner;
	FField* Next;
	FName NamePrivate;
	EObjectFlags FlagsPrivate;

public:

	FORCEINLINE FName& GetFName()
	{
		return NamePrivate;
	}

	FORCEINLINE FField* GetNext() const
	{
		return Next;
	}

	FORCEINLINE FFieldClass* GetClass() const
	{
		return ClassPrivate;
	}

	FORCEINLINE EObjectFlags GetFlags() const
	{
		if (FName::IsOptimized)
		{
			return QUICK_OFFSET(EObjectFlags, offsetof(FField, NamePrivate) + 4);
		}

		return FlagsPrivate;
	}
};

class FProperty : public FField
{
private:

	int32_t ArrayDim;

protected:

	static inline int FPropertySize = 0;

public:

	friend struct IUnrealVersion;

	FORCEINLINE int32_t GetArrayDim()
	{
		if (FName::IsOptimized)
		{
			return QUICK_OFFSET(int32_t, sizeof(FField) - 4);
		}

		return ArrayDim;
	}
};

class UEnum : public UObject
{
public:

	TArray<TPair<FName, int64_t>>& Names()
	{
		static auto FieldSize = ObjObjects::FindObjectByName<UStruct>(L"Field")->PropertiesSize();

		return *(TArray<TPair<FName, int64_t>>*)
			& QUICK_OFFSET(uint8_t, FieldSize + sizeof(FString));
	}

	DECLARE_STATIC_CLASS(L"/Script/CoreUObject.Enum");
};

class FStructProperty : public FProperty
{
	UScriptStruct* Struct;

public:

	FORCEINLINE UScriptStruct* GetStruct()
	{
		return QUICK_OFFSET(UScriptStruct*, FPropertySize);
	}
};

class FByteProperty : public FProperty
{
	UEnum* Enum;

public:

	FORCEINLINE UEnum* GetEnum()
	{
		return QUICK_OFFSET(UEnum*, FPropertySize);
	}
};

class FArrayProperty : public FProperty
{
	enum class EArrayPropertyFlags
	{
		None,
		UsesMemoryImageAllocator
	};

	FProperty* Inner;
	EArrayPropertyFlags ArrayFlags;

public:

	FORCEINLINE FProperty* GetInner()
	{
		return QUICK_OFFSET(FProperty*, FPropertySize);
	}
};

class FMapProperty : public FProperty
{
	FProperty* KeyProp;
	FProperty* ValueProp;

public:

	FORCEINLINE FProperty* GetKey()
	{
		return QUICK_OFFSET(FProperty*, FPropertySize);
	}

	FORCEINLINE FProperty* GetValue()
	{
		return QUICK_OFFSET(FProperty*, FPropertySize + sizeof(FProperty*));
	}
};

class FEnumProperty : public FProperty
{
	FProperty* UnderlyingProp;
	UEnum* Enum;

public:

	FORCEINLINE FProperty* GetUnderlying()
	{
		return QUICK_OFFSET(FProperty*, FPropertySize);
	}

	FORCEINLINE UEnum* GetEnum()
	{
		return QUICK_OFFSET(UEnum*, FPropertySize + sizeof(FProperty*));
	}
};