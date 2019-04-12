#include "pch.h"
#include "ImageIO.h"

#include "DirectXHelper.h"

#include <cwctype>
#include <string>
#include <algorithm>

using namespace HdrToolkit;
using namespace concurrency;

using namespace DX;
using namespace DirectX;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

// See https://en.wikipedia.org/wiki/Radiance_(software)#HDR_image_format
std::vector<String^> const HdrFileExtensions =
{
	".hdr",
	".rgbe",
	".pic",
	".xyze",
};

std::vector<String^> const DdsFileExtensions =
{
	".dds",
};

std::vector<String^> const TgaFileExtensions =
{
	".tga",
};

std::vector<String^> const WicFileExtensions =
{
	".bmp",
	".jpg",
	".jpeg",
	".png",
	".tif",
	".tiff",
	".wdp",
	".hdp",
	".jxr",
};

std::vector<String^> const BmpFileExtensions =
{
	".bmp",
};

std::vector<String^> const JpgFileExtensions =
{
	".jpg",
	".jpeg",
};

std::vector<String^> const PngFileExtensions =
{
	".png",
};

std::vector<String^> const TifFileExtensions =
{
	".tif",
	".tiff",
};

std::vector<String^> const WmpFileExtensions =
{
	".wdp",
	".hdp",
	".jxr",
};

template<typename Container>
bool IsFileExtension(String^ fileType, Container fileExtensions)
{
	for (String^ fileExtension : fileExtensions)
	{
		std::wstring extension = fileExtension->Data();
		std::transform(extension.begin(), extension.end(), extension.begin(), std::towlower);
		if (extension == fileType->Data())
		{
			return true;
		}
	}
	return false;
}

task<HRESULT> HresultTask(HRESULT result)
{
	return create_task([result]() {
		return result;
	});
}

Windows::Foundation::IAsyncOperation<StorageFile^>^ CopyFileToTempFolder(StorageFile^ file)
{
	Windows::Foundation::IAsyncOperation<StorageFile^>^ operation;
	if (file)
	{
		operation = file->CopyAsync(ApplicationData::Current->TemporaryFolder, file->Name, NameCollisionOption::ReplaceExisting);
	}
	else
	{
		operation = create_async([]() -> StorageFile^
		{
			return nullptr;
		});
	}
	return operation;
}

HRESULT LoadImageFromFile(ScratchImage& scratchImage, String^ fileType, const wchar_t* fileName)
{
	if (IsFileExtension(fileType, DdsFileExtensions))
	{
		return LoadFromDDSFile(fileName, DDS_FLAGS_NONE, nullptr, scratchImage);
	}
	else if (IsFileExtension(fileType, TgaFileExtensions))
	{
		return LoadFromTGAFile(fileName, nullptr, scratchImage);
	}
	else if (IsFileExtension(fileType, HdrFileExtensions))
	{
		return LoadFromHDRFile(fileName, nullptr, scratchImage);
	}
	else if (IsFileExtension(fileType, WicFileExtensions))
	{
		return LoadFromWICFile(fileName, WIC_FLAGS_NONE, nullptr, scratchImage);
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT HdrToolkit::LoadImageFromFile(ScratchImage& scratchImage, String^ fileType, String^ file)
{
	return ::LoadImageFromFile(scratchImage, fileType, file->Data());
}

HRESULT OpenImageFromFile(ID3D11Device* device, SourceRenderTexture* renderTexture, ImageDetails* imageDetails, StorageFile^ file)
{
	if (file)
	{
		HRESULT result = S_OK;
		ScratchImage scratchImage;

		result = LoadImageFromFile(scratchImage, file->FileType, file->Path);
		if (FAILED(result))
			return result;

		auto image = scratchImage.GetImages();

		result = renderTexture->Create(device, image->width, image->height, image->format, image->pixels, image->rowPitch);
		if (FAILED(result))
			return result;

		if (imageDetails != nullptr)
		{
			imageDetails->Reset();
			imageDetails->SetImageMetaInformation(file->Name, file->FileType, image->width, image->height, image->format);
		}

		// the file is in the temp folder so it does not matter if it does not really get deleted
		file->DeleteAsync();

		return result;
	}
	else
	{
		return E_ACCESSDENIED;
	}
}

HRESULT SaveImageToFile(Image const* image, String^ fileType, const wchar_t* fileName)
{
	if (IsFileExtension(fileType, DdsFileExtensions))
	{
		return SaveToDDSFile(*image, DDS_FLAGS_NONE, fileName);
	}
	else if (IsFileExtension(fileType, TgaFileExtensions))
	{
		return SaveToTGAFile(*image, fileName);
	}
	else if (IsFileExtension(fileType, HdrFileExtensions))
	{
		return SaveToHDRFile(*image, fileName);
	}
	else if (IsFileExtension(fileType, BmpFileExtensions))
	{
		return SaveToWICFile(*image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_BMP), fileName);
	}
	else if (IsFileExtension(fileType, JpgFileExtensions))
	{
		return SaveToWICFile(*image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_JPEG), fileName);
	}
	else if (IsFileExtension(fileType, PngFileExtensions))
	{
		return SaveToWICFile(*image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_PNG), fileName);
	}
	else if (IsFileExtension(fileType, TifFileExtensions))
	{
		return SaveToWICFile(*image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_TIFF), fileName);
	}
	else if (IsFileExtension(fileType, WmpFileExtensions))
	{
		return SaveToWICFile(*image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_WMP), fileName);
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT HdrToolkit::SaveImageToFile(Image const* image, String^ fileType, String^ file)
{
	return ::SaveImageToFile(image, fileType, file->Data());
}

