#pragma once





#include "Crypto.h"
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>

struct FileEntry
{
    FileEntry()
        : RelativePath("")
        , FileName("")
        , Offset(0)
        , OriginalSize(0)
        , CompressedSize(0)
        , EncryptedSize(0)
        , CheckSum()
    {
    }
    std::string         RelativePath;
    std::string         FileName;
    std::streampos      Offset;
    std::size_t              OriginalSize;
    std::size_t              CompressedSize;
    std::size_t              EncryptedSize;
    std::vector<uint8_t>  CheckSum;
};
class PakFile
{
public:

    PakFile();
    PakFile(const EncryptionKey&);
    ~PakFile();
    bool Initialize(const EncryptionKey& );
    bool WritePak(const std::string&, const std::vector<std::string>&);
    bool ReadPak(const std::string&, std::map<std::string, std::vector<uint8_t>>& OutFiles);


private:
    EncryptionKey Encryption;
};