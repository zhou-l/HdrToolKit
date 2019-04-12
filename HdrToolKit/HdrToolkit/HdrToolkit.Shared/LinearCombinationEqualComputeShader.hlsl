cbuffer OperationConstantBuffer : register(b0)
{
	float TargetConstant;
	float SourceConstant;
	uint2 padding;
}

Texture2D<float> SourceTexture : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] = TargetConstant * TargetTexture[position.xy] + SourceConstant * SourceTexture[position.xy];
}