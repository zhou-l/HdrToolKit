Texture2D<float3> SourceTextures[2] : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] = SourceTextures[0][position.xy] * SourceTextures[1][position.xy];
}