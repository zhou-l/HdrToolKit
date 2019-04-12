Texture2D<float> SourceTexture : register(t0);
Texture2D<float3> ColorMap : register(t1);

RWTexture2D<float3> TargetTexture : register(u0);

SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	//TargetTexture[position.xy] = float4(ColorMap.SampleLevel(Sampler, float2(SourceTexture[position.xy], 0), 0), 1.0);
	TargetTexture[position.xy] = ColorMap.SampleLevel(Sampler, float2(SourceTexture[position.xy], 0), 0);
}