#include "Event.h"
#include "../Interface/sandbox_h.h"

EventManager gEventManager;

void EventManager::EmitMouseMotionEvent(int x, int y)
{
	SandboxEvent event;
	event.eventType = SBET_MouseEvent;
	event.mouseEvent.bKeyDown = false;
	event.mouseEvent.buttonIndex = -1;
	event.mouseEvent.x = x;
	event.mouseEvent.y = y;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_eventQueue.push(event);
}

void EventManager::EmitMouseKeyEvent(int buttonIndex, int x, int y, bool bKeyDown)
{
	SandboxEvent event;
	event.eventType = SBET_MouseEvent;
	event.mouseEvent.bKeyDown = bKeyDown;
	event.mouseEvent.buttonIndex = buttonIndex;
	event.mouseEvent.x = x;
	event.mouseEvent.y = y;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_eventQueue.push(event);
}

void EventManager::EmitKeyboardEvent(int keyIndex, bool bKeyDown)
{
	SandboxEvent event;
	event.eventType = SBET_KeyEvent;
	event.keyboardEvent.bKeyDown = bKeyDown;
	event.keyboardEvent.keyIndex = keyIndex;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_eventQueue.push(event);
}

bool EventManager::PopEvent(SandboxEvent& event)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_eventQueue.empty())
	{
		return false;
	}

	event = m_eventQueue.front();
	m_eventQueue.pop();
	return true;
}
