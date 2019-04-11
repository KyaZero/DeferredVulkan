#include "stdafx.h"
#include "InputHandler.h"

#include <Windows.h>
#include <windowsx.h>

frostwave::InputHandler::InputHandler()
{
	for (size_t i = 0; i < 255; i++)
	{
		myKeyState[i] = false;
		myPreviousKeyState[i] = false;
	}
	for (size_t i = 0; i < 3; i++)
	{
		myMouseState[i] = false;
		myPreviousMouseState[i] = false;
	}
	myMousePosition.x = 0;
	myMousePosition.y = 0;
}

frostwave::InputHandler::~InputHandler()
{
}

void frostwave::InputHandler::Run()
{
	for (size_t i = 0; i < 3; i++)
	{
		myPreviousMouseState[i] = myMouseState[i];
	}
	for (size_t i = 0; i < 255; i++)
	{
		myPreviousKeyState[i] = myKeyState[i];
	}

	myMouseWheelDelta = 0;
	myPreviousMousePosition.x = myMousePosition.x;
	myPreviousMousePosition.y = myMousePosition.y;
}

bool frostwave::InputHandler::HandleEvents(u32 message, u64 wParam, u64 lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		myMousePosition.x = (i32)GET_X_LPARAM(lParam);
		myMousePosition.y = (i32)GET_Y_LPARAM(lParam);
		break;
	case WM_MOUSEWHEEL:
		myMouseWheelDelta = (i16)GET_WHEEL_DELTA_WPARAM(wParam);
		break;
	case WM_LBUTTONDOWN:
		myMouseState[0] = true;
		break;
	case WM_LBUTTONUP:
		myMouseState[0] = false;
		break;
	case WM_RBUTTONDOWN:
		myMouseState[1] = true;
		break;
	case WM_RBUTTONUP:
		myMouseState[1] = false;
		break;
	case WM_MBUTTONDOWN:
		myMouseState[2] = true;
		break;
	case WM_MBUTTONUP:
		myMouseState[2] = false;
		break;
	case WM_KEYDOWN:
		myKeyState[wParam] = true;
		break;
	case WM_KEYUP:
		myKeyState[wParam] = false;
		break;
	default:
		return false;
		break;
	}
	return true;
}

bool frostwave::InputHandler::IsKeyDown(Keys aKey) const
{
	return myKeyState[static_cast<i32>(aKey)];
}

bool frostwave::InputHandler::IsKeyUp(Keys aKey) const
{
	return !myKeyState[static_cast<i32>(aKey)];
}

bool frostwave::InputHandler::IsKeyPressed(Keys aKey) const
{
	return (myKeyState[static_cast<i32>(aKey)] && !myPreviousKeyState[static_cast<i32>(aKey)]);
}

bool frostwave::InputHandler::IsKeyReleased(Keys aKey) const
{
	return (!myKeyState[static_cast<i32>(aKey)] && myPreviousKeyState[static_cast<i32>(aKey)]);
}

bool frostwave::InputHandler::IsMouseButtonDown(MouseButtons aButton) const
{
	return myMouseState[static_cast<i32>(aButton)];
}

bool frostwave::InputHandler::IsMouseButtonUp(MouseButtons aButton) const
{
	return !myMouseState[static_cast<i32>(aButton)];
}

bool frostwave::InputHandler::IsMouseButtonPressed(MouseButtons aButton) const
{
	return (myMouseState[static_cast<i32>(aButton)] && !myPreviousMouseState[static_cast<i32>(aButton)]);
}

bool frostwave::InputHandler::IsMouseButtonReleased(MouseButtons aButton) const
{
	return (!myMouseState[static_cast<i32>(aButton)] && myPreviousMouseState[static_cast<i32>(aButton)]);
}

i16 frostwave::InputHandler::GetMouseWheelDelta() const
{
	return myMouseWheelDelta;
}

fw::Vector2i frostwave::InputHandler::GetMousePositionDelta() const
{
	Vector2i result;
	result.x = myMousePosition.x - myPreviousMousePosition.x;
	result.y = myMousePosition.y - myPreviousMousePosition.y;
	return result;
}

fw::Vector2i frostwave::InputHandler::GetAbsoluteMousePosition() const
{
	POINT point;
	GetCursorPos(&point);
	Vector2i result;
	result.x = static_cast<i32>(point.x);
	result.y = static_cast<i32>(point.y);
	return result;
}

void frostwave::InputHandler::SetMousePosition(i32 aX, i32 aY)
{
	SetCursorPos(aX, aY);
}