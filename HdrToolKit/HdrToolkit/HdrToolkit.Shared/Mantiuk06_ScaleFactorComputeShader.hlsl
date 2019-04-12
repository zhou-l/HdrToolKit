Texture2D<float> SourceTexture : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float GFIXATE = 0.1f;
	float EDGE_WEIGHT = 0.01f;
	float detectT = 0.001f;
	float a = 0.038737f;
	float b = 0.537756f;

	float g = max(detectT, abs(SourceTexture[position.xy]));
	TargetTexture[position.xy] = 1.0f / (a * pow(g, b));
}