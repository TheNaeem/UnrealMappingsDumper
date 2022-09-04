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
		auto Chunk = Inst->Objects[ChunkIndex]->Object;

		if (Chunk)
			return (Chunk + WithinChunkIndex);
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
std::initializer_list<ScanObject> DefaultEngine<T>::GetGObjectsPatterns()
{
	return  
	{
		PatternScanObject("48 89 05 ? ? ? ? E8 ? ? ? ? ? ? ? 0F 84", 3, true)
	};
}

template class DefaultEngine<UObjectDependency>;
template class DefaultEngine<FortniteUObjectBase>;

template <typename T>
typename DefaultEngine<T>::ObjObjects* DefaultEngine<T>::ObjObjects::Inst;