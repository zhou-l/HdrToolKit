cbuffer ConstantBuffer : register(b0)
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
}

cbuffer PupilConstantBuffer : register(b1)
{
	float Size;
}

struct GeometryShaderInput
{
	float4 Position : SV_POSITION;
};

struct PixelShaderInput
{
	float4 Position : SV_POSITION;
};

[maxvertexcount(6)]
void main(point GeometryShaderInput input[1], inout TriangleStream< PixelShaderInput > output)
{
	float lens_nucleus = 0.4f;

	float4 vertices[4] =
	{
		float4(0.5f * Size, lens_nucleus, 0.0f, 1.0f),
		float4(-0.5f * Size, lens_nucleus, 0.0f, 1.0f),
		float4(0.5f * Size, 2.0f, 0.0f, 1.0f),
		float4(-0.5f * Size, 2.0f, 0.0f, 1.0f),
	};

	PixelShaderInput vertex;

	float s;
	float c;
	sincos(input[0].Position.x, s, c);

	matrix ModelMatrix =
	{
		c, -s, 0.0f, 0.0f,
		s, c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	matrix ViewProjectionMatrix = mul(ViewMatrix, ProjectionMatrix);

	for (uint i = 0; i < 4; i++)
	{
		vertex.Position = mul(mul(vertices[i], ModelMatrix), ViewProjectionMatrix);

		output.Append(vertex);
	}

	output.RestartStrip();
}