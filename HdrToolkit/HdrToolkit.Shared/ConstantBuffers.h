#pragma once
#include "pch.h"

namespace HdrToolkit
{
	struct ConstantBuffer_Reinhard02
	{
		float Key;
		float Sharpness;
		float LogAverageLuminance;
		uint32_t MaxLevels;

		float InitialScale;
		float ScaleStepFactor;
		float Threshold;
		uint32_t padding;
	};

	struct ConstantBuffer_MaskBuffer
	{
		float Radius;
		float Threshold;
		uint32_t padding0;
		uint32_t padding1;
	};
}