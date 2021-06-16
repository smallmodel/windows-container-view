#pragma once

#include <queue>
#include <mutex>
#include "../Interface/sandbox_h.h"

class EventManager
{
private:
	std::queue<struct SandboxEvent> m_eventQueue;
	std::mutex m_mutex;

public:
	void EmitMouseMotionEvent(int x, int y);
	void EmitMouseKeyEvent(int buttonIndex, int x, int y, bool bKeyDown);
	void EmitKeyboardEvent(int keyIndex, bool bKeyDown);

	bool PopEvent(SandboxEvent& event);
};

extern EventManager gEventManager;
