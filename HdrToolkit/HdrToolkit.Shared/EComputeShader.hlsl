cbuffer PupilConstantBuffer : register(b0)
{
	float PupilRadius;
}

Texture2D<float> ApertureTexture : register(t0);

RWTexture2D<float3> RealTargetTexture : register(u0);
RWTexture2D<float3> ImagTargetTexture : register(u1);

static const float PI = 3.14159265f;

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint2 dimension;
	ApertureTexture.GetDimensions(dimension.x, dimension.y);

	// coordinates between -1 and 1
	float2 texCoord = float2(2 * position.xy) / dimension - 1.0f;

	float realFactor = 0.0f;
	float imagFactor = 0.0f;

	float distance2 = dot(texCoord, texCoord);
	if (distance2 <= PupilRadius * PupilRadius)
	{
		float a = 10.0f / 4.0f * PI * distance2;
		sincos(a, imagFactor, realFactor);
	}

	RealTargetTexture[position.xy] = ApertureTexture[position.xy].rrr * realFactor;
	ImagTargetTexture[position.xy] = ApertureTexture[position.xy].rrr * imagFactor;
}