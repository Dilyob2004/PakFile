#include "Crypto.h"
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iostream>
#include <vector>
int Crypto::GetSizeOfCheckSumSHA256()
{
    return EVP_MD_size(EVP_sha256());
}

std::vector<uint8_t> Crypto::GenerateBytes(int Bytes)
{
    std::vector<uint8_t> Buffer(Bytes);
    if (!RAND_bytes(Buffer.data(), Buffer.size())) {
        std::cerr << "Failed to generate random bytes!\n";
        return {};
    }
    return Buffer;
}
std::vector<uint8_t> Crypto::CreateChecksumSHA256(const uint8_t* Buffer, uint32_t Size)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
        return {};


    if (1 != EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr))
    {
        EVP_MD_CTX_free(ctx);
        return {};
    }

    if (1 != EVP_DigestUpdate(ctx, Buffer, Size))
    {
        EVP_MD_CTX_free(ctx);
        return {};
    }

    std::vector<uint8_t> checksum(EVP_MD_size(EVP_sha256()));
    uint32_t len;
    if (1 != EVP_DigestFinal_ex(ctx, checksum.data(), &len))
    {
        EVP_MD_CTX_free(ctx);
        return {};
    }

    EVP_MD_CTX_free(ctx);
    return checksum;
}

std::vector<uint8_t> Crypto::EncryptAES256(const uint8_t* data, size_t size, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);

        return {};
    }

    std::vector<uint8_t> ciphertext(size + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len;
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, data, static_cast<int>(size))) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    int ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

std::vector<uint8_t> Crypto::DecryptAES256(const uint8_t* data, size_t size, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    std::vector<uint8_t> plaintext(size + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len;
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, data, static_cast<int>(size)))
    {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len))
    {
        uint64_t errCode = ERR_get_error();
        char errMsg[120];
        ERR_error_string_n(errCode, errMsg, sizeof(errMsg));
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);

    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}