Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint2 dimension;
	SourceTexture.GetDimensions(dimension.x, dimension.y);

	uint2 sourcePosition = 2 * position.xy;

	float3 minimum = 1.0f / 0.0f;

	uint2 offset;
	[unroll(2)]
	for (offset.y = 0; offset.y < 2; offset.y++)
	{
		[unroll(2)]
		for (offset.x = 0; offset.x < 2; offset.x++)
		{
			uint2 texPosition = sourcePosition.xy + offset.xy;

			if (all(texPosition < dimension))
			{
				minimum = min(minimum, SourceTexture[texPosition]);
			}
		}
	}

	TargetTexture[position.xy] = minimum;
}