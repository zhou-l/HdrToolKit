cbuffer BlurConstantBuffer : register(b0)
{
	float StandardDeviation;
	float RadiusInStandardDeviations;
	float2 Direction;
}

Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	int2 dimension;
	SourceTexture.GetDimensions(dimension.x, dimension.y);

	float3 colorSum = 0.0f;
	float gaussSum = 0.0f;

	uint width = uint(ceil(2.0f * RadiusInStandardDeviations * StandardDeviation));
	int radius = int(width / 2);
	float variance = StandardDeviation * StandardDeviation;

	int2 delta = Direction.x != 0.0f ? int2(1, 0) : int2(0, 1);

	for (int dt = -radius; dt <= radius; dt++)
	{
		int2 texPosition = position.xy + dt * delta;

		if (all(texPosition >= 0) && all(texPosition < dimension))
		{
			float gauss = exp(-float(dt * dt) / (2.0f * variance));

			gaussSum += gauss;
			colorSum += gauss * SourceTexture[texPosition];
		}
	}

	TargetTexture[position.xy] = colorSum / gaussSum;
}