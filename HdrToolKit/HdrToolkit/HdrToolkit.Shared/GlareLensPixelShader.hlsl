cbuffer GlareLensConstantBuffer : register(b0)
{
	float4 CircleInfo; // x,y are center, z, w are radius
	float4 TexInfo; // x, y are size
};

Texture2D TextureInLens : register(t0);
Texture2D TextureAtBack : register(t1);

SamplerState Sampler : register(s0);

struct PixelShaderInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{

	// Try clipping here
	float2 pos = input.TexCoord.xy;
	float2 pixPos;
	pixPos.x = ceil(pos.x * float(TexInfo.x) - 0.5f);
	pixPos.y = ceil(pos.y * float(TexInfo.y) - 0.5f);
	
	float2 pixLensCenter = CircleInfo.xy;
	float Radius = CircleInfo.z;
	float pixDist2Center = length(pixPos - pixLensCenter);

	// Draw the circle
	float2 scPos = float2(input.TexCoord.x * (TexInfo.x - 1), input.TexCoord.y * (TexInfo.y - 1));
	float scDist2Center = length(scPos.xy - CircleInfo.xy);
	float halfStrokeThick = 2.5;
	float4 circleColor = float4(0, 0, 0, 0);
	
	if (scDist2Center >= Radius - halfStrokeThick  && scDist2Center <= Radius + halfStrokeThick) // Lens decoration
	{
		float opacity = 1 -  abs(scDist2Center - Radius) / halfStrokeThick;
		circleColor = float4(0.8, 0.8, 0.8, clamp(opacity, 0, 1));
	}
	float3 color = float3(0, 0, 0);
	if (pixDist2Center < Radius) // Within the lens 
	{
		float3 lensColor = TextureInLens.Sample(Sampler, input.TexCoord).rgb;
		float3 backColor = TextureAtBack.Sample(Sampler, input.TexCoord).rgb;
		//float3 color = float3(1, 0, 0);
		float a = 0.8; // the foreground alpha
		 color = a * lensColor + (1.0 - a) * backColor;
		
	}
	else
	{
		 color = TextureAtBack.Sample(Sampler, input.TexCoord).rgb;
	}

	float3 finalColor = circleColor.a * circleColor.rgb + (1.0 - circleColor.a) * color.rgb;

	return float4(finalColor, 1.0);
}