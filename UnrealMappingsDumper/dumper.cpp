#include "pch.h"

#include "dumper.h"
#include "unrealFunctions.h"
#include "app.h"

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
void Dumper<Engine>::Run()
{
	Engine::ObjObjects::ForEach([](Engine::UObject*& Object)
		{

		});
}

template class Dumper<DefaultEngine<>>;
template class Dumper<Engine_Fortnite>;
template class Dumper<Engine_UE5>;