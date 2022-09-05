#pragma once

#include "engine.h"

class IDumper
{
public:

	virtual bool Init(uintptr_t GObjectsOverride = 0, uintptr_t FNameToStringOverride = 0) = 0;
	virtual void Run(ECompressionMethod CompressionMethod = ECompressionMethod::None) const = 0;
};

template <typename Engine = Engine_UE5>
class Dumper : public IDumper
{
public:

	struct FPropertyData
	{
		Engine::FProperty* Prop;
		uint16_t Index;
		uint8_t ArrayDim;
		Engine::FName Name;
		EPropertyType Type;

		FPropertyData(Engine::FProperty* P, int Idx) :
			Prop(P),
			Index(Idx),
			ArrayDim(P->GetArrayDim()),
			Name(P->GetName()),
			Type(P->GetPropertyType())
		{
		}
	};

	bool Init(uintptr_t GObjectsOverride, uintptr_t FNameToStringOverride) override;
	void Run(ECompressionMethod CompressionMethod) const override;
};