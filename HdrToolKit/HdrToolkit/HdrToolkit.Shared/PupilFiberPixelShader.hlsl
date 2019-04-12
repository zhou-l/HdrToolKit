struct PixelShaderInput
{
	float4 Position : SV_POSITION;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return 0.0f;
}