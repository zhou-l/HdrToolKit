Texture2D<float3> SourceTexture : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

float Luminance(float3 color)
{
	return dot(color, float3(0.2126f, 0.7152f, 0.0722f));
}

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	TargetTexture[position.xy] = Luminance(SourceTexture[position.xy]);
}