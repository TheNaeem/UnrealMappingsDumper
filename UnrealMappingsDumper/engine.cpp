#include "pch.h"

#include "engine.h"
#include "unrealFunctions.h"

std::wstring_view FNameBase::AsString() const
{
	FString Ret;
	FNameToString(this, Ret);

	if (Ret.Data() != nullptr)
	{
		return std::wstring_view(Ret.Data());
	}

	return {};
}

std::string FNameBase::ToString() const
{
	auto Ret = AsString();

	return std::string(Ret.begin(), Ret.end());
}

template <typename T>
void DefaultEngine<T>::UObject::GetPathName(std::wstring& Result, UObject* StopOuter)
{
	if (this == StopOuter || this == NULL)
	{
		Result += L"None";
		return;
	}

	if (this->OuterPrivate && this->OuterPrivate != StopOuter)
	{
		this->OuterPrivate->GetPathName(Result, StopOuter);
		Result += L".";
	}

	Result += NamePrivate.AsString();
}

// inspired by https://github.com/kem0x/FortKit/blob/b22bb917ea254ac789333922c709d112c8989184/FortKitInsider/ue4.cpp#L176
template <typename T>
EPropertyType DefaultEngine<T>::FProperty::GetPropertyType() 
{
	switch (this->GetClass()->GetId())
	{
		case CASTCLASS_FObjectProperty:
		case CASTCLASS_FClassProperty:
		case CASTCLASS_FObjectPtrProperty:
		case CASTCLASS_FClassPtrProperty:
		{
			return EPropertyType::ObjectProperty;
		}
		case CASTCLASS_FStructProperty:
		{
			return EPropertyType::StructProperty;
		}
		case CASTCLASS_FInt8Property:
		{
			return EPropertyType::Int8Property;
		}
		case CASTCLASS_FInt16Property:
		{
			return EPropertyType::Int16Property;
		}
		case CASTCLASS_FIntProperty:
		{
			return EPropertyType::IntProperty;
		}
		case CASTCLASS_FInt64Property:
		{
			return EPropertyType::Int16Property;
		}
		case CASTCLASS_FUInt16Property:
		{
			return EPropertyType::UInt16Property;
		}
		case CASTCLASS_FUInt32Property:
		{
			return EPropertyType::UInt32Property;
		}
		case CASTCLASS_FUInt64Property:
		{
			return EPropertyType::UInt64Property;
		}
		case CASTCLASS_FArrayProperty:
		{
			return EPropertyType::ArrayProperty;
		}
		case CASTCLASS_FFloatProperty:
		{
			return EPropertyType::FloatProperty;
		}
		case CASTCLASS_FDoubleProperty:
		{
			return EPropertyType::DoubleProperty;
		}
		case CASTCLASS_FBoolProperty:
		{
			return EPropertyType::BoolProperty;
		}
		case CASTCLASS_FStrProperty:
		{
			return EPropertyType::StrProperty;
		}
		case CASTCLASS_FNameProperty:
		{
			return EPropertyType::NameProperty;
		}
		case CASTCLASS_FTextProperty:
		{
			return EPropertyType::TextProperty;
		}
		case CASTCLASS_FEnumProperty:
		{
			return EPropertyType::EnumProperty;
		}
		case CASTCLASS_FInterfaceProperty:
		{
			return EPropertyType::InterfaceProperty;
		}
		case CASTCLASS_FMapProperty:
		{
			return EPropertyType::MapProperty;
		}
		case CASTCLASS_FByteProperty:
		{
			FByteProperty* ByteProp = static_cast<FByteProperty*>(this);

			if (ByteProp->GetEnum())
				return EPropertyType::EnumAsByteProperty;
			
			return EPropertyType::ByteProperty;
		}
		case CASTCLASS_FMulticastDelegateProperty:
		case CASTCLASS_FMulticastInlineDelegateProperty:
		case CASTCLASS_FMulticastSparseDelegateProperty:
		{
			return EPropertyType::MulticastDelegateProperty;
		}
		case CASTCLASS_FDelegateProperty:
		{
			return EPropertyType::DelegateProperty;
		}
		case CASTCLASS_FSoftObjectProperty:
		case CASTCLASS_FSoftClassProperty:
		case CASTCLASS_FWeakObjectProperty:
		{
			return EPropertyType::SoftObjectProperty;
		}
		case CASTCLASS_FLazyObjectProperty:
		{
			return EPropertyType::LazyObjectProperty;
		}
		case CASTCLASS_FSetProperty:
		{
			return EPropertyType::SetProperty;
		}
		case CASTCLASS_FFieldPathProperty:
		{
			return EPropertyType::FieldPathProperty;
		}
		default:
		{
			return EPropertyType::Unknown;
		}
	}
}

template <typename T>
DefaultEngine<T>::UObject* DefaultEngine<T>::ObjObjects::GetObjectByIndex(int Index)
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

template <typename T>
void DefaultEngine<T>::ObjObjects::ForEach(std::function<void(UObject*&)> Action)
{
	for (int i = 0; i < Num(); i++)
	{
		auto Obj = GetObjectByIndex(i);

		if (!Obj) continue;

		Action(Obj);
	}
}

template <typename T>
std::vector<std::shared_ptr<IScanObject>> DefaultEngine<T>::GetFNameStringPattrns() // TODO: add more
{
	return
	{
		std::make_shared<PatternScanObject>("E8 ? ? ? ? 83 7D C8 00 48 8D 15 ? ? ? ? 0F 5A DE", 1, true),
		std::make_shared<StringRefScanObject<std::wstring>>(
			L"%s %s SetTimer passed a negative or zero time. The associated timer may fail to be created/fire! If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay).",
			true, 1, true, 0xE8)
	};
}

template <typename T>
std::vector<std::shared_ptr<IScanObject>> DefaultEngine<T>::GetGObjectsPatterns() // TODO: add more
{
	return
	{
		std::make_shared<PatternScanObject>("48 89 05 ? ? ? ? E8 ? ? ? ? ? ? ? 0F 84", 3, true),
		std::make_shared<PatternScanObject>("48 8B 05 ? ? ? ? 48 8B 0C 07 48 85 C9 74 20", 3, true),
		std::make_shared<PatternScanObject>("48 8B 05 ? ? ? ? 48 8B 0C", 3, true)
	};
}

template class DefaultEngine<UObjectDependency>;
template class DefaultEngine<FortniteUObjectBase>;

template <typename T>
typename DefaultEngine<T>::ObjObjects* DefaultEngine<T>::ObjObjects::Inst;