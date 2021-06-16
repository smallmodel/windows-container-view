#pragma once

#ifdef SANDBOXCLIENT_EXPORTS
#define SANDBOXCLIENT_API __declspec(dllexport)
#else
#define SANDBOXCLIENT_API __declspec(dllimport)
#endif

void SANDBOXCLIENT_API SandboxInitClient(const wchar_t* IPAddress);
