#include "pch.h"

#include "engine.h"
#include "unrealFunctions.h"

std::wstring_view FNameBase::AsString()
{
	FString Ret;
	FNameToString(this, Ret);

	if (Ret.Data() != nullptr)
	{
		return std::wstring_view(Ret.Data());
	}

	return {};
}

std::string FNameBase::ToString()
{
	auto Ret = AsString();

	return std::string(Ret.begin(), Ret.end());
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
		FUObjectItem* Chunk = Inst->Objects[ChunkIndex]->Object;

		if (Chunk)
			return (Chunk + WithinChunkIndex);
	}

	return nullptr;
}

template <typename T>
void DefaultEngine<T>::ObjObjects::ForEach(std::function<void(UObject*&)> Action)
{
	for (size_t i = 0; i < Num(); i++)
	{
		auto Obj = GetObjectByIndex(i);

		if (!Obj) continue;

		Action(Obj);
	}
}

template <typename T>
const std::initializer_list<ScanObject> DefaultEngine<T>::GetGObjectsPatterns()
{
	const auto Ret =
	{
		PatternScanObject("48 89 05 ? ? ? ? E8 ? ? ? ? ? ? ? 0F 84", true, 3)
	};

	return Ret;
}