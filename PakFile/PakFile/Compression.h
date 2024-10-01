#pragma once

#include <vector>
enum class ECompressType
{
    ZLIB,
    LZ4,
    LZ4HC
};
enum class ECompressionLevel
{
    Low,
    High,
    Default
};
class Compression
{
public:
    static std::vector<uint8_t> CompressData(ECompressType Type, const uint8_t* Buffer, uint32_t Size, ECompressionLevel CompressionLevel = ECompressionLevel::Default);

    static std::vector<uint8_t> DecompressData(ECompressType Type, const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size);
private:
    static std::vector<uint8_t> CompressDataLZ4(const uint8_t* Buffer, uint32_t Size, bool HighLevel = false, int CompressionLevel = 1);

    static std::vector<uint8_t> DecompressDataLZ4(const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size);
    static std::vector<uint8_t> CompressDataZlib(const uint8_t* Buffer, uint32_t Size, int CompressionLevel);

    static std::vector<uint8_t> DecompressDataZlib(const uint8_t* Buffer, uint32_t CompressedSize, uint32_t Size);
};
