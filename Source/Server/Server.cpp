#include "Server.h"
#include "ServerPrivate.h"
#include <rpc.h>
#include "../Interface/sandbox_h.h"

void SandboxInitServer()
{
	RPC_STATUS Status;

	Status = RpcServerUseProtseq(
		(RPC_WSTR)TEXT("ncacn_ip_tcp"),
		RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
		NULL
	);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	Status = RpcServerRegisterIfEx(
		Sandbox_v1_0_s_ifspec,
		NULL,
		NULL,
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
		10,
		NULL
	);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	RPC_BINDING_VECTOR* BindingVector;
	Status = RpcServerInqBindings(&BindingVector);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	Status = RpcEpRegister(
		Sandbox_v1_0_s_ifspec,
		BindingVector,
		NULL,
		NULL
	);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}

	Status = RpcServerListen(
		1,
		10,
		1
	);

	if (Status != RPC_S_OK)
	{
		throw Status;
	}
}

void SANDBOXSERVER_API SandboxInitVideo()
{
	InitVideo();
}

/*
void SANDBOXSERVER_API SandboxSetBeginImageTransmission(OnBeginImageTransmission Function)
{
	BeginImageTransmission = Function;
}

void SANDBOXSERVER_API SandboxSetReceivedImageData(OnReceivedImageData Function)
{
	ReceivedImageData = Function;
}

void SANDBOXSERVER_API SandboxSetEndImageTransmission(OnEndImageTransmission Function)
{
	EndImageTransmission = Function;
}
*/
