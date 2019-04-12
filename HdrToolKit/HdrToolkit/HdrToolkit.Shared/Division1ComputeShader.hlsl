Texture2D<float3> SourceTexture : register(t0);
Texture2D<float3> OtherSourceTexture : register(t1);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float3 div = OtherSourceTexture[uint2(0, 0)];

	if (div.x != 0.0f)
		div.x = 1.0f / div.x;
	if (div.y != 0.0f)
		div.y = 1.0f / div.y;
	if (div.z != 0.0f)
		div.z = 1.0f / div.z;

	TargetTexture[position.xy] = SourceTexture[position.xy] * div;
}