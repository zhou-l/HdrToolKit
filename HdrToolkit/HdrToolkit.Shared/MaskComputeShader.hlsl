cbuffer MaskConstantBuffer : register(b0)
{
	float Radius;
	float Threshold;
}

Texture2D<float3> OriginalTexture : register(t0);
Texture2D<float3> ToneMappedTexture : register(t1);
Texture2D<float> MaskTexture : register(t2);

RWTexture2D<float3> TargetTexture : register(u0);

float Luminance(float3 color)
{
	return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float3 color = OriginalTexture[position.xy] * MaskTexture[position.xy];
	if (Luminance(color) >= Threshold)
	{
		//TargetTexture[position.xy] = OriginalTexture[position.xy]; 
		TargetTexture[position.xy] = ToneMappedTexture[position.xy];
	}
}