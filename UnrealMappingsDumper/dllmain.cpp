#include "pch.h"

#include "app.h"

void WINAPI Main()
{
	UE_LOG("Unreal Mappings Dumper created by OutTheShade");

	auto App = CreateAppInstance(EUnrealVersion::UE5);

	if (!App)
	{
		UE_LOG("Couldn't instantiate dumper instance. Returning.");
		return;
	}

	if (!App->Init())
	{
		UE_LOG("Failed to initialize the dumper. Returning.");
		return;
	}

	App->Run();

	delete App;
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
