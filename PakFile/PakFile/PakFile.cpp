#include "PakFile.h"
#include "Compression.h"
#include <chrono>
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;



PakFile::PakFile()
    : Encryption()
{
}

PakFile::PakFile(const EncryptionKey& Key)
{
    Initialize(Key);
}
PakFile::~PakFile()
{

}
bool PakFile::Initialize(const EncryptionKey& Key)
{
    Encryption = Key;
    return true;
}
bool PakFile::WritePak(const std::string& Name, const std::vector<std::string>& Files)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Start create Pak file.\n";
    std::map<std::string, FileEntry> FileEntries;

    std::ofstream PakFile(Name, std::ios::binary);

    if (!PakFile.is_open())
    {
        std::cerr << "Error creating Pak file!\n";
        return false;
    }

    std::streampos DataOffset = 0;
    for (const auto& Path : Files)
    {
        std::ifstream FileReader(Path, std::ios::binary | std::ios::ate);
        if (!FileReader.is_open())
        {
            std::cerr << "Error opening file: " <<  Path << std::endl;
            PakFile.clear();
            PakFile.close();
            return false;
        }

        uint32_t FileSize = static_cast<uint32_t>(FileReader.tellg());
        std::vector<uint8_t> Buffer(FileSize);

        FileReader.seekg(0, std::ios::beg);
        FileReader.read(reinterpret_cast<char*>(Buffer.data()), Buffer.size());
        FileReader.close();



        std::vector<uint8_t> CompressedBuffer = Compression::CompressData(ECompressType::ZLIB, Buffer.data(), Buffer.size());

        if (CompressedBuffer.empty())
        {
            std::cerr << "Error compressed file: " << Path << std::endl;
            PakFile.clear();
            PakFile.close();
            return false;
        }

        std::vector<uint8_t> EncryptedBuffer = Crypto::EncryptAES256(CompressedBuffer.data(), CompressedBuffer.size(), Encryption.Key, Encryption.Iv);

        if (EncryptedBuffer.empty())
        {
            std::cerr << "Error Encrypted file: " << Path << std::endl;
            PakFile.clear();
            PakFile.close();
            return false;
        }

        PakFile.seekp(DataOffset, std::ios::beg);
        PakFile.write(reinterpret_cast<const char*>(EncryptedBuffer.data()), EncryptedBuffer.size());


        FileEntry Entry;
        Entry.RelativePath = fs::path(Path).parent_path().filename().string() + "/" + fs::path(Path).filename().string();
        Entry.FileName = fs::path(Path).filename().string();
        Entry.Offset = DataOffset;
        Entry.OriginalSize = Buffer.size();
        Entry.CompressedSize = CompressedBuffer.size();
        Entry.EncryptedSize = EncryptedBuffer.size();
        Entry.CheckSum = Crypto::CreateChecksumSHA256(Buffer.data(), Buffer.size());
        FileEntries[Entry.RelativePath] = Entry;
        DataOffset += EncryptedBuffer.size();

        std::cout << "File " << Entry.FileName << " archived in pak.\n";

    }

    std::streampos MetaOffset = DataOffset;
    PakFile.seekp(MetaOffset, std::ios::beg);
    for (const auto& [RelativePath, Entry] : FileEntries)
    {
        PakFile.write(Entry.RelativePath.c_str(), Entry.RelativePath.size() + 1);
        PakFile.write(Entry.FileName.c_str(), Entry.FileName.size() + 1);

        PakFile.write(reinterpret_cast<const char*>(&Entry.Offset), sizeof(Entry.Offset));
        PakFile.write(reinterpret_cast<const char*>(&Entry.OriginalSize), sizeof(Entry.OriginalSize));
        PakFile.write(reinterpret_cast<const char*>(&Entry.CompressedSize), sizeof(Entry.CompressedSize));
        PakFile.write(reinterpret_cast<const char*>(&Entry.EncryptedSize), sizeof(Entry.EncryptedSize));
        PakFile.write(reinterpret_cast<const char*>(Entry.CheckSum.data()), Entry.CheckSum.size());

        if (PakFile.fail())
        {
            std::cerr << "Error Write to file a meta data: " << Entry.FileName << std::endl;
            return false;
        }

    }

    PakFile.write(reinterpret_cast<const char*>(&MetaOffset), sizeof(MetaOffset));
    PakFile.close();

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "Pak file created, time elapsed: " << duration.count() << std::endl;
    return true;
}

