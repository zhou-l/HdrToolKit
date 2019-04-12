Texture2D<float3> SourceTextureRGB : register(t0);
Texture2D<float> SourceTextureFloat : register(t1);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] = SourceTextureRGB[position.xy] / SourceTextureFloat[position.xy];
}