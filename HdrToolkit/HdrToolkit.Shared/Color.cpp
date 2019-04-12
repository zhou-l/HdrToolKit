#include "pch.h"
#include "Color.h"

using namespace HdrToolkit;

GreyValueColor::GreyValueColor()
{
	Red = 0.0f;
	Alpha = 1.0f;
}

float GreyValueColor::Red::get()
{
	return m_color;
}

void GreyValueColor::Red::set(float value)
{
	m_color = value;
}

float GreyValueColor::Green::get()
{
	return m_color;
}

void GreyValueColor::Green::set(float value)
{
	m_color = value;
}

float GreyValueColor::Blue::get()
{
	return m_color;
}

void GreyValueColor::Blue::set(float value)
{
	m_color = value;
}

Color::Color()
{
	Red = 0.0f;
	Green = 0.0f;
	Blue = 0.0f;
	Alpha = 1.0f;
}