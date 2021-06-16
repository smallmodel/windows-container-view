#include <Windows.h>
#include "Server.h"
#include "ServerPrivate.h"
#include "Event.h"

extern "C" void RpcBeginImage(
	handle_t IDL_handle,
	unsigned long Width,
	unsigned long Height,
	unsigned long Size
)
{
	BeginImageTransmission(Width, Height, Size);
}

extern "C" void RpcSendImageChunkData(
	handle_t IDL_handle,
	unsigned long BitmapSize,
	const unsigned char* BitmapData
)
{
	ReceivedImageData(BitmapSize, BitmapData);
}

extern "C" void RpcEndImage(handle_t IDL_handle)
{
	EndImageTransmission();
}

extern "C" boolean RpcPopEvent(handle_t IDL_handle, struct SandboxEvent* Event)
{
	return gEventManager.PopEvent(*Event);
}
