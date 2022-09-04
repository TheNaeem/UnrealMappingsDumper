#pragma once

#include "engine.h"

class IDumper
{
public:

	virtual bool Init(uintptr_t GObjectsOverride = 0, uintptr_t FNameToStringOverride = 0) = 0;
};

template <typename Engine>
class DumperInternal
{
public:

	bool Init(uintptr_t GObjectsOverride, uintptr_t FNameToStringOverride);
};

template <typename Engine = Engine_UE5>
class Dumper : public IDumper
{
	DumperInternal<Engine> Impl;

public:

	bool Init(uintptr_t GObjectsOverride, uintptr_t FNameToStringOverride) override
	{
		return Impl.Init(GObjectsOverride, FNameToStringOverride);
	}
};