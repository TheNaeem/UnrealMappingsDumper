#pragma once

#include <urlmon.h>
#pragma comment(lib,"urlmon.lib")

constexpr const char* OodlePath = "Oodle.dll";
constexpr const char* OodleDownload = "https://cdn.discordapp.com/attachments/817251677086285848/992648087371792404/oo2core_9_win64.dll";

enum class OodleFormat
{
	Invalid = -1,
	None = 3,
	Kraken = 8,
	Leviathan = 13,
	Mermaid = 9,
	Selkie = 11,
	Hydra = 12,
	Count = 14,
	Force32 = 0x40000000
};

enum class OodleCompressionLevel : uint32_t
{
	None,
	SuperFast,
	VeryFast,
	Fast,
	Normal,
	Optimal1,
	Optimal2,
	Optimal3,
	Optimal4,
	Optimal5
};

typedef uint64_t(*_OodleCompressFunc)(
	OodleFormat format,
	void* buffer,
	int64_t bufferSize,
	void* outputBuffer,
	OodleCompressionLevel level,
	uint64_t a6,
	uint64_t a7,
	uint64_t a8);

inline _OodleCompressFunc OodleLZ_Compress;

class Oodle
{
	Oodle()
	{
		if (!std::filesystem::exists(OodlePath))
		{
			URLDownloadToFileA(NULL, OodleDownload, OodlePath, 0, NULL);
		}

		auto OodleHandle = LoadLibraryA(OodlePath);
		auto OodleCompressAddy = GetProcAddress(OodleHandle, "OodleLZ_Compress");
		OodleLZ_Compress = (_OodleCompressFunc)OodleCompressAddy;
	}

	static void EnsureInitialized()
	{
		static Oodle _;
	}

	static __forceinline uint32_t GetCompressionBound(uint32_t uncompressedSize)
	{
		return uncompressedSize + 274 * ((uncompressedSize + 0x3FFFF) / 0x40000);
	}

public:
	static std::vector<uint8_t> Compress(std::stringstream& uncompressedStream, OodleFormat format = OodleFormat::Kraken, OodleCompressionLevel level = OodleCompressionLevel::Optimal5)
	{
		auto streamData = uncompressedStream.str();
		return Compress((void*)streamData.c_str(), streamData.size(), format, level);
	}

	static std::vector<uint8_t> Compress(void* uncompressedBuf, int64_t bufSize, OodleFormat format = OodleFormat::Kraken, OodleCompressionLevel level = OodleCompressionLevel::Optimal5)
	{
		EnsureInitialized();

		auto compressedBuf = std::make_unique<uint8_t[]>(GetCompressionBound(bufSize));

		auto compressedSize = OodleLZ_Compress(
			format,
			uncompressedBuf,
			bufSize,
			compressedBuf.get(),
			level,
			0, 0, 0);

		return std::vector<uint8_t>(compressedBuf.get(), compressedBuf.get() + compressedSize);
	}
};