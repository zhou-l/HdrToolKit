cbuffer TextureRenderConstantBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
}

struct VertexShaderInput
{
	float2 TexCoord : TEXCOORD;
};

struct PixelShaderInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(float4(input.TexCoord.x, 1.0f - input.TexCoord.y, 0.0f, 1.0f), ViewMatrix), ProjectionMatrix);
	output.TexCoord = input.TexCoord;

	return output;
}