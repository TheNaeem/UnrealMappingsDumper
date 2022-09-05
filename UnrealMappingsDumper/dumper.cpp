#include "pch.h"

#include "dumper.h"
#include "unrealFunctions.h"
#include "app.h"
#include "writer.h"

template <typename Engine>
bool Dumper<Engine>::Init(uintptr_t GObjectsOverride, uintptr_t FNameToStringOverride)
{
	if (GObjectsOverride)
		Engine::ObjObjects::SetInstance(GObjectsOverride);

	if (FNameToStringOverride)
		FNameToString = (_FNameToString)FNameToStringOverride;

	if (GObjectsOverride && FNameToStringOverride)
		return true;

	uintptr_t GObjectsAddy = 0;

	for (ScanObject Scan : Engine::GetGObjectsPatterns())
	{
		GObjectsAddy = Scan.TryFind();

		if (GObjectsAddy)
			break;
	}

	if (!GObjectsAddy)
	{
		UE_LOG("Could not find the address for GObjects. Try overriding it or adding the correct sig for it.");
		return false;
	}

	Engine::ObjObjects::SetInstance(GObjectsAddy);

	return true;
}

template <typename Engine>
void Dumper<Engine>::Run(ECompressionMethod CompressionMethod) const
{
	StreamWriter Buffer;
	phmap::parallel_flat_hash_map<Engine::FName, int> NameMap;

	std::vector<typename Engine::UEnum*> Enums;
	std::vector<typename Engine::UStruct*> Structs; // TODO: a better way than making this completely dynamic

	auto GetStructNames = [&](Engine::UStruct* Struct)
	{
		NameMap.insert_or_assign(Struct->GetName(), 0);

		if (Struct->Super() && !NameMap.contains(Struct->Super()->GetName()))
			NameMap.insert_or_assign(Struct->Super()->GetName(), 0);

		auto Props = Struct->GetProperties();

		while (Props)
		{
			NameMap.insert_or_assign(Props->GetName(), 0);
			Props = static_cast<Engine::FProperty*>(Props->GetNext());
		}
	};

	auto GetEnumNames = [&](Engine::UEnum* Enum)
	{
		NameMap.insert_or_assign(Enum->GetName(), 0);

		for (auto i = 0; i < Enum->Names.Num(); i++)
		{
			NameMap.insert_or_assign(Enum->Names[i].Key, 0);
		}
	};

	std::function<void(typename Engine::FProperty*&, EPropertyType)> WritePropertyWrapper{}; // hacky but idc

	auto WriteProperty = [&](Engine::FProperty*& Prop, EPropertyType Type)
	{
		if (Type == EPropertyType::EnumAsByteProperty)
			Buffer.Write(EPropertyType::EnumProperty);
		else Buffer.Write(Type);

		switch (Type)
		{
			case EPropertyType::EnumProperty:
			{
				auto EnumProp = static_cast<Engine::FEnumProperty*>(Prop);

				auto Inner = EnumProp->GetUnderlying();
				auto InnerType = Inner->GetPropertyType();
				WritePropertyWrapper(Inner, InnerType);
				Buffer.Write(NameMap[EnumProp->GetEnum()->GetName()]);

				break;
			}
			case EPropertyType::EnumAsByteProperty:
			{
				Buffer.Write(EPropertyType::ByteProperty);
				Buffer.Write(NameMap[static_cast<Engine::FByteProperty*>(Prop)->GetEnum()->GetName()]);

				break;
			}
			case EPropertyType::StructProperty:
			{
				Buffer.Write(NameMap[static_cast<Engine::FStructProperty*>(Prop)->GetStruct()->GetName()]);
				break;
			}
			case EPropertyType::SetProperty:
			case EPropertyType::ArrayProperty:
			{
				auto Inner = static_cast<Engine::FArrayProperty*>(Prop)->GetInner();
				auto InnerType = Inner->GetPropertyType();
				WritePropertyWrapper(Inner, InnerType);

				break;
			}
			case EPropertyType::MapProperty:
			{
				auto Inner = static_cast<Engine::FMapProperty*>(Prop)->GetKey();
				auto InnerType = Inner->GetPropertyType();
				WritePropertyWrapper(Inner, InnerType);

				auto Value = static_cast<Engine::FMapProperty*>(Prop)->GetValue();
				auto ValueType = Value->GetPropertyType();
				WritePropertyWrapper(Value, ValueType);

				break;
			}
		}
	};

	WritePropertyWrapper = WriteProperty;

	Engine::ObjObjects::ForEach([&](Engine::UObject*& Object)
		{
			if (Object->Class() == Engine::UClass::StaticClass() ||
				Object->Class() == Engine::UScriptStruct::StaticClass())
			{
				auto Struct = static_cast<Engine::UStruct*>(Object);

				Structs.push_back(Struct);
				GetStructNames(Struct);
			}
			else if (Object->Class() == Engine::UEnum::StaticClass())
			{
				auto Enum = static_cast<Engine::UEnum*>(Object);
				Enums.push_back(Enum);
				GetEnumNames(Enum);
			}
		});

	Buffer.Write<int>(NameMap.size());

	int CurrentNameIndex = 0;

	for (auto N : NameMap)
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
		Buffer.Write(NameMap[Enum->GetName()]);
		Buffer.Write<uint8_t>(Enum->Names.Num());

		for (size_t i = 0; i < Enum->Names.Num(); i++)
		{
			Buffer.Write<int>(NameMap[Enum->Names[i].Key]);
		}
	}

	Buffer.Write<uint32_t>(Structs.size());

	for (auto Struct : Structs)
	{
		Buffer.Write(NameMap[Struct->GetName()]);
		Buffer.Write<int32_t>(Struct->Super() ? NameMap[Struct->Super()->GetName()] : 0xffffffff);

		std::vector<FPropertyData> Properties;

		auto Props = Struct->GetProperties();
		unsigned short PropCount = 0;
		unsigned short SerializablePropCount = 0;

		while (Props)
		{
			FPropertyData Data(Props, PropCount);

			Properties.push_back(Data);
			Props = static_cast<Engine::FProperty*>(Props->GetNext());

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

			WriteProperty(P.Prop, P.Type);
		}
	}

	std::vector<uint8_t> UsmapData;

	switch (CompressionMethod)
	{
		case ECompressionMethod::None:
		default:
		{
			std::string UncompressedStream = Buffer.GetBuffer().str();
			UsmapData = std::vector<uint8_t>(UncompressedStream.size(), UncompressedStream[0]);
		}
	}

	auto FileOutput = FileWriter("Mappings.usmap");

	FileOutput.Write<uint16_t>(0x30C4); //magic
	FileOutput.Write<uint8_t>(0); //version
	FileOutput.Write(CompressionMethod); //compression: Oodle 
	FileOutput.Write<uint32_t>(UsmapData.size()); //compressed size
	FileOutput.Write<uint32_t>(Buffer.Size()); //decompressed size

	FileOutput.Write(UsmapData.data(), UsmapData.size());
}

template class Dumper<DefaultEngine<>>;
template class Dumper<Engine_Fortnite>;
template class Dumper<Engine_UE5>;