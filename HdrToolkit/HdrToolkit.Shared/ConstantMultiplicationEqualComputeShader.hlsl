cbuffer OperationConstantBuffer : register(b0)
{
	float Constant;
	uint3 padding;
}

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] *= Constant;
}