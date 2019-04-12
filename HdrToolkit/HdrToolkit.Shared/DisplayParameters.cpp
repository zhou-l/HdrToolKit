#include "pch.h"
#include "DisplayParameters.h"

using namespace HdrToolkit;

using namespace Platform;

DisplayOutputView::DisplayOutputView()
{
	Output = DisplayOutput::Output;
}

DisplayOutputView::DisplayOutputView(int outputTexture)
{
	Output = static_cast<DisplayOutput>(outputTexture);
}

DisplayOutputView::DisplayOutputView(DisplayOutput output)
{
	Output = output;
}

String^ DisplayOutputView::Description::get()
{
	static Windows::ApplicationModel::Resources::ResourceLoader^ loader;
	if (loader == nullptr)
	{
		loader = ref new Windows::ApplicationModel::Resources::ResourceLoader();
	}

	switch (Output)
	{
	case DisplayOutput::Input:
		return loader->GetString("DisplayOutputInputString");
	default:
	case DisplayOutput::Output:
		return loader->GetString("DisplayOutputOutputString");
	case DisplayOutput::PostProcessed:
		return loader->GetString("DisplayOutputPostProcessString");
	case DisplayOutput::BlobMask:
		return loader->GetString("DisplayOutputBlobMaskString");
	case DisplayOutput::Overlay:
		return loader->GetString("DisplayOutputOverlayString");
	case DisplayOutput::Aperture:
		return loader->GetString("DisplayOutputApertureString");
	case DisplayOutput::Glare:
		return loader->GetString("DisplayOutputGlareString");
	case DisplayOutput::GlareOverlay:
		return loader->GetString("DisplayOutputGlareOverlayString");
	case DisplayOutput::TonemappedImage:
		return loader->GetString("DisplayOutputTonemappedImageString");
	}
}

int DisplayOutputView::AsInt()
{
	return static_cast<int>(Output);
}