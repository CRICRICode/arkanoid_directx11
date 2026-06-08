#include "inputclass.h"

namespace
{
	const unsigned int KEY_COUNT = 256;
}


InputClass::InputClass()
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	for (unsigned int i = 0; i < KEY_COUNT; i++)
	{
		m_keys[i] = false;
	}
}


void InputClass::KeyDown(unsigned int input)
{
	if (input < KEY_COUNT)
	{
		m_keys[input] = true;
	}
}


void InputClass::KeyUp(unsigned int input)
{
	if (input < KEY_COUNT)
	{
		m_keys[input] = false;
	}
}


bool InputClass::IsKeyDown(unsigned int key) const
{
	return key < KEY_COUNT && m_keys[key];
}
