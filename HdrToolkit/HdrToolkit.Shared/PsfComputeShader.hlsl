Texture2D<float3> Texture : register(t0);

Texture2D<float3> RealTexture : register(t1);
Texture2D<float3> ImagTexture : register(t2);

RWTexture2D<float3> TargetTexture : register(u0);

SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer ConstantBuffer : register(b0)
{
	float Wavelength;
	float3 Wavecolor;
}

[numthreads(16, 16, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
	uint2 dimension;
	TargetTexture.GetDimensions(dimension.x, dimension.y);

	/// these are from the glare_demo
	float d = 20.32f;
	float scale = Wavelength * d * 1e-4f;

	float glareScale = 1.5f; // constant used in the glare_demo, a nice part of the glare is visible with not too many halos
	glareScale *= dimension.x / 512.0f; // glare is designed to look good with 512 pixels
	glareScale *= 0.5f; // glareSize was doubled to fit more of the image

	float3 color = 0.0f;

	float2 texCoord = float2(position.xy) / dimension;
	texCoord = fmod(texCoord + 0.5f, 1.0f); // shift because result of FFT has center not in the middle of the image
	texCoord -= 0.5f; // shift middle of image to origin
	if (dot(texCoord, texCoord) <= 0.25f) // keep glare filter circular
	{
		texCoord /= scale * glareScale; // coordinate scale
		texCoord += 0.5f; // inverse shift to middle of screen
		texCoord = fmod(texCoord + 0.5f, 1.0f); // inverse shift because of FFT

		float3 real = RealTexture.SampleLevel(Sampler, texCoord, 0);
		float3 imag = ImagTexture.SampleLevel(Sampler, texCoord, 0);
		color = Wavecolor * (real * real + imag * imag) / (scale * scale);
	
		//// Do gamma correction
		//color = pow(color, 2.2f);
	}
	//float3 val = Texture[position.xy] + color;
	//val = clamp(val, 0, 1);
	TargetTexture[position.xy] = Texture[position.xy] + color;
	//TargetTexture[position.xy] = clamp (Texture[position.xy] + color, 0.0, 1.0);
}