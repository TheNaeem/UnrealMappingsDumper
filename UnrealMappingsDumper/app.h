#pragma once

#include "dumper.h"

enum class EUnrealVersion
{
	UE4_26,
	UE4_27,
	UE5,
	UE5_01,
	FORTNITE
};

IDumper* CreateAppInstance(EUnrealVersion Version);

static void UE_LOG(const char* str, ...)
{
	va_list fmt;
	va_start(fmt, str);

	printf("[=] ");
	vprintf(str, fmt);
	printf("\n");

	va_end(fmt);
}