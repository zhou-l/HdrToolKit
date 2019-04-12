struct VertexShaderInput
{
	float2 Position : POSITION;
};

struct GeometryShaderInput
{
	float4 Position : SV_POSITION;
};

GeometryShaderInput main(VertexShaderInput input)
{
	GeometryShaderInput output;

	output.Position = float4(input.Position, 0.0f, 1.0f);

	return output;
}