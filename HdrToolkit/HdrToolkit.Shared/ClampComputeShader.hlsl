cbuffer OperationConstantBuffer : register(b0)
{
	float3 LowerBound;
	float padding0;
	float3 UpperBound;
	float padding1;
}

Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float4> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{

	TargetTexture[position.xy] = float4(float3(clamp(SourceTexture[position.xy], LowerBound, UpperBound)), 1.0);
}