// Current scale space at index 0, previous and next at indices 1 and 2
Texture2D<float> Textures[3] : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	float value = Textures[0][position.xy];

	int3 offset;
	[unroll(3)]
	for (offset.z = 0; offset.z < 3; offset.z++)
	{
		[unroll(3)]
		for (offset.y = -1; offset.y <= 1; offset.y++)
		{
			[unroll(3)]
			for (offset.x = -1; offset.x <= 1; offset.x++)
			{
				if (any(offset != 0))
				{
					if (Textures[offset.z][position.xy + offset.xy] >= value)
					{
						return;
					}
				}
			}
		}
	}

	TargetTexture[position.xy] = value;
}