cbuffer OperationConstantBuffer : register(b0)
{
	int2 Shift;
	uint2 padding;
}

Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float3> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint2 dimension;
	SourceTexture.GetDimensions(dimension.x, dimension.y);

	TargetTexture[position.xy] = SourceTexture[uint2(int2(position.xy + dimension) - Shift) % dimension];
}