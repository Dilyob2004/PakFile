#include "Compression.h"

#include <zlib/zlib.h>
#include <lz4/lz4.h>
#include <lz4/lz4hc.h>


std::vector<uint8_t> Compression::CompressData(ECompressType Type, const uint8_t* Buffer, uint32_t Size, ECompressionLevel CompressionLevel)
{
    int Level = 0;
    std::vector<uint8_t> Result{ };
    switch (Type)
    {
    case ECompressType::ZLIB:

        if (CompressionLevel == ECompressionLevel::Default)
            Level = Z_DEFAULT_COMPRESSION;
        else if (CompressionLevel == ECompressionLevel::High)
            Level = Z_BEST_COMPRESSION;
        else if (CompressionLevel == ECompressionLevel::Low)
            Level = Z_BEST_SPEED;

        Result = CompressDataZlib(Buffer, Size, Level);
        break;

    case ECompressType::LZ4:
        Result = CompressDataLZ4(Buffer, Size);
        break;

    case ECompressType::LZ4HC:

        if (CompressionLevel == ECompressionLevel::Default)
            Level = LZ4HC_CLEVEL_DEFAULT;
        else if (CompressionLevel == ECompressionLevel::High)
            Level = LZ4HC_CLEVEL_MAX;
        else if (CompressionLevel == ECompressionLevel::Low)
            Level = LZ4HC_CLEVEL_MIN;

        Result = CompressDataLZ4(Buffer, Size, true, Level);
        break;
    default:
        Result = CompressDataZlib(Buffer, Size, -1);
        break;
    }
    return Result;
}

std::vector<uint8_t> Compression::DecompressData(ECompressType Type, const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size)
{
    std::vector<uint8_t> Result{ };
    switch (Type)
    {
    case ECompressType::ZLIB:
        Result = DecompressDataZlib(Buffer, CompressedSize, Size);
        break;
    case ECompressType::LZ4:
        Result = DecompressDataLZ4(Buffer, CompressedSize, Size);
        break;

    case ECompressType::LZ4HC:
        Result = DecompressDataLZ4(Buffer, CompressedSize, Size);
        break;
    default:
        Result = DecompressDataZlib(Buffer, CompressedSize, Size);
        break;
    }
    return Result;
}



std::vector<uint8_t> Compression::CompressDataLZ4(const uint8_t* Buffer, uint32_t Size, bool HighLevel, int CompressionLevel)
{
    int OriginalSize = static_cast<int>(Size);
    int CompressedSize = LZ4_compressBound(OriginalSize);

    std::vector<uint8_t> CompressedData(CompressedSize);
    int Result = HighLevel
        ? LZ4_compress_HC(reinterpret_cast<const char*>(Buffer), reinterpret_cast<char*>(CompressedData.data()), OriginalSize, CompressedSize, CompressionLevel)
        : LZ4_compress_default(reinterpret_cast<const char*>(Buffer), reinterpret_cast<char*>(CompressedData.data()), OriginalSize, CompressedSize);
    if (Result <= 0)
        return {};

    CompressedData.resize(Result);

    return CompressedData;
}

std::vector<uint8_t> Compression::DecompressDataLZ4(const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size)
{
    int OriginalSize = static_cast<int>(Size);


    std::vector<uint8_t> DecompressedData(OriginalSize);
    int Result = LZ4_decompress_safe(reinterpret_cast<const char*>(Buffer), reinterpret_cast<char*>(DecompressedData.data()), CompressedSize, OriginalSize);
    if (Result < 0)
        return {};

    DecompressedData.resize(Result);
    return DecompressedData;
}
std::vector<uint8_t> Compression::CompressDataZlib(const uint8_t* Buffer, uint32_t Size, int CompressionLevel)
{
    uLongf OriginalSize = static_cast<uLongf>(Size);
    uLongf CompressedSize = compressBound(OriginalSize);

    std::vector<uint8_t> CompressedData(CompressedSize);

    if (compress2(CompressedData.data(), &CompressedSize, Buffer, OriginalSize, CompressionLevel) != Z_OK)
        return {};

    CompressedData.resize(CompressedSize);

    return CompressedData;
}

std::vector<uint8_t> Compression::DecompressDataZlib(const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size)
{
    uLongf OriginalSize = static_cast<uLongf>(Size);

    std::vector<uint8_t> DecompressedData(OriginalSize);


    if (uncompress(DecompressedData.data(), &OriginalSize, Buffer, CompressedSize) != Z_OK)
        return {};

    DecompressedData.resize(OriginalSize);
    return DecompressedData;
}

