#define MAX_LEVELS 16

cbuffer Reinhard02ConstantBuffer : register(b0)
{
	float Key;
	float Sharpness;
	float LogAverageLuminance;
	uint MaxLevels;
	float InitialScale;
	float ScaleStepFactor;
	float Threshold;
}

Texture2D<float3> Texture : register(t0);
Texture2D<float> BlurTextures[MAX_LEVELS] : register(t1);

RWTexture2D<float3> TargetTexture : register(u0);

float Luminance(float3 color)
{
	return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

float CorrectedLuminance(float luminance)
{
	return luminance * Key / LogAverageLuminance;
}

float SampleGauss(uint level, uint2 position)
{
	if (level == 0)
	{
		return Luminance(Texture[position]);
	}
	else if (level <= MAX_LEVELS)
	{
		return BlurTextures[level - 1][position];
	}
	else
	{
		return 0.0f;
	};
}

float CalculateV(float v1, float v2, float scale)
{
	return (v1 - v2) / (pow(2.0f, Sharpness) * Key / (scale * scale) + v1);
}

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float3 color = Texture[position.xy];

	float scale = InitialScale;

	float v1 = 0.0f;
	float v2 = CorrectedLuminance(SampleGauss(0, position.xy));

	uint maxIterations = min(MaxLevels, MAX_LEVELS);

	for (uint i = 0; i < maxIterations; i++)
	{
		v1 = v2;
		v2 = CorrectedLuminance(SampleGauss(i + 1, position.xy));

		if (abs(CalculateV(v1, v2, scale)) < Threshold)
		{
			break;
		}

		scale *= ScaleStepFactor;
	}

	TargetTexture[position.xy] = (color * Key / LogAverageLuminance) / (1.0f + v1);
}