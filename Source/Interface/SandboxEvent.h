#pragma once

enum SandboxEventType
{
	SBET_KeyEvent,
	SBET_MouseEvent
};

struct SandboxMouseEvent
{
	int buttonIndex;
	int bKeyDown;
	int x;
	int y;
};

struct SandboxKeyboardEvent
{
	int keyIndex;
	int bKeyDown;
};

struct SandboxEvent
{
	int eventType;

	[switch_is(eventType)] union
	{
		[case(0)] struct SandboxKeyboardEvent keyboardEvent;
		[case(1)] struct SandboxMouseEvent mouseEvent;
		[default] ;
	};
};
