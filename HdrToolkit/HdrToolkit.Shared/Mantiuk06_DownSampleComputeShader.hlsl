Texture2D<float> SourceTexture : register(t0);

RWTexture2D<float> TargetTexture : register(u0);

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint x = position.x;
	uint y = position.y;

	uint inWidth;
	uint inHeight;
	SourceTexture.GetDimensions(inWidth, inHeight);

	uint outWidth;
	uint outHeight;
	TargetTexture.GetDimensions(outWidth, outHeight);

	float dx = (float)inWidth / ((float)outWidth);
	float dy = (float)inHeight / ((float)outHeight);

	float normalize = 1.0f / (dx * dy);

	uint iy1 = (y * inHeight) / outHeight;
	uint iy2 = ((y + 1) * inHeight) / outHeight;
	float fy1 = (iy1 + 1) - y * dy;
	float fy2 = (y + 1) * dy - iy2;

	uint ix1 = (x * inWidth) / outWidth;
	uint ix2 = ((x + 1) * inWidth) / outWidth;
	float fx1 = (ix1 + 1) - x * dx;
	float fx2 = (x + 1) * dx - ix2;

	float pixVal = 0.0f;
	float factorx, factory;
	for (uint i = iy1; i <= iy2 && i < inHeight; i++)
	{
		if (i == iy1)
			factory = fy1; // We're just getting the bottom edge of this pixel
		else if (i == iy2)
			factory = fy2; // We're just gettting the top edge of this pixel
		else
			factory = 1.0f; // We've got the full height of this pixel
		for (uint j = ix1; j <= ix2 && j < inWidth; j++)
		{
			if (j == ix1)
				factorx = fx1; // We've just got the right edge of this pixel
			else if (j == ix2)
				factorx = fx2; // We've just got the left edge of this pixel
			else
				factorx = 1.0f; // We've got the full width of this pixel

			pixVal += SourceTexture[uint2(j, i)] * factorx * factory;
		}
	}

	TargetTexture[position.xy] = pixVal * normalize;
}