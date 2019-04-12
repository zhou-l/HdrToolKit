cbuffer OperationConstantBuffer : register(b0)
{
	float3 FirstConstant;
	float padding;
	float3 SecondConstant;
}

Texture2D<float3> SourceTextures[2] : register(t0);

RWTexture2D<float4> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float3 colorVal = FirstConstant * SourceTextures[0][position.xy] +
		SecondConstant * SourceTextures[1][position.xy];

	//float3 colorVal = 0.5 * SourceTextures[0][position.xy] +
	//	0.5 * SourceTextures[1][position.xy];
	float4 targetVal = float4(colorVal, 1.0);
	// Clamping
	//targetVal = clamp(targetVal, 0.0, 1.0);
	TargetTexture[position.xy] = targetVal;
}