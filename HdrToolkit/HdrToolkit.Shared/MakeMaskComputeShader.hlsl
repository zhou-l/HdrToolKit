cbuffer MaskConstantBuffer : register(b0)
{
	float Radius;
	float Threshold;
}

Texture2D<float> SourceTexture : register(t0);

RWTexture2D<unorm float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	if (SourceTexture[position.xy] >= Threshold)
	{
		int radius = ceil(Radius);

		int2 offset;
		for (offset.y = -radius; offset.y <= radius; offset.y++)
		{
			for (offset.x = -radius; offset.x <= radius; offset.x++)
			{
				if (dot(offset, offset) <= Radius * Radius)
				{
					TargetTexture[position.xy + offset.xy] = 1.0f;
				}
			}
		}
	}
}