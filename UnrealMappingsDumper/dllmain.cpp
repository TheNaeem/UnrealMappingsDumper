#include "pch.h"

#include "app.h"

#define CLEANUP() \
	delete App; \
	FreeLibraryAndExitThread(Module, NULL); \

constexpr bool OpenConsole = true;

void WINAPI Main(HMODULE Module)
{
	if constexpr (OpenConsole)
	{
		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
	}

	UE_LOG("Unreal Mappings Dumper created by OutTheShade");

	auto App = CreateAppInstance(EUnrealVersion::UE5); // TODO: a way to determine the engine version at runtime
	 
	if (!App) 
	{
		UE_LOG("Couldn't instantiate dumper instance. Returning.");
		CLEANUP();
		return;
	}

	if (!App->Init())
	{
		UE_LOG("Failed to initialize the dumper. Returning.");
		CLEANUP();
		return;
	}

	auto Start = std::chrono::steady_clock::now();

	App->Run(ECompressionMethod::Oodle);

	auto End = std::chrono::steady_clock::now();

	UE_LOG("Successfully generated mappings file in %.02f ms", (End - Start).count() / 1000000.);

	CLEANUP();
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
