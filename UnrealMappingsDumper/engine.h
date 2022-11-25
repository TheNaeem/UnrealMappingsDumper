#pragma once

#include "uobjectDependency.h"
#include "unrealEnums.h"
#include "unrealContainers.h"
#include "scanning.h"

typedef int FThreadSafeCounter;

#define DECLARE_STATIC_CLASS(PATH) \
    static FORCEINLINE class UClass* StaticClass() \
	{ \
		static auto Inst = FindObject<class UClass>(PATH); \
		return Inst; \
	} \


/// <summary>
/// The idea here is to easily override UE classes for specific Engine versiosn if needed. DefaultEngine is targeted for UE5.
/// </summary>
template <typename UObjectBase = UObjectDependency>
class DefaultEngine
{
public:

	class FName : public UObjectBase::FName
	{
	};

	template <class T>
	static T* FindObject(std::wstring FullName)
	{
		for (int i = 0; i < ObjObjects::Num(); i++)
		{
			auto Obj = ObjObjects::GetObjectByIndex(i);

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

	class UObject
	{
		void* Vtbl;
		EObjectFlags ObjectFlags;
		int32_t InternalIndex;
		class UClass* ClassPrivate;
		FName NamePrivate;
		UObject* OuterPrivate;

	public:

		void GetPathName(std::wstring& Result, UObject* StopOuter = nullptr);

		FORCEINLINE FName GetName()
		{
			return NamePrivate;
		}

		FORCEINLINE std::wstring GetPath()
		{
			std::wstring Ret;

			GetPathName(Ret);

			return Ret;
		}

		FORCEINLINE class UClass* Class()
		{
			return ClassPrivate;
		}

		FORCEINLINE UObject* Outer()
		{
			return OuterPrivate;
		}
	};

	class UField : public UObject
	{
		UField* Next;

	public:

		FORCEINLINE UField* GetNext()
		{
			return Next;
		}
	};

	class FFieldClass
	{
		FName Name;
		EClassCastFlags Id;
		uint64_t CastFlags;
		EClassFlags ClassFlags;
		FFieldClass* SuperClass;
		class FField* DefaultObject;
		class FField* (*ConstructFn)(class FFieldVariant&, FName&, EObjectFlags);
		FThreadSafeCounter UnqiueNameIndexCounter;

	public:

		FORCEINLINE std::string GetName() const
		{
			return Name.ToString();
		}

		FORCEINLINE UObjectBase::FName GetFName() const
		{
			return Name;
		}

		FORCEINLINE uint64_t GetId() const
		{
			return Id;
		}

		FORCEINLINE uint64_t GetCastFlags() const
		{
			return CastFlags;
		}

		FORCEINLINE bool HasAnyCastFlags(const uint64_t InCastFlags) const
		{
			return !!(CastFlags & InCastFlags);
		}

		FORCEINLINE bool HasAllCastFlags(const uint64_t InCastFlags) const
		{
			return (CastFlags & InCastFlags) == InCastFlags;
		}

		FORCEINLINE EClassCastFlags GetId()
		{
			return Id;
		}
	};

	class FFieldVariant
	{
		union FFieldObjectUnion
		{
			class FField* Field;
			UObject* Object;
		}Container;

		bool bIsUObject;
	};

	class FField
	{
		void* Vtbl;
		FFieldClass* ClassPrivate;
		FFieldVariant Owner;
		FField* Next;
		FName NamePrivate;
		EObjectFlags FlagsPrivate;

	public:

		FORCEINLINE FName GetName()
		{
			return NamePrivate;
		}

		FORCEINLINE FField* GetNext()
		{
			return Next;
		}

		FORCEINLINE FFieldClass* GetClass() const
		{
			return ClassPrivate;
		}

		FORCEINLINE EObjectFlags GetFlags() const
		{
			return FlagsPrivate;
		}

		FORCEINLINE uint64_t GetCastFlags() const
		{
			return GetClass()->GetCastFlags();
		}
	};

	class FProperty : public FField
	{
		int32_t	ArrayDim;
		int32_t	ElementSize;
		EPropertyFlags PropertyFlags;
		uint16_t RepIndex;
		TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
		int32_t Offset_Internal;

	public:

		FName RepNotifyFunc;
		FProperty* PropertyLinkNext;
		FProperty* NextRef;
		FProperty* DestructorLinkNext;
		FProperty* PostConstructLinkNext;

		EPropertyType GetPropertyType();

		FORCEINLINE int32_t GetArrayDim()
		{
			return ArrayDim;
		}
	};

	class FStructProperty : public FProperty
	{
		class UScriptStruct* Struct;

	public:

		FORCEINLINE class UScriptStruct* GetStruct()
		{
			return Struct;
		}
	};

	class FByteProperty : public FProperty
	{
		class UEnum* Enum;

	public:

		FORCEINLINE class UEnum* GetEnum()
		{
			return Enum;
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
			return Inner;
		}
	};

	class FMapProperty : public FProperty
	{
		FProperty* KeyProp;
		FProperty* ValueProp;

	public:

		FORCEINLINE FProperty* GetKey()
		{
			return KeyProp;
		}

		FORCEINLINE FProperty* GetValue()
		{
			return ValueProp;
		}
	};

	class FEnumProperty : public FProperty
	{
		FProperty* UnderlyingProp;
		class UEnum* Enum;

	public:

		FORCEINLINE FProperty* GetUnderlying()
		{
			return UnderlyingProp;
		}

		FORCEINLINE class UEnum* GetEnum()
		{
			return Enum;
		}
	};

	class FStructBaseChain
	{
		FStructBaseChain** StructBaseChainArray;
		int32_t NumStructBasesInChainMinusOne;
	};

	class UEnum : public UField
	{
	public:

		enum class ECppForm
		{
			Regular,
			Namespaced,
			EnumClass
		};

		FString CppType;
		TArray<TPair<FName, int64_t>> Names;
		ECppForm CppForm;
		EEnumFlags EnumFlags;

		DECLARE_STATIC_CLASS(L"/Script/CoreUObject.Enum");

		std::wstring_view GetCPPString()
		{
			return CppType.AsString();
		}
	};

	class UStruct : public UField, FStructBaseChain
	{
	private:

		UStruct* SuperStruct;

	public:

		UField* Children;
		FField* ChildProperties;
		int32_t PropertiesSize;
		int32_t MinAlignment;
		TArray<uint8_t> Script;
		FProperty* PropertyLink;
		FProperty* RefLink;
		FProperty* DestructorLink;
		FProperty* PostConstructLink;
		TArray<UObject*> ScriptAndPropertyObjectReferences;

		FORCEINLINE UStruct* Super()
		{
			return SuperStruct;
		}

		FORCEINLINE FProperty* GetProperties()
		{
			return static_cast<FProperty*>(ChildProperties);
		}
	};

	class UScriptStruct : public UStruct
	{
	public:

		DECLARE_STATIC_CLASS(L"/Script/CoreUObject.ScriptStruct");
	};

	class UClass : public UStruct
	{
		typedef void (*ClassConstructorType)(const class FObjectInitializer&);
		typedef UObject* (*ClassVTableHelperCtorCallerType)(class FVTableHelper& Helper);
		typedef UClass* (*StaticClassFunctionType)();

	public:

		ClassConstructorType ClassConstructor;
		ClassVTableHelperCtorCallerType ClassVTableHelperCtorCaller;
		void* CppClassStaticFunctions;
		mutable int32_t ClassUnique;
		int32_t FirstOwnedClassRep = 0;
		bool bCooked;
		bool bLayoutChanging;
		EClassFlags ClassFlags;
		EClassCastFlags ClassCastFlags;
		UClass* ClassWithin;
		FName ClassConfigName;
		TUndefinedArray ClassReps;
		TArray<UField*> NetFields;

		DECLARE_STATIC_CLASS(L"/Script/CoreUObject.Class");
	};

	// Completely refactor this if we need to override it for a different engine version, but still keep it as a singleton.
	class ObjObjects
	{
		enum
		{
			NumElementsPerChunk = 64 * 1024,
		};

		static ObjObjects* Inst;

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

		static void ForEach(std::function<void(UObject*&)> Action);

		static FORCEINLINE int Num()
		{
			return Inst->NumElements;
		}

		static void SetInstance(uintptr_t Val)
		{
			if (Val)
				Inst = (ObjObjects*)Val;
		}
	};

public:

	static std::vector<std::shared_ptr<IScanObject>> GetFNameStringPattrns();
	static std::vector<std::shared_ptr<IScanObject>> GetGObjectsPatterns();

	template <typename T>
	static FORCEINLINE UClass* StaticClass()
	{
		return T::StaticClass();
	}
};

class Engine_UE5 : public DefaultEngine<> // DefaultEngine is pretty much already for UE5
{
public:

};

class Engine_Fortnite : public DefaultEngine<FortniteUObjectBase>
{
public:

};