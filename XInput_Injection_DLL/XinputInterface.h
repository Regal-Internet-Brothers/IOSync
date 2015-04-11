#pragma once

// Preprocessor related:
//#define WIN32_LEAN_AND_MEAN

// Includes:
#include <windows.h>

// Structures:
typedef struct _XINPUT_GAMEPAD
{
    WORD                                wButtons;
    BYTE                                bLeftTrigger;
    BYTE                                bRightTrigger;
    SHORT                               sThumbLX;
    SHORT                               sThumbLY;
    SHORT                               sThumbRX;
    SHORT                               sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_STATE
{
    DWORD                               dwPacketNumber;
    XINPUT_GAMEPAD                      Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_VIBRATION
{
    WORD                                wLeftMotorSpeed;
    WORD                                wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES
{
    BYTE                                Type;
    BYTE                                SubType;
    WORD                                Flags;
    XINPUT_GAMEPAD                      Gamepad;
    XINPUT_VIBRATION                    Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	typedef struct _XINPUT_BATTERY_INFORMATION
	{
		BYTE BatteryType;
		BYTE BatteryLevel;
	} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

	typedef struct _XINPUT_KEYSTROKE
	{
		WORD    VirtualKey;
		WCHAR   Unicode;
		WORD    Flags;
		BYTE    UserIndex;
		BYTE    HidCode;
	} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;
#endif