bool PakFile::ReadPak(const std::string& pakFilename, std::map<std::string, std::vector<uint8_t>>& OutFiles)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "Start create Pak file.\n";

    std::map<std::string, FileEntry> FileEntries;

    std::ifstream PakFile(pakFilename, std::ios::binary);
    if (!PakFile.is_open())
    {
        std::cerr << "Pak File " << pakFilename << " does not found!" << std::endl;

        return false;
    }

    PakFile.seekg(-static_cast<std::streamoff>(sizeof(std::streampos)), std::ios::end);

    std::streampos MetaOffset;
    PakFile.read(reinterpret_cast<char*>(&MetaOffset), sizeof(MetaOffset));

    PakFile.seekg(0, std::ios::end);

    std::streampos FileSize = PakFile.tellg();
    std::streampos MetaEnd = FileSize - static_cast<std::streamoff>(sizeof(std::streampos));

    PakFile.seekg(MetaOffset, std::ios::beg);
    while (PakFile.tellg() < MetaEnd)
    {
        FileEntry Entry;

        std::getline(PakFile, Entry.RelativePath, '\0');
        std::getline(PakFile, Entry.FileName, '\0');

        PakFile.read(reinterpret_cast<char*>(&Entry.Offset), sizeof(Entry.Offset));
        PakFile.read(reinterpret_cast<char*>(&Entry.OriginalSize), sizeof(Entry.OriginalSize));
        PakFile.read(reinterpret_cast<char*>(&Entry.CompressedSize), sizeof(Entry.CompressedSize));
        PakFile.read(reinterpret_cast<char*>(&Entry.EncryptedSize), sizeof(Entry.EncryptedSize));

        Entry.CheckSum.resize(Crypto::GetSizeOfCheckSumSHA256());

        PakFile.read(reinterpret_cast<char*>(Entry.CheckSum.data()), Entry.CheckSum.size());


        if (PakFile.fail())
        {
            std::cerr << "Error Read a meta data file: " <<  Entry.FileName << std::endl;

            PakFile.close();
            return false;
        }

        FileEntries[Entry.RelativePath] = Entry;
    }

    for (const auto& [RelativePath, Entry] : FileEntries)
    {
        PakFile.seekg(Entry.Offset, std::ios::beg);

        std::vector<uint8_t> Buffer(Entry.EncryptedSize);

        PakFile.read(reinterpret_cast<char*>(Buffer.data()), Entry.EncryptedSize);

        std::vector<uint8_t> Decrypted = Crypto::DecryptAES256(Buffer.data(), Buffer.size(), Encryption.Key, Encryption.Iv);


        if (Decrypted.empty())
        {
            std::cerr << "Decrypt AES256 data size mismatch for file: " << Entry.FileName << std::endl;

            PakFile.close();
            return false;
        }

        std::vector<uint8_t> DecompressedBuffer = Compression::DecompressData(ECompressType::ZLIB, Decrypted.data(), (uint32_t)Entry.CompressedSize, (uint32_t)Entry.OriginalSize);


        if (DecompressedBuffer.empty() || DecompressedBuffer.size() != Entry.OriginalSize)
        {
            std::cerr << "Decompressed data size mismatch for file: " << Entry.FileName << std::endl;
            PakFile.close();

            return false;
        }

        if (Crypto::CreateChecksumSHA256(DecompressedBuffer.data(), DecompressedBuffer.size()) != Entry.CheckSum)
        {
            std::cerr << "Checksum mismatch for file: " << Entry.FileName << std::endl;

            PakFile.close();
            return false;
        }
        std::cerr << "File " << Entry.FileName <<  "extracted, size: " << DecompressedBuffer.size() << "bytes."<< std::endl;

        
        OutFiles[Entry.RelativePath] = std::move(DecompressedBuffer);

    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "Files extracted from Pak, time elapsed: " << duration.count() << std::endl;
    PakFile.close();
    return true;
}