task<HRESULT> HdrToolkit::PickOpenImage(ID3D11Device* device, SourceRenderTexture* renderTexture, ImageDetails* imageDetails)
{
	auto openPicker = ref new FileOpenPicker();
	openPicker->ViewMode = PickerViewMode::Thumbnail;
	openPicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;
	for (auto extension : HdrFileExtensions)
	{
		openPicker->FileTypeFilter->Append(extension);
	}
	for (auto extension : DdsFileExtensions)
	{
		openPicker->FileTypeFilter->Append(extension);
	}
	for (auto extension : TgaFileExtensions)
	{
		openPicker->FileTypeFilter->Append(extension);
	}
	for (auto extension : WicFileExtensions)
	{
		openPicker->FileTypeFilter->Append(extension);
	}

	return create_task(openPicker->PickSingleFileAsync()).then([=](StorageFile^ file)
	{
		return CopyFileToTempFolder(file);
	}).then([=](StorageFile^ tempFile)
	{
		return OpenImageFromFile(device, renderTexture, imageDetails, tempFile);
	});
}

task<HRESULT> HdrToolkit::OpenImage(ID3D11Device* device, SourceRenderTexture* renderTexture, ImageDetails* imageDetails, IStorageItem^ item)
{
	return create_task([=]()
	{
		return CopyFileToTempFolder(safe_cast<StorageFile^>(item));
	}).then([=](StorageFile^ tempFile)
	{
		return OpenImageFromFile(device, renderTexture, imageDetails, tempFile);
	});
}

task<HRESULT> HdrToolkit::PickSaveImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture)
{
	return PickSaveImage(device, context, texture, nullptr);
}

task<HRESULT> HdrToolkit::PickSaveImage(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, String^ suggestedFileName)
{
	auto savePicker = ref new FileSavePicker();
	Vector<String^>^ bmpExtensions = ref new Vector<String^>{ BmpFileExtensions.begin(), BmpFileExtensions.end() };
	Vector<String^>^ jpgExtensions = ref new Vector<String^>{ JpgFileExtensions.begin(), JpgFileExtensions.end() };
	Vector<String^>^ pngExtensions = ref new Vector<String^>{ PngFileExtensions.begin(), PngFileExtensions.end() };
	Vector<String^>^ tifExtensions = ref new Vector<String^>{ TifFileExtensions.begin(), TifFileExtensions.end() };
	Vector<String^>^ wmpExtensions = ref new Vector<String^>{ WmpFileExtensions.begin(), WmpFileExtensions.end() };
	Vector<String^>^ hdrExtensions = ref new Vector<String^>{ HdrFileExtensions.begin(), HdrFileExtensions.end() };
	savePicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;
	savePicker->FileTypeChoices->Insert("Joint Photographic Experts Group", jpgExtensions);
	savePicker->FileTypeChoices->Insert("Bitmap", bmpExtensions);
	savePicker->FileTypeChoices->Insert("Portable Network Graphic", pngExtensions);
	savePicker->FileTypeChoices->Insert("Tagged Image File Format", tifExtensions);
	savePicker->FileTypeChoices->Insert("Windows Media Photo", wmpExtensions);
	savePicker->FileTypeChoices->Insert("HDR", hdrExtensions);
	savePicker->SuggestedFileName = suggestedFileName ? suggestedFileName : "screenshot";
	return create_task(savePicker->PickSaveFileAsync()).then([=](StorageFile^ file)
	{
		if (file)
		{
			HRESULT result = S_OK;

			auto folder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
			wchar_t fileName[_MAX_PATH];
			wcscpy_s(fileName, folder->Path->Data());
			wcscat_s(fileName, L"\\");
			wcscat_s(fileName, file->Name->Data());

			ScratchImage scratchImage;
			result = CaptureTexture(device, context, texture, scratchImage);
			if (FAILED(result))
				return HresultTask(result);

			result = ::SaveImageToFile(scratchImage.GetImages(), file->FileType, fileName);
			if (FAILED(result))
				return HresultTask(result);

			return create_task(folder->GetFileAsync(file->Name)).then([file](StorageFile^ tempFile)
			{
				return tempFile->MoveAndReplaceAsync(file);
			}).then([]() {
				return S_OK;
			});
		}
		else
		{
			return HresultTask(E_ACCESSDENIED);
		}
	});
}