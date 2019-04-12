#define MAX_LEVELS 16

cbuffer Reinhard02ConstantBuffer : register(b0)
{
	float Key;
	float Sharpness;
	float LogAverageLuminance;
	uint MaxLevels;
	float InitialScale;
	float ScaleStepFactor;
	float Threshold;
}

Texture2D Texture : register(t0);
Texture2D BlurTexture0 : register(t1);
Texture2D BlurTexture1 : register(t2);
Texture2D BlurTexture2 : register(t3);
Texture2D BlurTexture3 : register(t4);
Texture2D BlurTexture4 : register(t5);
Texture2D BlurTexture5 : register(t6);
Texture2D BlurTexture6 : register(t7);
Texture2D BlurTexture7 : register(t8);
Texture2D BlurTexture8 : register(t9);
Texture2D BlurTexture9 : register(t10);
Texture2D BlurTexture10 : register(t11);
Texture2D BlurTexture11 : register(t12);
Texture2D BlurTexture12 : register(t13);
Texture2D BlurTexture13 : register(t14);
Texture2D BlurTexture14 : register(t15);
Texture2D BlurTexture15 : register(t16);

SamplerState Sampler : register(s0);

struct PixelShaderInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

float Luminance(float4 color)
{
	return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

float CorrectedLuminance(float luminance)
{
	return luminance * Key / LogAverageLuminance;
}

float SampleGauss(uint level, float2 texCoord)
{
	switch (level)
	{
	case 0:
		return Luminance(Texture.Sample(Sampler, texCoord));
	case 1:
		return BlurTexture0.Sample(Sampler, texCoord).r;
	case 2:
		return BlurTexture1.Sample(Sampler, texCoord).r;
	case 3:
		return BlurTexture2.Sample(Sampler, texCoord).r;
	case 4:
		return BlurTexture3.Sample(Sampler, texCoord).r;
	case 5:
		return BlurTexture4.Sample(Sampler, texCoord).r;
	case 6:
		return BlurTexture5.Sample(Sampler, texCoord).r;
	case 7:
		return BlurTexture6.Sample(Sampler, texCoord).r;
	case 8:
		return BlurTexture7.Sample(Sampler, texCoord).r;
	case 9:
		return BlurTexture8.Sample(Sampler, texCoord).r;
	case 10:
		return BlurTexture9.Sample(Sampler, texCoord).r;
	case 11:
		return BlurTexture10.Sample(Sampler, texCoord).r;
	case 12:
		return BlurTexture11.Sample(Sampler, texCoord).r;
	case 13:
		return BlurTexture12.Sample(Sampler, texCoord).r;
	case 14:
		return BlurTexture13.Sample(Sampler, texCoord).r;
	case 15:
		return BlurTexture14.Sample(Sampler, texCoord).r;
	case 16:
		return BlurTexture15.Sample(Sampler, texCoord).r;
	default:
		return 0.0f;
	}
}

float CalculateV(float v1, float v2, float scale)
{
	return (v1 - v2) / (pow(2.0f, Sharpness) * Key / (scale * scale) + v1);
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float luminanceScale = Key / LogAverageLuminance;
	float4 color = Texture.Sample(Sampler, input.TexCoord);

	float scale = InitialScale;

	float v1 = 0.0f;
	float v2 = CorrectedLuminance(SampleGauss(0, input.TexCoord));

	uint maxIterations = min(MaxLevels, MAX_LEVELS);

	[unroll(MAX_LEVELS)]
	for (uint i = 0; i < maxIterations; i++)
	{
		v1 = v2;
		v2 = CorrectedLuminance(SampleGauss(i + 1, input.TexCoord));

		if (abs(CalculateV(v1, v2, scale)) < Threshold)
		{
			i = maxIterations;
		}

		scale *= ScaleStepFactor;
	}

	return float4(color.rgb * luminanceScale / (1.0f + v1), 1.0f);
}