Texture2D<float> SourceTexture : register(t0);

RWTexture2D<float> TargetTextures[2] : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint2 dimension;
	SourceTexture.GetDimensions(dimension.x, dimension.y);

	if (position.x < dimension.x)
	{
		TargetTextures[0][position.xy] = SourceTexture[position.xy + uint2(1, 0)] - SourceTexture[position.xy];
	}
	else
	{
		TargetTextures[0][position.xy] = 0.0f;
	}

	if (position.y < dimension.y)
	{
		TargetTextures[1][position.xy] = SourceTexture[position.xy + uint2(0, 1)] - SourceTexture[position.xy];
	}
	else
	{
		TargetTextures[1][position.xy] = 0.0f;
	}
}