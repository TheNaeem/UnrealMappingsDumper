#pragma once

static void UE_LOG(const char* str, ...)
{
	va_list fmt;
	va_start(fmt, str);

	printf("[=] ");
	vprintf(str, fmt);
	printf("\n");

	va_end(fmt);
}

namespace App
{
	bool Init();
}