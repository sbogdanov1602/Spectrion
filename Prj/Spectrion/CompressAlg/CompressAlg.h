// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the COMPRESSALG_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// COMPRESSALG_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef COMPRESSALG_EXPORTS
#define COMPRESSALG_API __declspec(dllexport)
#else
#define COMPRESSALG_API __declspec(dllimport)
#endif

COMPRESSALG_API void* GetCompressed(int* Data, int DataLength, int* PacketLength, int PacketType);
COMPRESSALG_API int* GetDecompressed(int* Block, int ReturnedLength);
