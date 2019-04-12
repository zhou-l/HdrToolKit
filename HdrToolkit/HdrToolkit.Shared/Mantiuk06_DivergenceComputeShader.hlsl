Texture2D<float> SourceTextures[2] : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float divergenceX;
	float divergenceY;
	if (position.x == 0)
	{
		divergenceX = SourceTextures[0][position.xy];
	}
	else
	{
		divergenceX = SourceTextures[0][position.xy] - SourceTextures[0][position.xy - uint2(1, 0)];
	}

	if (position.y == 0)
	{
		divergenceY = SourceTextures[1][position.xy];
	}
	else
	{
		divergenceY = SourceTextures[1][position.xy] - SourceTextures[1][position.xy - uint2(0, 1)];
	}

	TargetTexture[position.xy] += divergenceX + divergenceY;
}