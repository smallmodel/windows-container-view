#pragma once

#ifdef SANDBOXSERVER_EXPORTS
#define SANDBOXSERVER_API __declspec(dllexport)
#else
#define SANDBOXSERVER_API __declspec(dllimport)
#endif

typedef void (*OnBeginImageTransmission)(unsigned long Width, unsigned long Height, unsigned long Size);
typedef void (*OnReceivedImageData)(unsigned long Length, const unsigned char* BitmapData);
typedef void (*OnEndImageTransmission)();

void SANDBOXSERVER_API SandboxInitServer();
void SANDBOXSERVER_API SandboxInitVideo();
void SANDBOXSERVER_API SandboxSetBeginImageTransmission(OnBeginImageTransmission Function);
void SANDBOXSERVER_API SandboxSetReceivedImageData(OnReceivedImageData Function);
void SANDBOXSERVER_API SandboxSetEndImageTransmission(OnEndImageTransmission Function);
