cbuffer GammaCorrectionConstantBuffer : register(b0)
{
	float Gamma;
	float ConstantMultiplier;
}

Texture2D Texture : register(t0);

SamplerState Sampler : register(s0);

struct PixelShaderInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 color = Texture.Sample(Sampler, input.TexCoord).rgb;
	return float4(ConstantMultiplier * pow(color, Gamma), 1.0f);
}