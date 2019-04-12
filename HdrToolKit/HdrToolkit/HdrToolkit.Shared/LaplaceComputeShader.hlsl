Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	int2 dimension;
	SourceTexture.GetDimensions(dimension.x, dimension.y);

	int x = position.x;
	int y = position.y;

	float3 laplacian = 0.0f;

	if (x - 1 >= 0 && x + 1 < dimension.x)
	{
		laplacian += SourceTexture[uint2(x + 1, y)] - 2 * SourceTexture[position.xy] + SourceTexture[uint2(x - 1, y)];
	}

	if (y - 1 >= 0 && y + 1 < dimension.y)
	{
		laplacian += SourceTexture[uint2(x, y + 1)] - 2 * SourceTexture[position.xy] + SourceTexture[uint2(x, y - 1)];
	}

	TargetTexture[position.xy] = laplacian;
}