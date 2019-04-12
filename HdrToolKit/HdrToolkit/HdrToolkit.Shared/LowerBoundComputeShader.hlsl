cbuffer OperationConstantBuffer : register(b0)
{
	float3 LowerBound;
	uint padding;
}

Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] = max(SourceTexture[position.xy], LowerBound);
}