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
	float factor = 1.0f / (dx * dy);

	float sy = y * dy;
	uint iy1 = (y * inHeight) / outHeight;
	uint iy2 = min(((y + 1) * inHeight) / outHeight, inHeight - 1);

	float sx = x * dx;
	uint ix1 = (x * inWidth) / outWidth;
	uint ix2 = min(((x + 1) * inWidth) / outWidth, inWidth - 1);

	TargetTexture[position.xy] = (((ix1 + 1) - sx) * ((iy1 + 1 - sy)) * SourceTexture[uint2(ix1, iy1)] +
		((ix1 + 1) - sx) * (sy + dy - (iy1 + 1)) * SourceTexture[uint2(ix1, iy2)] +
		(sx + dx - (ix1 + 1)) * ((iy1 + 1 - sy)) * SourceTexture[uint2(ix2, iy1)] +
		(sx + dx - (ix1 + 1)) * (sy + dx - (iy1 + 1)) * SourceTexture[uint2(ix2, iy2)]) *
		factor;
}