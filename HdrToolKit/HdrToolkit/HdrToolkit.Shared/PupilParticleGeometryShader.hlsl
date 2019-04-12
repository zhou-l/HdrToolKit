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
	float2 TexCoord : TEXCOORD;
};

[maxvertexcount(6)]
void main(point GeometryShaderInput input[1], inout TriangleStream< PixelShaderInput > output)
{
	float4 vertices[4] =
	{
		float4(0.5f * Size, -0.5 * Size, 0.0f, 1.0f),
		float4(-0.5f * Size, -0.5 * Size, 0.0f, 1.0f),
		float4(0.5f * Size, 0.5f * Size, 0.0f, 1.0f),
		float4(-0.5f * Size, 0.5f * Size, 0.0f, 1.0f),
	};

	float2 texCoords[4] =
	{
		float2(1.0f, -1.0f),
		float2(-1.0f, -1.0f),
		float2(1.0f, 1.0f),
		float2(-1.0f, 1.0f),
	};


	PixelShaderInput vertex;

	matrix ViewProjectionMatrix = mul(ViewMatrix, ProjectionMatrix);

	for (uint i = 0; i < 4; i++)
	{
		vertex.Position = mul(vertices[i] + input[0].Position.xyzz, ViewProjectionMatrix);
		vertex.TexCoord = texCoords[i];

		output.Append(vertex);
	}

	output.RestartStrip();
}