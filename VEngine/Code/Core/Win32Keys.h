#pragma once

#include <Windows.h>

//All these values are Win32 virtual keys currently in use.
enum class Keys
{
	None = 0,
	Space = 0x20,
	Enter = 0x0D,
	Esc = 0x1B,
	Tab = 0x09,
	Tilde = 0xC0,
	Ctrl = 0x11,
	Shift = 0x10,
	Delete = 0x2E,
	BackSpace = 0x08,
	//@Todo: there's all sorts of scancode shit with Win32 and keyboards that won't make this work internationally.
	W = 'W',
	M = 'M',
	A = 'A',
	S = 'S',
	D = 'D',
	F = 'F',
	Y = 'Y',
	Z = 'Z',
	X = 'X',
	P = 'P',
	E = 'E',
	R = 'R',
	O = 'O',
	G = 'G',
	V = 'V',
	B = 'B',
	Q = 'Q',
	T = 'T',
	C = 'C',
	I = 'I',
	Num1 = '1',
	Num2 = '2',
	Num3 = '3',
	Num4 = '4',
	Num5 = '5',
	Num6 = '6',
	Num7 = '7',
	Num8 = '8',
	Num9 = '9',
	Num0 = '0',
	F1 = 0x70,
	F2 = 0x71,
	F3 = 0x72,
	F4 = 0x73,
	F8 = 0x77,
	F11 = 0x7A,
	Up = 0x26,
	Down = 0x28,
	Right = 0x27,
	Left = 0x25,
	NumPad1 = VK_NUMPAD1,
	NumPad2 = VK_NUMPAD2,
	NumPad3 = VK_NUMPAD3,
	NumPad5 = VK_NUMPAD5,
};
