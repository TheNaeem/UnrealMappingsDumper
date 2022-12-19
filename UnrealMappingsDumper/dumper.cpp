#include "pch.h"

#include "dumper.h"
#include "writer.h"
#include "oodle.h"

static EPropertyType GetPropertyType(FProperty* Prop)
{
	switch (Prop->GetClass()->GetId())
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
		return EPropertyType::Int64Property;
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
		FByteProperty* ByteProp = static_cast<FByteProperty*>(Prop);

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
	{
		return EPropertyType::SoftObjectProperty;
	}
	case CASTCLASS_FWeakObjectProperty:
	{
		return EPropertyType::WeakObjectProperty;
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

struct FPropertyData
{
	FProperty* Prop;
	uint16_t Index;
	uint8_t ArrayDim;
	FName Name;
	EPropertyType PropertyType;

	FPropertyData(FProperty* P, int Idx) :
		Prop(P),
		Index(Idx),
		ArrayDim(P->GetArrayDim()),
		Name(P->GetFName()),
		PropertyType(GetPropertyType(P))
	{
	}
};

void Dumper::Run(ECompressionMethod CompressionMethod)
{
	StreamWriter Buffer;
	phmap::parallel_flat_hash_map<FName, int> NameMap;

	std::vector<UEnum*> Enums;
	std::vector<UStruct*> Structs; // TODO: a better way than making this completely dynamic

	std::function<void(class FProperty*&, EPropertyType)> WritePropertyWrapper{}; // hacky.. i know

	auto WriteProperty = [&](FProperty*& Prop, EPropertyType Type)
	{
		if (Type == EPropertyType::EnumAsByteProperty)
			Buffer.Write(EPropertyType::EnumProperty);
		else Buffer.Write(Type);

		switch (Type)
		{
		case EPropertyType::EnumProperty:
		{
			auto EnumProp = static_cast<FEnumProperty*>(Prop);

			auto Inner = EnumProp->GetUnderlying();
			auto InnerType = GetPropertyType(Inner);
			WritePropertyWrapper(Inner, InnerType);
			Buffer.Write(NameMap[EnumProp->GetEnum()->GetFName()]);

			break;
		}
		case EPropertyType::EnumAsByteProperty:
		{
			Buffer.Write(EPropertyType::ByteProperty);
			Buffer.Write(NameMap[static_cast<FByteProperty*>(Prop)->GetEnum()->GetFName()]);

			break;
		}
		case EPropertyType::StructProperty:
		{
			Buffer.Write(NameMap[static_cast<FStructProperty*>(Prop)->GetStruct()->GetFName()]);
			break;
		}
		case EPropertyType::SetProperty:
		case EPropertyType::ArrayProperty:
		{
			auto Inner = static_cast<FArrayProperty*>(Prop)->GetInner();
			auto InnerType = GetPropertyType(Inner);
			WritePropertyWrapper(Inner, InnerType);

			break;
		}
		case EPropertyType::MapProperty:
		{
			auto Inner = static_cast<FMapProperty*>(Prop)->GetKey();
			auto InnerType = GetPropertyType(Inner);
			WritePropertyWrapper(Inner, InnerType);

			auto Value = static_cast<FMapProperty*>(Prop)->GetValue();
			auto ValueType = GetPropertyType(Value);
			WritePropertyWrapper(Value, ValueType);

			break;
		}
		}
	};

	WritePropertyWrapper = WriteProperty;

	ObjObjects::ForEach([&](UObject*& Object)
		{
			if (Object->Class() == UClass::StaticClass() ||
			Object->Class() == UScriptStruct::StaticClass())
			{
				auto Struct = static_cast<UStruct*>(Object);

				Structs.push_back(Struct);

				NameMap.insert_or_assign(Struct->GetFName(), 0);

				if (Struct->Super() && !NameMap.contains(Struct->Super()->GetFName()))
					NameMap.insert_or_assign(Struct->Super()->GetFName(), 0);

				auto Props = Struct->ChildProperties();

				while (Props)
				{
					NameMap.insert_or_assign(Props->GetFName(), 0);
					Props = static_cast<FProperty*>(Props->GetNext());
				}
			}
			else if (Object->Class() == UEnum::StaticClass())
			{
				auto Enum = static_cast<UEnum*>(Object);
				Enums.push_back(Enum);

				NameMap.insert_or_assign(Enum->GetFName(), 0);

				auto& EnumNames = Enum->Names();

				for (auto i = 0; i < EnumNames.Num(); i++)
				{
					NameMap.insert_or_assign(EnumNames[i].Key.GetNumber(), 0);
				}
			}
		});

	Buffer.Write<int>(NameMap.size());

	int CurrentNameIndex = 0;

	for (auto&& N : NameMap)
	{
		NameMap[N.first] = CurrentNameIndex;

		auto Name = N.first.ToString();
		std::string_view NameView = Name;

		auto Find = Name.find("::");
		if (Find != std::string::npos)
		{
			NameView = NameView.substr(Find + 2);
		}

		Buffer.Write<uint8_t>(NameView.length());
		Buffer.WriteString(NameView);

		CurrentNameIndex++;
	}

	Buffer.Write<uint32_t>(Enums.size());

	for (auto Enum : Enums)
	{
		Buffer.Write(NameMap[Enum->GetFName()]);

		auto& EnumNames = Enum->Names();
		Buffer.Write<uint8_t>(EnumNames.Num());

		for (size_t i = 0; i < EnumNames.Num(); i++)
		{
			Buffer.Write<int>(NameMap[EnumNames[i].Key]);
		}
	}

	Buffer.Write<uint32_t>(Structs.size());

	for (auto Struct : Structs)
	{
		Buffer.Write(NameMap[Struct->GetFName()]);
		Buffer.Write<int32_t>(Struct->Super() ? NameMap[Struct->Super()->GetFName()] : 0xffffffff);

		std::vector<FPropertyData> Properties;

		auto Props = Struct->ChildProperties();
		uint16_t PropCount = 0;
		uint16_t SerializablePropCount = 0;

		while (Props)
		{
			FPropertyData Data(Props, PropCount);

			Properties.push_back(Data);
			Props = static_cast<FProperty*>(Props->GetNext());

			PropCount += Data.ArrayDim;
			SerializablePropCount++;
		}

		Buffer.Write(PropCount);
		Buffer.Write(SerializablePropCount);

		for (auto P : Properties)
		{
			Buffer.Write<uint16_t>(P.Index);
			Buffer.Write(P.ArrayDim);
			Buffer.Write(NameMap[P.Name]);

			WriteProperty(P.Prop, P.PropertyType);
		}
	}

	std::vector<uint8_t> UsmapData;

	switch (CompressionMethod)
	{
	case ECompressionMethod::Oodle:
	{
		UsmapData = Oodle::Compress(Buffer.GetBuffer());
		break;
	}
	default:
	{
		std::string UncompressedStream = Buffer.GetBuffer().str();
		UsmapData.resize(UncompressedStream.size());
		memcpy(UsmapData.data(), UncompressedStream.data(), UsmapData.size());
	}
	}

	auto FileOutput = FileWriter("Mappings.usmap");

	FileOutput.Write<uint16_t>(0x30C4); //magic
	FileOutput.Write<uint8_t>(0); //version
	FileOutput.Write(CompressionMethod); //compression
	FileOutput.Write<uint32_t>(UsmapData.size()); //compressed size
	FileOutput.Write<uint32_t>(Buffer.Size()); //decompressed size

	FileOutput.Write(UsmapData.data(), UsmapData.size());
}