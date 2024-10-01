
#include "PakFile.h"

int main()
{


	EncryptionKey Key(Crypto::GenerateBytes(32), Crypto::GenerateBytes(16));

	PakFile pak(Key);

	std::vector<std::string> OutReaderFiles = 
	{ 
		"../Content/BistroExterior.bin",
		"../Content/BistroInterior.bin",
		"../Content/Hair.bin",
		"../Content/Kitchen.bin",
		"../Content/ShaderBalls.bin",
	};

	pak.WritePak("../Content/Resources.pak", OutReaderFiles);

	std::map<std::string, std::vector<uint8_t>> OutFiles;
	pak.ReadPak("../Content/Resources.pak", OutFiles);

	for (auto [Key, Value] : OutFiles)
	{
		std::cout << Key << std::endl;
	}



	return 0;
}