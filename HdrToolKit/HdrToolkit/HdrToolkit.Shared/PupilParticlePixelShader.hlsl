struct PixelShaderInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	if (dot(input.TexCoord, input.TexCoord) > 1.0f)
		discard;
	
	return 0.0f;
}