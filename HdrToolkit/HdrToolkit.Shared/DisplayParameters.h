#pragma once

namespace HdrToolkit
{
	enum class DisplayOutput : int
	{
		Input = -1,
		Output = -2,
		PostProcessed = 0,
		BlobMask = 128,
		Overlay = 129,
		Aperture = 130,
		Glare = 131,
		GlareOverlay = 132,
		TonemappedImage = 133,
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class DisplayOutputView sealed
	{
	internal:
		DisplayOutputView(DisplayOutput output);

		property DisplayOutput Output;

	public:
		DisplayOutputView();
		DisplayOutputView(int outputTexture);

		property Platform::String^ Description
		{
			Platform::String^ get();
		}

		int AsInt();
	};
}