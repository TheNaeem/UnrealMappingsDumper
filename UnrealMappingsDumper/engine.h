#pragma once

#include "uobjectDependency.h"
#include "unrealEnums.h"
#include "unrealTemplateCore.h"

template <typename UObjectBase = UObjectDependency>
class DefaultEngine
{
public:

	class UObject
	{
		void* Vtbl;
		EObjectFlags ObjectFlags;
		int32_t InternalIndex;
		class UClass* ClassPrivate;
		UObjectBase::FName NamePrivate;
		UObject* OuterPrivate;

	public:

		FORCEINLINE UClass* Class()
		{
			return ClassPrivate;
		}
	};

	class UField
	{
		UField* Next;

	public:

		FORCEINLINE UField* Next()
		{
			return Next;
		}
	};

	class FField
	{
		FFieldClass* ClassPrivate;
		FFieldVariant Owner;
		FField* Next;
		UObjectBase::FName NamePrivate;
		EObjectFlags FlagsPrivate;

	public:

		FORCEINLINE FField* Next()
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

	class UStruct : public UField
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

		__forceinline UStruct* Super()
		{
			return SuperStruct;
		}
	};
};

class Engine_UE5_01 : public DefaultEngine<>
{
public:

};

class Engine_Fortnite : public DefaultEngine<FortniteUObjectBase>
{
public:

};