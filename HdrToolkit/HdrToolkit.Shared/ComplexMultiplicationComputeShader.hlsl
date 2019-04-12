Texture2D<float3> FirstRealSourceTexture : register(t0);
Texture2D<float3> FirstImagSourceTexture : register(t1);
Texture2D<float3> SecondRealSourceTexture : register(t2);
Texture2D<float3> SecondImagSourceTexture : register(t3);

RWTexture2D<float3> RealTargetTexture : register(u0);
RWTexture2D<float3> ImagTargetTexture : register(u1);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float3 a = FirstRealSourceTexture[position.xy];
	float3 b = FirstImagSourceTexture[position.xy];

	float3 c = SecondRealSourceTexture[position.xy];
	float3 d = SecondImagSourceTexture[position.xy];

	RealTargetTexture[position.xy] = (a * c - b * d);
	ImagTargetTexture[position.xy] = (b * c + a * d);
}