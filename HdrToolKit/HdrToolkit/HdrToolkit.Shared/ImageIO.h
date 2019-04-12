#pragma once

#include "RenderTexture.h"
#include "ImageDetails.h"

namespace HdrToolkit
{
	HRESULT LoadImageFromFile(DirectX::ScratchImage& scratchImage, Platform::String^ fileType, Platform::String^ file);

	concurrency::task<HRESULT> PickOpenImage(ID3D11Device* device, HdrToolkit::SourceRenderTexture* renderTexture, ImageDetails* imageDetails);
	concurrency::task<HRESULT> OpenImage(ID3D11Device* device, HdrToolkit::SourceRenderTexture* renderTexture, ImageDetails* imageDetails, Windows::Storage::IStorageItem^ item);

	HRESULT SaveImageToFile(DirectX::Image const* image, Platform::String^ fileType, Platform::String^ file);

	concurrency::task<HRESULT> PickSaveImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture);
	concurrency::task<HRESULT> PickSaveImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, Platform::String^ suggestedFileName);
}