#pragma once

#include <vector>

struct EncryptionKey
{
    EncryptionKey()
        : Key()
        , Iv()
    {
    }
    EncryptionKey(const std::vector<uint8_t>& InKey, const std::vector<uint8_t> & InIv)
        : Key(InKey)
        , Iv(InIv)
    {
    }
    EncryptionKey(const EncryptionKey& Copy)
        : Key(Copy.Key)
        , Iv(Copy.Iv)
    {
    }

    EncryptionKey(EncryptionKey&& Copy) noexcept
        : Key(std::move(Copy.Key))
        , Iv(std::move(Copy.Iv))
    {
    }
    
    EncryptionKey& operator = (const EncryptionKey& Other)
    {
        (*this).Key = Other.Key;
        (*this).Iv = Other.Iv;
        return *this;
    }

    EncryptionKey& operator = (EncryptionKey&& Other) noexcept
    {
        (*this).Key = std::move(Other.Key);
        (*this).Iv = std::move(Other.Iv);
        return *this;
    }



    std::vector<uint8_t> Key;
    std::vector<uint8_t> Iv;
};

class Crypto
{
public:
    static std::vector<uint8_t> GenerateBytes(int Bytes);

    static int GetSizeOfCheckSumSHA256();
    static std::vector<uint8_t> CreateChecksumSHA256(const uint8_t* Buffer, uint32_t Size);

    static std::vector<uint8_t> EncryptAES256(const uint8_t* data, size_t size, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);
    static std::vector<uint8_t> DecryptAES256(const uint8_t* data, size_t size, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

};