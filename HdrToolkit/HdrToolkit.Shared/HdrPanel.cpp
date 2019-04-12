/// Class based on
/// XAML SwapChainPanel DirectX interop sample (https://code.msdn.microsoft.com/windowsapps/XAML-SwapChainPanel-00cb688b)
/// with licence
// MICROSOFT LIMITED PUBLIC LICENSE version 1.1
// This license governs use of code marked as “sample?or “example?available on this web site without a license agreement, as provided under the section above titled “NOTICE SPECIFIC TO SOFTWARE AVAILABLE ON THIS WEB SITE.?If you use such code(the “software?, you accept this license.If you do not accept the license, do not use the software.
// 1. Definitions
// The terms “reproduce, ?“reproduction, ?“derivative works, ?and “distribution?have the same meaning here as under U.S.copyright law.
// A “contribution?is the original software, or any additions or changes to the software.
// A “contributor?is any person that distributes its contribution under this license.
// “Licensed patents?are a contributor’s patent claims that read directly on its contribution.
// 2. Grant of Rights
// (A) Copyright Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
// (B)Patent Grant - Subject to the terms of this license, including the license conditions and limitations in section 3, each contributor grants you a non - exclusive, worldwide, royalty - free license under its licensed patents to make, have made, use, sell, offer for sale, import, and / or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.
// 3. Conditions and Limitations
// (A) No Trademark License - This license does not grant you rights to use any contributors?name, logo, or trademarks.
// (B)If you bring a patent claim against any contributor over patents that you claim are infringed by the software, your patent license from such contributor to the software ends automatically.
// (C)If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution notices that are present in the software.
// (D)If you distribute any portion of the software in source code form, you may do so only under this license by including a complete copy of this license with your distribution.If you distribute any portion of the software in compiled or object code form, you may only do so under a license that complies with this license.
// (E)The software is licensed “as - is.?You bear the risk of using it.The contributors give no express warranties, guarantees or conditions.You may have additional consumer rights under your local laws which this license cannot change.To the extent permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular purpose and non - infringement.
// (F)Platform Limitation - The licenses granted in sections 2(A) and 2(B)extend only to the software or derivative works that you create that run directly on a Microsoft Windows operating system product, Microsoft run - time technology(such as the.NET Framework or Silverlight), or Microsoft application platform(such as Microsoft Office or Microsoft Dynamics).

#pragma once
#include "pch.h"
#include "HdrPanel.h"
#include "DirectXHelper.h"

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <math.h>
#include <ppltasks.h>
#include <windows.ui.xaml.media.dxinterop.h>

#include <iomanip>

#include "ImageIO.h"

#include "ConstantBuffers.h"

#include "Mantiuk06.h"
#include "Mai11.h"
#include "ColorConverter.h"

#include "ColorMap.h"
#include <deque>
#include <fstream>
#include <windows.h>
#include "kmeans.h"
#include "dkm.hpp"

// Disable Mantiuk GPU!!!
#define MANTIUK_GPU
#ifndef MANTIUK_GPU
#define MANTIUK_CPU
#endif


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? a : b)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? a : b)
#endif

using namespace Microsoft::WRL;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Shapes;

using namespace Concurrency;
using namespace DirectX;
using namespace D2D1;
using namespace DirectXPanels;
using namespace DX;

using namespace concurrency;
using namespace HdrToolkit;
using namespace std;

static int const NUM_PARTICES = 200;
static const float GLARE_REVEAL_RAD = 0.015f; // Normalized distance for glare revelation

float Random(float min, float max)
{
	return (max - min) * rand() / static_cast<float>(RAND_MAX) + min;
}

std::wstring TimeToString(std::chrono::time_point<std::chrono::system_clock> time)
{
	auto time_t = std::chrono::system_clock::to_time_t(time);
	std::tm local_time;
	localtime_s(&local_time, &time_t);

	std::wstringstream stream;
	stream << std::put_time(&local_time, L"%Y-%m-%d %H:%M:%S");

	return stream.str();
}

void getNonZeroPixles(size_t width, size_t height, std::vector<int>& labelMap, std::vector<std::vector<D2D1_POINT_2F>>& ccPosList )
{
	float invH = 1.0f / float(height);
	float invW = 1.0f / float(width);
	// turn label map into list of pixel positions
	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width; x++)
		{
			size_t i = y * width + x;
			int label = labelMap.at(i);
			if (label > 0)
			{
				if (label > ccPosList.size())
				{
					vector<D2D1_POINT_2F> pixPos;
					ccPosList.push_back(pixPos);
				}
				float fx = float(x + 0.5f) * invW;
				float fy = float(y + 0.5f) * invH;
				ccPosList[label - 1].push_back(Point2F(fx, fy));
			}

		}
	}

}

 void calcConnectedComps(size_t width, size_t height, const std::vector<float>& luminance, std::vector<int>& components, std::vector<std::vector<float>>& indvidualCC)
{
	// An image of visting information
	std::vector<bool> visited(width * height, false);
	components.resize(width * height, 0);
	// vector of individual component
	for (size_t i = 0; i < indvidualCC.size(); i++)
		indvidualCC[i].clear();
	indvidualCC.clear();
	// A stack recording components to be visited
	deque<XMINT2> stack;
	int           compId = 1; // component id

							  // Work on the luminance only
	for (UINT64 y = 0; y < height; y++)
	{
		for (UINT64 x = 0; x < width; x++)
		{
			UINT64 index = y * width + x;
			if (visited.at(index))
				continue;
			else
			{
				if (luminance.at(index) == 0.0f)
					visited[index] = true;
				else
				{
					// check connected component
					stack.push_back(XMINT2(x, y));
					while (!stack.empty())
					{
						XMINT2 pt = stack.back();
						UINT64 ptIdx = UINT64(pt.y) * UINT64(width) + UINT64(pt.x);
						stack.pop_back();
						if (visited[ptIdx])
							continue;
						else
						{
							visited[ptIdx] = true;
							components[ptIdx] = compId; // write component Id
							if (compId - 1>= indvidualCC.size()) // create new image for individual CC
							{
								vector<float> indCompImg(width * height, 0);
								indvidualCC.push_back(indCompImg);
							}
							indvidualCC[compId - 1][ptIdx] = luminance.at(ptIdx); // we set the luminance for compImgs

																			   // Check the four neighbors?
							UINT64 nIdx;
							// top
							if (pt.y + 1 <= height - 1)
							{
								XMINT2 top = XMINT2(pt.x, pt.y + 1);
								nIdx = top.y * width + top.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(top);
								}

							}
							// top left
							if (pt.y + 1 <= height - 1 && pt.x - 1 >= 0)
							{
								XMINT2 topLeft = XMINT2(pt.x - 1, pt.y + 1);
								nIdx = topLeft.y * width + topLeft.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(topLeft);
								}
							}

							// top right
							if (pt.y + 1 <= height - 1 && pt.x + 1 <= width - 1)
							{
								XMINT2 topRight = XMINT2(pt.x + 1, pt.y + 1);
								nIdx = topRight.y * width + topRight.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(topRight);
								}
							}
							// bottom
							if (pt.y - 1 >= 0)
							{
								XMINT2 bottom = XMINT2(pt.x, pt.y - 1);
								nIdx = bottom.y * width + bottom.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(bottom);
								}

							}
							// bottom left
							if (pt.y - 1 >= 0 && pt.x - 1>= 0)
							{
								XMINT2 bottomLeft = XMINT2(pt.x - 1, pt.y - 1);
								nIdx = bottomLeft.y * width + bottomLeft.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(bottomLeft);
								}
							}
							// bottom right
							if (pt.y - 1 >= 0 && pt.x + 1 <= width)
							{
								XMINT2 bottomRight = XMINT2(pt.x + 1, pt.y - 1);
								nIdx = bottomRight.y * width + bottomRight.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(bottomRight);
								}
							}

							// left
							if (pt.x - 1 >= 0)
							{
								XMINT2 left = XMINT2(pt.x - 1, pt.y);
								nIdx = left.y * width + left.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(left);
								}

							}
							// right
							if (pt.x + 1 <= width - 1)
							{
								XMINT2 right = XMINT2(pt.x + 1, pt.y);
								nIdx = right.y * width + right.x;
								if (visited.at(nIdx) != true && luminance.at(nIdx) != 0)
								{
									stack.push_back(right);
								}

							}

						}
					}
					compId++;
				}
			}
		}
	}
}

HdrPanel::HdrPanel()
	: m_renderTarget(m_renderTargetView, m_depthStencilView, m_viewport)
{
	bool initLater = true;

	if (!initLater)
	{
		Log(L"Constructor called. Initializing");
		critical_section::scoped_lock lock(m_criticalSection);
		Init();
	}
	else
	{
		Log(L"Contructor called");
	}

#ifdef _DEBUG
	// Test CC
	vector<float> A = {
		1,1,0,0,0,0,0,
		1,1,0,0,1,1,0,
		1,1,0,0,0,1,0,
		1,1,0,0,0,0,0,
		0,0,0,0,0,1,0,
		0,0,0,0,0,0,0
	};

	vector<int> C;
	vector<vector<float>> indCC;
	size_t w = 7;
	size_t h = 6;
	calcConnectedComps(w, h, A, C, indCC);
	//auto folder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
	//auto installFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	for (size_t i = 0; i < indCC.size(); i++)
	{


		wchar_t fileName[512];
		int cx = swprintf_s(fileName, 512, L"CC%i.txt", int(i));
		String^ sFilename = ref new Platform::String(fileName);
		String^ fullName = installFolder->Path + L"\\" + sFilename;
		ofstream ofCC;
		ofCC.open(fullName->Data());
		if (!ofCC.is_open())
			OutputDebugStringW(L"CC file is not opened.");
		else
		{
			for (size_t y = 0; y < h; y++)
			{
				for (size_t x = 0; x < w; x++)
				{
					if (x < w - 1)
						ofCC << C.at(y * w + x) << "\t";
					else
						ofCC << endl;
				}
			}

		}

		ofCC.close();
	}
#endif

	m_glareLensPixRad = 25; // the radius of lens is 25 pixels
	m_playAnim = false;
	m_dispGlares = true;
}

IAsyncAction^ HdrPanel::InitAsync()
{
	return create_async([=]()
	{
		Log(L"Initializing");
		Init();
	});
}

HdrPanel::~HdrPanel()
{
}

IAsyncOperation<bool>^ HdrPanel::TryPickOpenImage()
{
	return create_async([this]()
	{
		return PickOpenImage(m_d3dDevice.Get(), &m_inputTexture, &m_imageDetails).then([this](HRESULT result)
		{
			ProcessLoadedImage(result);
			return SUCCEEDED(result);
		});
	});
}

IAsyncOperation<bool>^ HdrPanel::TryOpenImage(IStorageItem^ item)
{
	return create_async([this, item]()
	{
		return OpenImage(m_d3dDevice.Get(), &m_inputTexture, &m_imageDetails, item).then([this](HRESULT result)
		{
			ProcessLoadedImage(result);
			return SUCCEEDED(result);
		});
	});
}

void HdrPanel::TryPickSaveImage()
{
	if (!m_textureLoaded)
		return;

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	IRenderTexture* texture;

	String^ filenameAddition;
	switch (static_cast<DisplayOutput>(m_parameters.GetOutputTexture()))
	{
	case DisplayOutput::Input:
		filenameAddition = "";
		texture = m_texture.Get();
		break;
	default:
	case DisplayOutput::Output:
		filenameAddition = "";
		texture = m_outputTexture.Get();
		break;
	case DisplayOutput::BlobMask:
		filenameAddition = "_blob-mask";
		texture = m_blobMask.Get();
		break;
	case DisplayOutput::Overlay:
		filenameAddition = "_bright-pixels";
		texture = m_overlay.Get();
		break;
	case DisplayOutput::Aperture:
		filenameAddition = "_aperture";
		texture = m_apertureTexture.Get();
		break;
	case DisplayOutput::Glare:
		filenameAddition = "_glare";
		texture = m_glareTexture.Get();
		break;
	case DisplayOutput::GlareOverlay:
		filenameAddition = "_glare-overlay";
		m_renderToolkit.CSMultiply(context, m_outputIntermediateTexture.Get(), m_glareOverlay.Get(), m_parameters.GetGlareIntensity());
		texture = m_outputIntermediateTexture.Get();
		break;
	case DisplayOutput::TonemappedImage:
		filenameAddition = "_tone-mapped";
		texture = m_tonemappedImage.Get();
		break;
	}

	if (texture->GetWidth() != m_outputSaveTexture.GetWidth() || texture->GetHeight() != m_outputSaveTexture.GetHeight())
	{
		m_outputSaveTexture.Create(device, texture->GetWidth(), texture->GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	if (!(m_parameters.GetInvertImage() && m_outputTexture.Get() == texture)) // no inversion
	{
		if (m_parameters.GetUndoDefaultGammaCorrection())
		{
			m_renderToolkit.RenderTexture(context, m_outputSaveTexture.Get(), texture);
		}
		else
		{
			m_renderToolkit.GammaCorrectTexture(context, m_outputSaveTexture.Get(), texture, 2.2f, false);
		}
	}
	else
	{
		if (m_parameters.GetUndoDefaultGammaCorrection())
		{
			m_renderToolkit.GammaCorrectTexture(context, m_outputSaveTexture.Get(), texture, Square(2.2f), false);
		}
		else
		{
			m_renderToolkit.GammaCorrectTexture(context, m_outputSaveTexture.Get(), texture, 2.2f, false);
		}
	}

	PickSaveImage(device, context, m_outputSaveTexture.GetTexture(), m_imageDetails.GetFileName() + filenameAddition).then([=](HRESULT result)
	{
		if (result == E_ACCESSDENIED)
		{
			return;
		}
		else
		{
			Log(L"An error occurred while trying to save an image");
		}
	});
}

void HdrPanel::ApplyColormap(IRenderTexture* target, IRenderTexture* source)
{

	/*critical_section::scoped_lock lock(m_criticalSection);*/
	m_criticalSection.lock();
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	RenderToolkitHelper::ClearRenderTarget(context, target->GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, target->GetDepthStencilView());


	RenderTexture imagTexture;
	imagTexture.Create(device, source->GetWidth(), source->GetHeight(), DXGI_FORMAT_R32_FLOAT);

	RenderTexture texColormap;
	auto map = m_parameters.GetMap();
	if (map)
	{
		map->CreateTexture(device, texColormap, m_parameters.GetInvertColormap());
		m_renderToolkit.CSLuminance(context, imagTexture.Get(), source->Get());
		m_renderToolkit.CSColorMap(context, target->Get(), imagTexture.Get(), texColormap.Get());
	}
	else
	{
		m_renderToolkit.CSCopy(context, target->Get(), source->Get());
	}
#ifdef _DEBUG
	// Output the colormapped texture
	ScratchImage scratchImgCM;
	ThrowIfFailed(
		CaptureTexture(
			device,
			context,
			target->GetTexture(),
			scratchImgCM
		)
	);
	auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	wchar_t fileName[512];
	int cx = swprintf_s(fileName, 512, L"colormapped.png");
	String^ sFilename = ref new Platform::String(fileName);
	String^ fullName = installFolder->Path + L"\\" + sFilename;
	cout << fullName->Data() << endl;
	HRESULT result = HdrToolkit::SaveImageToFile(scratchImgCM.GetImages(), L".png", fullName);
#endif
	m_criticalSection.unlock();
}

IAsyncOperationWithProgress<bool, UpdateProgress>^ HdrPanel::Update(ParametersView^ parameters)
{
	m_parameters.Set(parameters->Output);
	m_parametersGlare.Set(parameters->Glare);
	m_parametersBlob.Set(parameters->Blob);
	m_parametersReinhard02.Set(parameters->Reinhard02);
	m_parametersMantiuk06.Set(parameters->Mantiuk06);

	return create_async([this, parameters](progress_reporter<UpdateProgress> reporter)
	{
		UpdateProgress progress;
		progress.Loading = 0.0;
		progress.Tonemapping = 0.0;
		progress.FindingBlobs = 0.0;
		progress.Glaring = 0.0;
		progress.Status = "StatusIdleString";

		reporter.report(progress);

		auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;


		if (!m_loadingComplete || !m_textureLoaded)
		{
			// nothing
			return false;
		}
		else
		{
			auto start = std::chrono::system_clock::now();

			try
			{
				progress.Status = "StatusLoadingString";
				Update_ImageDetails(reporter, progress);

				auto device = m_d3dDevice.Get();
				auto context = m_d3dContext.Get();
				
				// Local TMO -- Mantiuk06
				progress.Status = "StatusTonemappingString";
				auto stTMO = std::chrono::system_clock::now();
				Update_Mantiuk06(reporter, progress, m_tonemappedImage2.Get(), m_texture.Get());
				auto etTMO = std::chrono::system_clock::now();

				// Global TMO -- Mai11
				progress.Status = "StatusTonemappingMai";
				auto stTMO2 = std::chrono::system_clock::now();
				Update_Mai11(reporter, progress, m_tonemappedImage.Get(), m_texture.Get());
				auto etTMO2 = std::chrono::system_clock::now();

				// Always apply colormap

				ApplyColormap(m_colormapped.Get(), m_tonemappedImage.Get());

				progress.Status = "StatusBlobdetectionString";
				auto stBlob = std::chrono::system_clock::now();
				Update_Blob(reporter, progress, m_blobMask.Get(), m_overlay.Get(), m_texture.Get(), m_tonemappedImage.Get());
				auto etBlob = std::chrono::system_clock::now();

				//-----------------------------------------------
				auto stGlare = std::chrono::system_clock::now();
				Update_indvidualGlares(reporter, progress);
				auto etGlare = std::chrono::system_clock::now();
			//--------------------------------------------------
			// original codes for glare overlay
				progress.Status = "StatusGlaringString";
				//auto stGlare = std::chrono::system_clock::now();
				Update_Glare(reporter, progress, m_outputTexture.Get(), m_glareTexture.Get(), m_apertureTexture.Get(), m_glareOverlay.Get(), m_overlay.Get(), m_tonemappedImage.Get());
				////auto etGlare = std::chrono::system_clock::now();
			//---------------------------------------------------

				progress.Status = "StatusDisplayingString";
				progress.Displaying = 50.0;
				reporter.report(progress);
				Render();
				progress.Status = "StatusDoneString";
				progress.Displaying = 100.0;
				reporter.report(progress);

				auto end = std::chrono::system_clock::now();
				std::chrono::duration<double, std::milli> duration = end - start;
				// Detailed durations
				std::chrono::duration<double, std::milli> durationTMO = etTMO - stTMO;
				std::chrono::duration<double, std::milli> durationTMO2 = etTMO2 - stTMO2;
				std::chrono::duration<double, std::milli> durationBlob = etBlob - stBlob;
				std::chrono::duration<double, std::milli> durationGlare = etGlare - stGlare;

				auto changes = m_parameters.GetChanges();
				auto changesGlare = m_parametersGlare.GetChanges();
				auto changesBlob = m_parametersBlob.GetChanges();
				auto changesMantiuk = m_parametersMantiuk06.GetChanges();

				std::wstring const change = L" (changed)";
				std::wstring const nochange = L"";

				auto colormap = m_parameters.GetMap();

				std::wstringstream stream;
				stream << std::setprecision(2) << std::fixed
					<< L"Started computation. Parameters: "
					<< L"Contrast factor = " << m_parametersMantiuk06.GetContrastFactor() << (changesMantiuk.ContrastFactor ? change : nochange) << L", "
					<< L"Wavelengths = " << m_parametersGlare.GetMinWavelength() << L" to " << m_parametersGlare.GetMaxWavelength() << (changesGlare.Wavelengths ? change : nochange) << L", "
					<< L"Blob threshold = " << m_parametersBlob.GetBlobThreshold() << (changesBlob.BlobThreshold ? change : nochange) << L", "
					<< L"Light threshold = " << m_parametersBlob.GetLightThreshold() << (changesBlob.LightThreshold ? change : nochange) << L", "
					<< L"Color map = " << (colormap == nullptr ? L"none" : colormap->GetStringRepresentation()->Data()) << (changes.Map ? change : nochange) << L", "
					<< L"Glare intensity = " << m_parameters.GetGlareIntensity() << (changes.GlareIntensity ? change : nochange) << L". "
					<< L"Computation took " << duration.count() << L" milli seconds."
					<< L"Local TMO took " << durationTMO.count() << L"ms; "
					<< L"Global TMO took " << durationTMO2.count() << L"ms;"
					<< L"Blob detction took " << durationBlob.count() << L"ms;"
					<<L"Glare took "<<durationGlare.count()<<L"ms.";
				Log(start, stream.str());
			}
			catch (...)
			{
				Log(start, L"Started computation. An exception occurred");
			}
			return true;
		}
	});
}

Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ HdrPanel::DispNoGlares(ParametersView ^ parameters)
{
	// TODO: insert return statement here
	return create_async([this, parameters](progress_reporter<UpdateProgress> reporter)
	{
		UpdateProgress progress;
		progress.Loading = 0.0;
		progress.Tonemapping = 0.0;
		progress.FindingBlobs = 0.0;
		progress.Glaring = 0.0;
		progress.Status = "StatusIdleString";

		reporter.report(progress);
		m_dispGlares = false;
		if (!m_loadingComplete || !m_textureLoaded)
		{
			// nothing
			return false;
		}
		else
		{
			try
			{
				Show(m_colormapped.Get());
			}
			catch (...)
			{
				Log(L"Show result with glares. An exception occurred");
			}



			return true;
		}
	});
}

Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ HdrPanel::DispWithGlares(ParametersView ^ parameters)
{
	// TODO: insert return statement here
	return create_async([this, parameters](progress_reporter<UpdateProgress> reporter)
	{
		UpdateProgress progress;
		progress.Loading = 0.0;
		progress.Tonemapping = 0.0;
		progress.FindingBlobs = 0.0;
		progress.Glaring = 0.0;
		progress.Status = "StatusIdleString";

		reporter.report(progress);
		m_dispGlares = true;
		if (!m_loadingComplete || !m_textureLoaded)
		{
			// nothing
			return false;
		}
		else
		{
			try
			{
				Show(m_outputTexture.Get());
			}
			catch (...)
			{
				Log(L"Show result with glares. An exception occurred");
			}



			return true;
		}
	});
}

Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ HdrToolkit::HdrPanel::PlayAnimationHL(ParametersView^ parameters)
{
	return create_async([this, parameters](progress_reporter<UpdateProgress> reporter)
	{
		UpdateProgress progress;
		progress.Loading = 0.0;
		progress.Tonemapping = 0.0;
		progress.FindingBlobs = 0.0;
		progress.Glaring = 0.0;
		progress.Status = "StatusIdleString";

		reporter.report(progress);

		if (!m_loadingComplete || !m_textureLoaded)
		{
			// nothing
			return false;
		}
		else
		{
			

			try
			{
				AnimateGlaresHighlight();
			}
			catch (...)
			{
				Log(L"Started Animation. An exception occurred");
			}



			return true;
		}
	});
}

Windows::Foundation::IAsyncOperationWithProgress<bool, UpdateProgress>^ HdrToolkit::HdrPanel::PlayAnimationFL(ParametersView^ parameters)
{
	return create_async([this, parameters](progress_reporter<UpdateProgress> reporter)
	{
		UpdateProgress progress;
		progress.Loading = 0.0;
		progress.Tonemapping = 0.0;
		progress.FindingBlobs = 0.0;
		progress.Glaring = 0.0;
		progress.Status = "StatusIdleString";

		reporter.report(progress);

		if (!m_loadingComplete || !m_textureLoaded)
		{
			// nothing
			return false;
		}
		else
		{


			try
			{
				AnimateGlaresFlicker();
			}
			catch (...)
			{
				Log(L"Started Animation. An exception occurred");
			}



			return true;
		}
	});
}

void HdrPanel::OnPointerInRange(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e)
{

	// Copy flags to another array
	vector<bool> prevShowGlares(m_showGlares.begin(), m_showGlares.end());
	m_showGlares = vector<bool>(m_brightPixCCList.size(), true);

	e->Handled = true;
	Windows::UI::Input::PointerPoint^ ptrPt = e->GetCurrentPoint(this);
	Windows::UI::Xaml::Controls::TextBlock^ pointerDetails = ref new Windows::UI::Xaml::Controls::TextBlock();
	pointerDetails->Name = ptrPt->PointerId.ToString();
	pointerDetails->Foreground = ref new SolidColorBrush(Windows::UI::Colors::Gray);
	UINT width = m_imageDetails.GetWidth();
	UINT height = m_imageDetails.GetHeight();

	float nptX = (ptrPt->Position.X + 0.5f)/ float(width);
	float nptY = (ptrPt->Position.Y + 0.5f)/ float(height);

	wchar_t wcNormImgPt[512];
	swprintf_s(wcNormImgPt, 512, L"\n nx=%f, ny=%f", nptX, nptY);
	//pointerDetails->Text = ref new String(wcNormImgPt);

	TranslateTransform^ x = ref new TranslateTransform();
	x->X = ptrPt->Position.X;
	x->Y = ptrPt->Position.Y;

	pointerDetails->RenderTransform = x;
	// Check which glares are closest to the pointer

	//float revealRad2 = GLARE_REVEAL_RAD * GLARE_REVEAL_RAD; // normalized radius for revealing 
	float revealRad2 = float(m_glareLensPixRad) * float(m_glareLensPixRad); // squared radius in pixels
	int min_dist_CC = -1;
	float min_dist2 = numeric_limits<float>::max(); // squared distance
	float pix_val_at_minDist = 0.0f; // pixel value at closest glare

	bool revealGlares = false;

	for (size_t i = 0; i < m_brightPixCCList.size(); i++) // Find nearest CC
	{
		vector<D2D1_POINT_2F> cc = m_brightPixCCList.at(i);
		for (size_t cci = 0; cci < cc.size(); cci++)
		{
			D2D1_POINT_2F pt = cc.at(cci);
			float ptX = (ceil(pt.x * float(width) - 0.5f));
			float ptY = (ceil(pt.y * float(height) - 0.5f));

			//float dist2 = (pt.x - nptX) * (pt.x - nptX) + (pt.y - nptY) * (pt.y - nptY);

			float dist2 = (ptX - ptrPt->Position.X)*(ptX - ptrPt->Position.X) + (ptY - ptrPt->Position.Y)*(ptY - ptrPt->Position.Y);
			if (dist2 < revealRad2)
			{
				if (dist2 < min_dist2)
				{
					min_dist2 = dist2;
					min_dist_CC = int(i);
					pix_val_at_minDist = m_brightPixValList.at(i).at(cci);
				}
				m_showGlares[i] = false; // Reveal all glares within radius.
				revealGlares = true;
				break; // Check next CC
			}
		}
	}

	// Update rendering
	bool needUpdate = false;
	for (size_t i = 0; i < m_showGlares.size(); i++)
	{
		if (m_showGlares[i] != prevShowGlares[i]) {
			needUpdate = true;
			break;
		}
	}


	if (needUpdate && m_dispGlares)
		ShowWithGlares(m_showGlares);
	// Draw lens overlay
	if (min_dist_CC >= 0)
	{

		wchar_t wcGlare[512];
		swprintf_s(wcGlare, 512, L"\nGlare %i\nDataVal=%.2f", min_dist_CC, pix_val_at_minDist);
		pointerDetails->Text += ref new String(wcGlare);
	}
	this->Children->Clear();

	if (revealGlares || e->KeyModifiers == Windows::System::VirtualKeyModifiers::Control)
	{
		// Disable the mouse pointer
		Windows::UI::Core::CoreCursor^ currCursor = ref new Windows::UI::Core::CoreCursor(CoreCursorType::Custom, 1234);
		CoreWindow::GetForCurrentThread()->PointerCursor = currCursor;
		// Always show local tone-mapped image regardless if we're around the glare
		if (e->KeyModifiers == Windows::System::VirtualKeyModifiers::Control)
		{
			// Show local tone mapped image within the lens
			GlareLensRender(m_glareLensPixRad, ptrPt->Position.X, ptrPt->Position.Y, m_tonemappedImage2.Get(), false);

		}
		else 
		{
		
			// Otherwise, show only when revealGlares is true
			if (revealGlares)
			{
				if (e->KeyModifiers == Windows::System::VirtualKeyModifiers::Shift)
				{
					// Mode that shows only bright pixels within lens
					GlareLensRender(m_glareLensPixRad, ptrPt->Position.X, ptrPt->Position.Y, m_overlay.Get(), true);
				}
				else
				{	//Draw an ellipse centered at the pointer
					Path^ path1 = ref new Path();
					//path1->Fill = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Azure);
					path1->Stroke = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(180, 190, 190, 190));
					path1->StrokeThickness = 2;
					// Lens circle
					GeometryGroup^ geometryGroup1 = ref new GeometryGroup();
					EllipseGeometry^ ellipseGeometry1 = ref new EllipseGeometry();
					ellipseGeometry1->Center = Point(ptrPt->Position.X, ptrPt->Position.Y);
					ellipseGeometry1->RadiusX = m_glareLensPixRad;
					ellipseGeometry1->RadiusY = m_glareLensPixRad;
					geometryGroup1->Children->Append(ellipseGeometry1);
					path1->Data = geometryGroup1;

					this->Children->Append(path1);
					this->Children->Append(pointerDetails);

					if (!needUpdate)			// Show original 
					{
						critical_section::scoped_lock lock(m_criticalSection);
						if (m_dispGlares)
							Show(m_outputTexture.Get());
						else
							Show(m_colormapped.Get());
					}

				}

			}
		
		}


	}
	else
	{
		Windows::UI::Core::CoreCursor^ currCursor = ref new Windows::UI::Core::CoreCursor(CoreCursorType::Arrow, 0);
		CoreWindow::GetForCurrentThread()->PointerCursor = currCursor;
		if (!needUpdate)			// Show original 
		{
			critical_section::scoped_lock lock(m_criticalSection);
			if (m_dispGlares)
				Show(m_outputTexture.Get());
			else
				Show(m_colormapped.Get());
		}
	}

}

void HdrToolkit::HdrPanel::SetPlayAnim(bool isPlay)
{
	m_playAnim = isPlay;
}

void HdrPanel::SetLensRadius(int Radius)
{
	m_glareLensPixRad = Radius;
}

void HdrPanel::GlareLensRender(float radius, float centerX, float centerY, IRenderTexture* tonemappedImg, bool showBrightPixsOnly)
{
	critical_section::scoped_lock lock(m_criticalSection);
	// Use color-mapped bright pixel texture
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	int src = 0;
	int dst = 1;
	auto map = m_parameters.GetMap();
	if (map)
	{
		// Always apply colormap
		RenderTexture imagTexture;
		imagTexture.Create(device, tonemappedImg->GetWidth(), tonemappedImg->GetHeight(), DXGI_FORMAT_R32_FLOAT);

		RenderTexture texColormap;
		map->CreateTexture(device, texColormap, m_parameters.GetInvertColormap());

		if (showBrightPixsOnly)
			m_renderToolkit.CSLuminance(context, imagTexture.Get(), m_overlay.Get());
		else
			m_renderToolkit.CSLuminance(context, imagTexture.Get(), tonemappedImg->Get());

		m_renderToolkit.CSColorMap(context, m_accumTemp[src].Get(), imagTexture.Get(), texColormap.Get());
	}
	else
	{
		if(showBrightPixsOnly)
			m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_overlay.Get());
		else
			m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), tonemappedImg->Get());
	}

	HRESULT hr = m_renderToolkit.GlareLensTexture(context, m_accumTemp[dst].Get(), 
		/*m_overlay.Get()*/ m_accumTemp[src].Get(), (m_dispGlares)? m_outputTexture.Get() : m_colormapped.Get(), radius, centerX, centerY);


	if (FAILED(hr))
	{
		Log(L"GlareLensRender Failed!");
		return;
	}
	else
		Show(m_accumTemp[dst].Get());
}

void HdrPanel::ShowWithGlares(const std::vector<bool>& showGlareList)
{
	critical_section::scoped_lock lock(m_criticalSection);
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	// Combine glared images
	RenderToolkitHelper::ClearRenderTarget(context, m_outputTexture.GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, m_outputTexture.GetDepthStencilView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[0].GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, m_accumTemp[0].GetDepthStencilView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[1].GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, m_accumTemp[1].GetDepthStencilView());

	int dst = 1;
	int src = 0;
	//m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_colormapped.Get());
	//if (showGlareList.size() > m_glareList.size())
	//{
	//	Log(L"The glare number doesn't match showGlareList size!");
	//	//return;
	//}
	wchar_t fileName[512];
	for (size_t i = 0; i < MIN(m_glareList.size(), showGlareList.size()); i++)
	{
		if (showGlareList[i])
		{

			m_renderToolkit.CSCombineLinear(context, m_accumTemp[dst].Get(),
				m_accumTemp[src].Get(), 1.0f, 
				m_glareList[i].Get(), m_parameters.GetGlareIntensity());
			swap(dst, src);
			m_renderToolkit.CSClamp(context, m_accumTemp[dst].Get(), m_accumTemp[src].Get(), 0.0f, 1.0f);
			swap(dst, src);

	/*		
			int cx = swprintf_s(fileName, 512, L"shownGlare%i.png", int(i));
			String^ sFilename = ref new Platform::String(fileName);

			saveTextureImage(device, context, m_accumTemp[src], sFilename);*/
		}

	}
	//m_renderToolkit.CSClamp(context, m_accumTemp[dst].Get(), m_accumTemp[src].Get(), 0.0f, 1.0f);
	//Platform::String^ filename = ref new Platform::String(L"combinedGlareOverlay.png");
	//saveTextureImage(device, context, m_accumTemp[src], filename);

	m_renderToolkit.CSCombineLinear(context, m_outputTexture.Get(),
		m_accumTemp[src].Get(), 1.0f,
		m_colormapped.Get(), 1.0f);

	//filename = L"outputShowGlares.png";
	//saveTextureImage(device, context, m_outputTexture, filename);
	//filename = L"colormapped.png";
	//saveTextureImage(device, context, m_colormapped, filename);

	//m_renderToolkit.CSClamp(context, m_outputTexture.Get(), m_accumTemp[src].Get(), 0.0f, 1.0f);
	//m_renderToolkit.CSCopy(context, m_outputTexture.Get(), m_accumTemp[src].Get());
	Show(m_outputTexture.Get());
}

ImageDetailsView^ HdrPanel::GetImageDetails()
{
	return m_imageDetails.Get();
}

String^ HdrPanel::GetLog()
{
	return StringReference(m_log.str().data());
}

void HdrPanel::Render()
{
	critical_section::scoped_lock lock(m_criticalSection);

	Show(GetDisplayOutputTexture(), true);
}

IRenderTexture* HdrPanel::GetDisplayOutputTexture()
{
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	auto width = m_imageDetails.GetWidth();
	auto height = m_imageDetails.GetHeight();

	RenderTexture intermediate;

	switch (static_cast<DisplayOutput>(m_parameters.GetOutputTexture()))
	{
	case DisplayOutput::Input:
		return m_texture.Get();
	default:
	case DisplayOutput::Output:
		return m_outputTexture.Get();
	case DisplayOutput::BlobMask:
		return m_blobMask.Get();
	case DisplayOutput::Overlay:
		return m_overlay.Get();
	case DisplayOutput::Aperture:
		intermediate.Create(device, std::min(width, height), std::min(width, height), DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_renderToolkit.RenderTexture(context, intermediate.Get(), m_apertureTexture.Get());
		m_renderToolkit.CSCopy(context, m_outputIntermediateTexture.Get(), intermediate.Get());
		return m_outputIntermediateTexture.Get();
	case DisplayOutput::Glare:
		intermediate.Create(device, std::min(width, height), std::min(width, height), DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_renderToolkit.RenderTexture(context, intermediate.Get(), m_glareTexture.Get());
		m_renderToolkit.CSCopy(context, m_outputIntermediateTexture.Get(), intermediate.Get());
		return m_outputIntermediateTexture.Get();
	case DisplayOutput::GlareOverlay:
		m_renderToolkit.CSMultiply(context, m_outputIntermediateTexture.Get(), m_glareOverlay.Get(), m_parameters.GetGlareIntensity());
		return m_outputIntermediateTexture.Get();
	case DisplayOutput::TonemappedImage:
		return m_tonemappedImage.Get();
	}
}

void HdrPanel::Update_ImageDetails(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress)
{
	critical_section::scoped_lock lock(m_criticalSection);

	progress.Loading = 1.0 / 8.0 * 100.0;
	reporter.report(progress);

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	auto readTextureValue = [=](RenderTexture& texture)
	{
		auto data = m_renderToolkit.CopyToVector(device, context, texture.Get());
		assert(data.size() == 4);
		return XMFLOAT4(data.at(0), data.at(1), data.at(2), data.at(3));
	};

	auto readTextureValueScalar = [=](RenderTexture& texture)
	{
		auto data = m_renderToolkit.CopyToVector(device, context, texture.Get());
		assert(data.size() == 1);
		return data.at(0);
	};

	if (!m_imageDetails.IsValid())
	{
		auto width = m_imageDetails.GetWidth();
		auto height = m_imageDetails.GetHeight();

		m_renderToolkit.RenderTexture(context, m_texture.Get(), m_inputTexture.Get());

		Show(m_texture.Get());

		RenderTexture luminance;
		luminance.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
		RenderTexture intermediate;
		intermediate.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

		auto pyramid = m_renderToolkit.Pyramid(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);

		m_renderToolkit.CSSum(context, pyramid, m_texture.Get());
		auto sum = readTextureValue(pyramid.at(0));
		progress.Loading = 2.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSMin(context, pyramid, m_texture.Get());
		auto min = readTextureValue(pyramid.at(0));
		progress.Loading = 3.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSMax(context, pyramid, m_texture.Get());
		auto max = readTextureValue(pyramid.at(0));
		progress.Loading = 4.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSLuminance(context, luminance.Get(), m_texture.Get());
		pyramid = m_renderToolkit.Pyramid(device, width, height, DXGI_FORMAT_R32_FLOAT);

		Show(luminance.Get());

		m_renderToolkit.CSSum(context, pyramid, luminance.Get());
		auto luminanceSum = readTextureValueScalar(pyramid.at(0));
		progress.Loading = 5.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSMin(context, pyramid, luminance.Get());
		auto luminanceMin = readTextureValueScalar(pyramid.at(0));
		progress.Loading = 6.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSMax(context, pyramid, luminance.Get());
		auto luminanceMax = readTextureValueScalar(pyramid.at(0));
		progress.Loading = 7.0 / 8.0 * 100.0;
		reporter.report(progress);

		float const delta = 1e-5f;
		m_renderToolkit.CSAdd(context, intermediate.Get(), luminance.Get(), delta);
		m_renderToolkit.CSLog(context, luminance.Get(), intermediate.Get());

		m_renderToolkit.CSSum(context, pyramid, luminance.Get());
		auto logLuminanceSum = readTextureValueScalar(pyramid.at(0));

		ThrowIfFailed(
			m_imageDetails.Create(
				width,
				height,
				sum,
				min,
				max,
				luminanceSum,
				luminanceMin,
				luminanceMax,
				logLuminanceSum
			)
		);
	}

	progress.Loading = 100.0;
	reporter.report(progress);
}

/// Partially based on glare_demo (http://people.compute.dtu.dk/jerf/code/glare_demo.zip) of Jeppe Revall Frisvad, DTU Informatics 2009
void HdrPanel::Update_Glare(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* glare, IRenderTexture* aperture, IRenderTexture* glareOverlay, IRenderTexture* overlay, IRenderTexture* source)
{
	critical_section::scoped_lock lock(m_criticalSection);

	reporter.report(progress);

	auto changes = m_parameters.PeekChanges();
	auto changesGlare = m_parametersGlare.PeekChanges();
	auto changesBlob = m_parametersBlob.PeekChanges();
	auto changesMantiuk06 = m_parametersMantiuk06.PeekChanges();

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	auto const width = m_imageDetails.GetWidth();
	auto const height = m_imageDetails.GetHeight();

	auto const glareSize = 2 * std::min(width, height);
	auto const apertureSize = RoundUpToPowerOfTwo(glareSize);
	auto const apertureSizeAA = apertureSize * 2;
	auto const glareSizeAA = RoundUpToPowerOfTwo(apertureSize);
	auto const fftHeight = RoundUpToPowerOfTwo(height + glareSize);
	auto const fftWidth = RoundUpToPowerOfTwo(width + glareSize);
#if defined(_DEBUG)
	OutputDebugStringW(("Dimension: " + width + " x " + height + ", FFT-Dimension: " + fftWidth + " x " + fftHeight + ", Glare-Dimension: " + glareSize + "? AA-Glare-Dimension: " + glareSizeAA + "? Aperture-Dimension: " + apertureSize + "? AA-Aperture-Dimension: " + apertureSizeAA + "?\n")->Data());
#endif

	auto readTextureValue = [=](RenderTexture& texture)
	{
		auto data = m_renderToolkit.CopyToVector(device, context, texture.Get());
		assert(data.size() == 4);
		return XMFLOAT4(data.at(0), data.at(1), data.at(2), data.at(3));
	};

	ID3D11ShaderResourceView* realSRV;
	ID3D11ShaderResourceView* imagSRV;

	RenderTexture realTexture;
	RenderTexture imagTexture;
	
	RenderTexture glareOverlayCopy;

	// TODO: Enable wavelength changes later
	if (!m_glareLoaded || changesGlare.Wavelengths)
	{
		Log( L"Creating glare filter!");
		//wchar_t wmsg[512];
		//swprintf_s(wmsg, 512, L"Creating glare filter as m_glareLoaded = %b and waveLengthsChanged = %b", m_glareLoaded, changesGlare.Wavelengths);
		//Log(wstring(wmsg));
		RenderTexture apertureAA;
		apertureAA.Create(device, apertureSizeAA, apertureSizeAA, DXGI_FORMAT_R32_FLOAT);

		realTexture.Create(device, apertureSize, apertureSize, DXGI_FORMAT_R32G32B32A32_FLOAT);
		imagTexture.Create(device, apertureSize, apertureSize, DXGI_FORMAT_R32G32B32A32_FLOAT);

		std::vector<XMFLOAT2> vertices(NUM_PARTICES);
		for (int i = 0; i < NUM_PARTICES; i++)
		{
			vertices.at(i).x = i * XM_2PI / NUM_PARTICES;
		}
		RenderToolkitHelper::UpdateResource(context, m_pupilVertexBuffer.Get(), vertices.data());

		XMFLOAT4 constant;
		constant.x = 3.0f / 512;
		RenderToolkitHelper::UpdateResource(context, m_pupilConstantBuffer.Get(), &constant);

		auto renderTargetView = apertureAA.GetRenderTargetView();
		auto depthStencilView = apertureAA.GetDepthStencilView();
		auto viewport = apertureAA.GetFullViewport();

		RenderToolkitHelper::ClearRenderTarget(context, renderTargetView, 1.0f, 1.0f, 1.0f, 1.0f, depthStencilView);
		RenderToolkitHelper::SetShaders(context, m_pupilVertexShader.Get(), m_pupilFiberGeometryShader.Get(), m_pupilFiberPixelShader.Get());
		RenderToolkitHelper::SetShaderBuffers(context, {}, { m_matricesConstantBuffer.Get(), m_pupilConstantBuffer.Get() }, {});
		UINT stride = sizeof(XMFLOAT2);
		UINT offset = 0;
		ID3D11Buffer* vertexBuffer = m_pupilVertexBuffer.Get();
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->IASetInputLayout(m_pupilInputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
		context->RSSetViewports(1, &viewport);
		context->Draw(NUM_PARTICES, 0);

		RenderToolkitHelper::SetShaders(context, m_pupilVertexShader.Get(), m_pupilParticleGeometryShader.Get(), m_pupilParticlePixelShader.Get());

		for (int i = 0; i < NUM_PARTICES; i++)
		{
			XMFLOAT2& vertex = vertices.at(i);
			vertex.x = ::Random(-1.0f, 1.0f);
			vertex.y = ::Random(-1.0f, 1.0f);
		}
		RenderToolkitHelper::UpdateResource(context, m_pupilVertexBuffer.Get(), vertices.data());

		constant.x = 12.0f / 512;
		RenderToolkitHelper::UpdateResource(context, m_pupilConstantBuffer.Get(), &constant);
		context->Draw(NUM_PARTICES, 0);

		float const lens_nucleus = 0.4f;

		for (int i = 0; i < NUM_PARTICES; i++)
		{
			XMFLOAT2& vertex = vertices.at(i);
			do
			{
				vertex.x = ::Random(-lens_nucleus, lens_nucleus);
				vertex.y = ::Random(-lens_nucleus, lens_nucleus);
			} while (Square(vertex.x) + Square(vertex.y) > Square(lens_nucleus));
		}
		RenderToolkitHelper::UpdateResource(context, m_pupilVertexBuffer.Get(), vertices.data());

		constant.x = 4.0f / 512;
		RenderToolkitHelper::UpdateResource(context, m_pupilConstantBuffer.Get(), &constant);
		context->Draw(NUM_PARTICES, 0);

		for (int i = 0; i < NUM_PARTICES; i++)
		{
			XMFLOAT2& vertex = vertices.at(i);
			do
			{
				vertex.x = ::Random(-lens_nucleus, lens_nucleus);
				vertex.y = ::Random(-lens_nucleus, lens_nucleus);
			} while (Square(vertex.x) + Square(vertex.y) > Square(lens_nucleus));
		}
		RenderToolkitHelper::UpdateResource(context, m_pupilVertexBuffer.Get(), vertices.data());
		context->Draw(NUM_PARTICES, 0);

		renderTargetView = nullptr;
		depthStencilView = nullptr;
		RenderToolkitHelper::SetShaderBuffers(context, {}, { nullptr, nullptr }, {});
		context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

		auto installFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		ScratchImage eyelashScratchImage;
		LoadImageFromFile(eyelashScratchImage, ".png", installFolder->Path + "\\glare_eyelashes.png");
		auto eyelashImage = eyelashScratchImage.GetImages();
		SourceRenderTexture eyelashTexture;
		eyelashTexture.Create(device, eyelashImage->width, eyelashImage->height, eyelashImage->format, eyelashImage->pixels, eyelashImage->rowPitch);

		m_renderToolkit.RenderTexture(context, realTexture.Get(), eyelashTexture.Get());
		m_renderToolkit.RenderTexture(context, imagTexture.Get(), apertureAA.Get());

		m_renderToolkit.CSMultiply(context, aperture, realTexture.Get(), imagTexture.Get());

		Show(aperture);

		float pupil_size = 9.0f; // Use fixed pupil diameter. This way the glare filter depends not on the actual luminance of the overlay.
		float pupil_radius = pupil_size / 10.0f;
		constant.x = pupil_radius;
		RenderToolkitHelper::UpdateResource(context, m_pupilConstantBuffer.Get(), &constant);

		m_renderToolkit.ComputeTexture(
			context,
			m_eComputeShader.Get(),
			{ realTexture.GetRenderTargetView(), imagTexture.GetRenderTargetView() },
			{ realTexture.GetUnorderedAccessView(), imagTexture.GetUnorderedAccessView() },
			{ m_pupilConstantBuffer.Get() },
			{ aperture->GetShaderResourceView() },
			apertureSize,
			apertureSize
		);

		Show(realTexture.Get());

#if defined(_DEBUG)
		OutputDebugStringW(L"Transforming aperture.\n");
#endif
		m_fft.Resize(glareSizeAA, glareSizeAA);
		m_fft.SetInputSRV(realTexture.GetShaderResourceView(), imagTexture.GetShaderResourceView(), apertureSize, apertureSize);
		m_fft.Transform(FFTPassTransform_Forward);
		m_fft.GetSRV(&realSRV, &imagSRV);
		progress.Glaring = 1.0 / 8.0 * 100.0;
		reporter.report(progress);

#if defined(_DEBUG)
		OutputDebugStringW(L"Creating glare texture.\n");
#endif

		apertureAA.Create(device, glareSizeAA, glareSizeAA, DXGI_FORMAT_R32G32B32A32_FLOAT);
		realTexture.Create(device, glareSizeAA, glareSizeAA, DXGI_FORMAT_R32G32B32A32_FLOAT);
		imagTexture.Create(device, glareSizeAA, glareSizeAA, DXGI_FORMAT_R32G32B32A32_FLOAT);

		RenderTexture glares[3];
		glares[0] = apertureAA;
		glares[1] = realTexture;
		glares[2] = imagTexture;

		bool current = 0;
		bool next = 1;

		int const n = 32; // number of wavelengths sampled

		RenderToolkitHelper::ClearRenderTarget(context, glares[current].GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, glares[current].GetDepthStencilView());
		for (int i = 0; i < n + 1; i++)
		{
			float wavelength = m_parametersGlare.GetMinWavelength() + (i * (m_parametersGlare.GetMaxWavelength() - m_parametersGlare.GetMinWavelength()) / n);
			XMFLOAT3 color = WavelengthToRgb(wavelength);

			constant.x = wavelength;
			constant.y = color.x;
			constant.z = color.y;
			constant.w = color.z;
			RenderToolkitHelper::UpdateResource(context, m_pupilConstantBuffer.Get(), &constant);

			m_renderToolkit.ComputeTexture(
				context,
				m_psfComputeShader.Get(),
				{ glares[next].GetRenderTargetView() },
				{ glares[next].GetUnorderedAccessView() },
				{ m_pupilConstantBuffer.Get() },
				{ glares[current].GetShaderResourceView(), realSRV, imagSRV },
				glareSizeAA,
				glareSizeAA
			);

			current = !current;
			next = !next;

			progress.Glaring = (1.0 + static_cast<double>(i) / n) / 8.0 * 100.0;
			reporter.report(progress);
		}

		auto pyramid = m_renderToolkit.Pyramid(device, glareSizeAA, glareSizeAA, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_renderToolkit.CSSum(context, pyramid, glares[current].Get());

		float normalization = ComputeLuminance(readTextureValue(pyramid.at(0)));

#if defined(_DEBUG)
		OutputDebugStringW(("Sum of glare pixels: " + normalization + "\n")->Data());
#endif

		m_renderToolkit.CSDivide(context, glares[next].Get(), glares[current].Get(), normalization / 1e3f); // normalization is divided by 1e3 => glare is multiplied by 1e3
		m_renderToolkit.CSClamp(context, glares[current].Get(), glares[next].Get(), 0.0f, 1.0f);
		m_renderToolkit.CSShift(context, glares[next].Get(), glares[current].Get(), -static_cast<int>(glareSizeAA / 2), -static_cast<int>(glareSizeAA / 2));

		progress.Glaring = 2.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSBlur(context, glares[current].Get(), glares[2].Get(), glares[next].Get(), 2.0f);
		m_renderToolkit.RenderTexture(context, glare, glares[current].Get());
		Show(glare);
	}
	float gamma = 1.5f;
	glareOverlayCopy.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
	if (changesGlare.HasChanged || changesBlob.HasChanged || changesMantiuk06.HasChanged)
	{
		realTexture.Create(device, fftWidth, fftHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		imagTexture.Create(device, fftWidth, fftHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

#if defined(_DEBUG)
		OutputDebugStringW(L"Transforming glare texture.\n");
#endif
		m_fft.Resize(fftWidth, fftHeight); // The device might be removed here, if the texture size is too large. Probably because of a lack of memory.
		m_fft.SetInputSRV(glare->GetShaderResourceView(), glareSize, glareSize);
		m_fft.Transform(FFTPassTransform_Forward);
		m_fft.GetSRV(&realSRV, &imagSRV);
		progress.Glaring = 3.0 / 8.0 * 100.0;
		reporter.report(progress);

		ID3D11UnorderedAccessView* realUAV;
		ID3D11UnorderedAccessView* imagUAV;

		ID3D11RenderTargetView* realRTV;
		ID3D11RenderTargetView* imagRTV;

		ViewRenderTexture realView(&realSRV, &realRTV, &realUAV, nullptr, fftWidth, fftHeight);
		ViewRenderTexture imagView(&imagSRV, &imagRTV, &imagUAV, nullptr, fftWidth, fftHeight);

		m_renderToolkit.CSCopy(context, realTexture.Get(), realView.Get());
		m_renderToolkit.CSCopy(context, imagTexture.Get(), imagView.Get());
		Show(overlay);

#if defined(_DEBUG)
		OutputDebugStringW(L"Transforming overlay.\n");
#endif
		m_fft.SetInputSRV(overlay->GetShaderResourceView(), width, height);
		m_fft.Transform(FFTPassTransform_Forward);
		m_fft.GetSRV(&realSRV, &imagSRV);
		m_fft.GetUAV(&realUAV, &imagUAV);
		m_fft.GetRTV(&realRTV, &imagRTV);
		progress.Glaring = 4.0 / 8.0 * 100.0;
		reporter.report(progress);

		ID3D11ShaderResourceView* nextRealSRV;
		ID3D11ShaderResourceView* nextImagSRV;

		ID3D11UnorderedAccessView* nextRealUAV;
		ID3D11UnorderedAccessView* nextImagUAV;

		ID3D11RenderTargetView* nextRealRTV;
		ID3D11RenderTargetView* nextImagRTV;

		m_fft.GetNextSRV(&nextRealSRV, &nextImagSRV);
		m_fft.GetNextUAV(&nextRealUAV, &nextImagUAV);
		m_fft.GetNextRTV(&nextRealRTV, &nextImagRTV);

		ViewRenderTexture nextRealView(&nextRealSRV, &nextRealRTV, &nextRealUAV, nullptr, fftWidth, fftHeight);
		ViewRenderTexture nextImagView(&nextImagSRV, &nextImagRTV, &nextImagUAV, nullptr, fftWidth, fftHeight);

		m_renderToolkit.CSCopy(context, nextRealView.Get(), realView.Get());
		m_renderToolkit.CSCopy(context, nextImagView.Get(), imagView.Get());

#if defined(_DEBUG)
		OutputDebugStringW(L"Multiplying transformed images.\n");
#endif
		m_renderToolkit.CSComplexMultiply(context, realView.Get(), imagView.Get(), realTexture.Get(), imagTexture.Get(), nextRealView.Get(), nextImagView.Get());
		progress.Glaring = 5.0 / 8.0 * 100.0;
		reporter.report(progress);

#if defined(_DEBUG)
		OutputDebugStringW(L"Creating glare overlay.\n");
#endif
		m_fft.Transform(FFTPassTransform_Inverse);
		m_fft.GetSRV(&realSRV, nullptr);
		progress.Glaring = 6.0 / 8.0 * 100.0;
		reporter.report(progress);

		m_renderToolkit.CSShift(context, realTexture.Get(), realView.Get(), -static_cast<int>(glareSize / 2), -static_cast<int>(glareSize / 2));

		// ALWAYS do gamma correction to glare overlay
		m_renderToolkit.CSSubtract(context, glareOverlayCopy.Get(), realTexture.Get(), overlay);
		m_renderToolkit.GammaCorrectTexture(context, glareOverlay, glareOverlayCopy.Get(), gamma, true);

		//m_renderToolkit.CSSubtract(context, glareOverlay, realTexture.Get(), overlay);

		//Show(glareOverlay);

		m_fft.Shutdown();
	}

	if (changes.GlareIntensity || changes.Map || changes.InvertColormap || changesGlare.HasChanged || changesBlob.HasChanged || changesMantiuk06.HasChanged)
	{
		if (m_parameters.GetMap() != nullptr)
		{
			//realTexture.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
			//imagTexture.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
			////RenderToolkitHelper::ClearRenderTarget(context, realTexture.GetRenderTargetView(), 1.0f, 1.0f, 1.0f);

			//m_renderToolkit.GammaCorrectTexture(context, imagTexture.Get(), source, gamma, true);
			//m_renderToolkit.CSCombineLinear(context, target, imagTexture.Get(), 1.0f, glareOverlay, -m_parameters.GetGlareIntensity());
			//m_renderToolkit.CSSubtract(context, imagTexture.Get(), realTexture.Get(), target);
			//m_renderToolkit.CSClamp(context, target, imagTexture.Get(), 0.0f, 1.0f);

			realTexture.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
			imagTexture.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			RenderTexture colormap;
			auto map = m_parameters.GetMap();
			map->CreateTexture(device, colormap, m_parameters.GetInvertColormap());
			m_renderToolkit.CSLuminance(context, imagTexture.Get(), m_tonemappedImage.Get()); // Always use tone mapped image for colormapping
			m_renderToolkit.CSColorMap(context, realTexture.Get(), imagTexture.Get(), colormap.Get());

			m_renderToolkit.CSCombineLinear(context, target, realTexture.Get(), 1.0f, glareOverlay, m_parameters.GetGlareIntensity());
		}
		else
		{
			m_renderToolkit.CSCombineLinear(context, target, source, 1.0f, glareOverlay, m_parameters.GetGlareIntensity());
		}

		progress.Glaring = 7.0 / 8.0 * 100.0;
		reporter.report(progress);
	}

	progress.Glaring = 100.0;
	reporter.report(progress);

	m_glareLoaded = true;
}

HRESULT HdrPanel::saveTextureImage(ID3D11Device1* device, ID3D11DeviceContext1* context, HdrToolkit::RenderTexture texture, Platform::String^ sFilename)
{
	ScratchImage scratchImage;
	ThrowIfFailed(
		CaptureTexture(
			device,
			context,
			texture.GetTexture(),
			scratchImage
		)
	);
	auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;


	String^ fullName = installFolder->Path + L"\\" + sFilename;
	cout << fullName->Data() << endl;
	HRESULT result = HdrToolkit::SaveImageToFile(scratchImage.GetImages(), L".png", fullName);

	{
		if (result == E_ACCESSDENIED)
		{
			Log(L"Texture image write access denied!");
		}
		else
		{
			Log(L"An error occurred while trying to save an image");
		}
	};
	if (FAILED(result))
		Log(L"Texture image cannot be written.");
	return result;
}

void HdrPanel::Update_indvidualGlares(concurrency::progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress)
{

	m_criticalSection.lock();
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	ScratchImage scratchImage;
	ScratchImage orgValScratchImg;

	if (m_parametersBlob.PeekChanges().HasChanged || m_parametersGlare.PeekChanges().Wavelengths)
	{
		ThrowIfFailed(
			CaptureTexture(
				device,
				context,
				m_overlay.GetTexture(),
				scratchImage
			)
		);
		ThrowIfFailed(
			CaptureTexture(
				device,
				context,
				m_texture.GetTexture(),
				orgValScratchImg
			)
		);
	}

	if (m_parametersBlob.PeekChanges().HasChanged)
	{
		// Clear CC
		for (size_t i = 0; i < m_connectedCompsImages.size(); i++)
			m_connectedCompsImages[i].clear();
		m_connectedCompsImages.clear();
		//		Flood-fill the blob overlay to get individual blobs*/
	
		// Set pixel to original value
		auto pixels = reinterpret_cast<XMFLOAT4*>(orgValScratchImg.GetImages()->pixels);
		// Get mask pixels
		auto overlayPixels = reinterpret_cast<XMFLOAT4*>(scratchImage.GetImages()->pixels);

		auto width = scratchImage.GetImages()->width;
		auto height = scratchImage.GetImages()->height;

		std::vector<float> red(width * height);
		std::vector<float> green(width * height);
		std::vector<float> blue(width * height);
		std::vector<float> luminance(width * height);

		for (UINT index = 0; index < width * height; index++)
		{
			XMFLOAT4 overlayColor = overlayPixels[index];
			if (overlayColor.x > 0 && overlayColor.y > 0 && overlayColor.z > 0)
			{
				XMFLOAT4 color = pixels[index];

				float r = color.x;
				float g = color.y;
				float b = color.z;
				float l = ComputeLuminance(r, g, b);

				overlayColor.x = r;
				overlayColor.y = g;
				overlayColor.z = b;
				overlayColor.w = l;

				overlayPixels[index] = overlayColor;
			}

		}
		GetConnectedComps(scratchImage, m_connectedCompsImages);
	}
	m_criticalSection.unlock();
	
	if ( m_parametersBlob.PeekChanges().HasChanged || m_parametersGlare.PeekChanges().Wavelengths)
	{
		// Create textures of individual connected components 
		SourceRenderTexture texCC;
		int texDst = 0;
		int texSrc = 1;
		size_t numGlares = MIN(MAX_GLARES, m_connectedCompsImages.size());
		for (size_t i = 0; i < numGlares; i++)
		{

			progress.Status = "StatusGlaringString" + i;
			texCC.Create(device, scratchImage.GetMetadata().width, scratchImage.GetMetadata().height, DXGI_FORMAT_R32G32B32A32_FLOAT, m_connectedCompsImages[i].data(), scratchImage.GetMetadata().width * sizeof(XMFLOAT4));
			// Ping pong result
			if (i == 0)
				Update_Glare(reporter, progress, m_accumTemp[texDst].Get(), m_glareTexture.Get(), m_apertureTexture.Get(), m_glareList[i].Get(), texCC.Get(), m_tonemappedImage.Get());
			else
			{
				m_glareLoaded = true;
				auto changesGlare = m_parametersGlare.PeekChanges(); 
				changesGlare.Wavelengths = false;
				Update_Glare(reporter, progress, m_accumTemp[texDst].Get(), m_glareTexture.Get(), m_apertureTexture.Get(), m_glareList[i].Get(), texCC.Get(), m_accumTemp[texSrc].Get());

			}//#ifdef _DEBUG
			//wchar_t fileName[512];
			//int cx = swprintf_s(fileName, 512, L"glareOverlay%i.png", int(i));
			//String^ sFilename = ref new Platform::String(fileName);

			//saveTextureImage(device, context, m_glareList[i], sFilename);
//#endif
			//Show(m_glareOverlay.Get());
			//Show(m_accumTemp[texDst].Get());
			// Swap dst and src
			swap(texDst, texSrc);
		}
		m_criticalSection.lock();
		// Copy dst texture to outputTexture
		m_renderToolkit.CSCopy(context, m_outputTexture.Get(), m_accumTemp[texSrc].Get());
		Show(m_outputTexture.Get());
		m_criticalSection.unlock();
	}

}

void HdrPanel::Update_Blob(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* blobMask, IRenderTexture* overlay, IRenderTexture* source, IRenderTexture* tonemappedSource)
{
	critical_section::scoped_lock lock(m_criticalSection);

	reporter.report(progress);

	auto changes = m_parametersBlob.PeekChanges();
	auto changesMantiuk = m_parametersMantiuk06.PeekChanges();

	ConstantBuffer_MaskBuffer maskBuffer;
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	auto const width = m_imageDetails.GetWidth();
	auto const height = m_imageDetails.GetHeight();

	if (changes.BlobThreshold)
	{
		if (m_parametersBlob.GetBlobThreshold() == 0.0f)
		{
			RenderToolkitHelper::ClearRenderTarget(context, blobMask->GetRenderTargetView(), 1.0f, 1.0f, 1.0f, 1.0f, blobMask->GetDepthStencilView());
		}
		else
		{
			UINT numScales = 1;
			float const sigma = 1.0f;
			float const scaleMultiplier = sqrt(sqrt(2.0f));
			float const radiusScale = sqrt(2.0f);

			// find blobs with diameter (2*radius) up to 20% of the image size	
			while (2.0f * sigma * pow(scaleMultiplier, numScales - 1) * radiusScale < std::min(width, height) / 5)
			{
				numScales++;
			}

			RenderToolkitHelper::ClearRenderTarget(context, blobMask->GetRenderTargetView(), 0.0f, 0.0f, 0.0f, 1.0f, blobMask->GetDepthStencilView());

			RenderTexture luminance;
			RenderTexture intermediate;
			RenderTexture tmp;
			luminance.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			intermediate.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			tmp.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			auto csScaleSpace = [=, &intermediate, &tmp](IRenderTexture* target, float scaledSigma, IRenderTexture* texture)
			{
				m_renderToolkit.CSBlur(context, tmp.Get(), intermediate.Get(), texture, scaledSigma); // tmp = blur(luminance) with intermediate
				m_renderToolkit.CSLaplace(context, intermediate.Get(), tmp.Get()); // intermediate = laplace(tmp) = laplace(blur(luminance))
				m_renderToolkit.CSMultiply(context, tmp.Get(), intermediate.Get(), Square(scaledSigma)); // tmp = sigma^2 * laplace(blur(luminance))
				m_renderToolkit.CSAbs(context, target, tmp.Get());
			};


			int previous = 0;
			int current = 1;
			int next = 2;
			IRenderTexture* currentTexture;
			IRenderTexture* previousTexture;
			IRenderTexture* nextTexture;

			RenderTexture scaleSpace[3];
			RenderTexture maxScaleSpace;
			scaleSpace[0].Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			scaleSpace[1].Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			scaleSpace[2].Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			maxScaleSpace.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			maskBuffer.Threshold = m_parametersBlob.GetBlobThreshold() * (m_imageDetails.GetMaxLuminance() - m_imageDetails.GetMinLuminance());

			m_renderToolkit.CSLuminance(context, luminance.Get(), m_texture.Get());
			for (UINT i = 0; i < numScales; i++)
			{
				float scaledSigma = static_cast<float>(sigma * pow(scaleMultiplier, i));
				float nextScaledSigma = scaledSigma * scaleMultiplier;

				// set previous and current
				if (i > 0)
				{
					current = (current + 1) % 3;
					previous = (previous + 1) % 3;
					next = (next + 1) % 3;
				}
				else
				{
					csScaleSpace(scaleSpace[current].Get(), scaledSigma, luminance.Get());
					csScaleSpace(scaleSpace[next].Get(), nextScaledSigma, luminance.Get());
				}

				previousTexture = scaleSpace[previous].Get();
				currentTexture = scaleSpace[current].Get();
				nextTexture = scaleSpace[next].Get();

				// set next
				if (i > 0 && i < numScales - 1)
				{
					csScaleSpace(scaleSpace[next].Get(), nextScaledSigma, luminance.Get());
				}
				else if (i == 0)
				{
					previousTexture = nextTexture;
				}
				else
				{
					nextTexture = previousTexture;
				}

				m_renderToolkit.ComputeTexture(
					context,
					m_findExtremaComputeShader.Get(),
					{ maxScaleSpace.GetRenderTargetView() },
					{ maxScaleSpace.GetUnorderedAccessView() },
					{},
					{ currentTexture->GetShaderResourceView(), previousTexture->GetShaderResourceView(), nextTexture->GetShaderResourceView() },
					width,
					height
				);

				maskBuffer.Radius = scaledSigma * radiusScale;
				RenderToolkitHelper::UpdateResource(context, m_blobBuffer.Get(), &maskBuffer);

#if defined(_DEBUG)
				OutputDebugStringW(("Iteration: " + (i + 1) + ", Scaled sigma: " + scaledSigma + ", Blur radius: " + static_cast<int>(2 * ceil(3 * scaledSigma) + 1) + ", Blob radius: " + maskBuffer.Radius + ", Previous == Next: " + (previousTexture == nextTexture) + ", Current != others: " + (currentTexture != nextTexture && currentTexture != previousTexture) + "\n")->Data());
#endif

				m_renderToolkit.ComputeTexture(
					context,
					m_makeMaskComputeShader.Get(),
					{},
					{ blobMask->GetUnorderedAccessView() },
					{ m_blobBuffer.Get() },
					{ maxScaleSpace.GetShaderResourceView() },
					width,
					height
				);

				Show(blobMask);
				progress.FindingBlobs = (i + 1) * 100.0 / (numScales + 1);
				reporter.report(progress);
				}
			}
		}

	if (changes.HasChanged || changesMantiuk.HasChanged)
	{
		maskBuffer.Threshold = m_parametersBlob.GetLightThreshold() * (m_imageDetails.GetMaxLuminance() - m_imageDetails.GetMinLuminance()) + m_imageDetails.GetMinLuminance();
		RenderToolkitHelper::UpdateResource(context, m_blobBuffer.Get(), &maskBuffer);

		m_renderToolkit.ComputeTexture(
			context,
			m_maskComputeShader.Get(),
			{ overlay->GetRenderTargetView() },
			{ overlay->GetUnorderedAccessView() },
			{ m_blobBuffer.Get() },
			{ source->GetShaderResourceView(), tonemappedSource->GetShaderResourceView(), blobMask->GetShaderResourceView() },
			width,
			height
		);
		Show(overlay);
	}

	progress.FindingBlobs = 100.0;
	reporter.report(progress);
	}

void HdrPanel::Update_Reinhard02(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source)
{
	critical_section::scoped_lock lock(m_criticalSection);

	reporter.report(progress);

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	auto const width = m_imageDetails.GetWidth();
	auto const height = m_imageDetails.GetHeight();

	ConstantBuffer_Reinhard02 constantBuffer;
	constantBuffer.Key = 0.18f;
	constantBuffer.Sharpness = 8.0f;
	constantBuffer.MaxLevels = 8;
	constantBuffer.LogAverageLuminance = m_imageDetails.GetLogAverageLuminance();
	constantBuffer.InitialScale = 1.0f;
	constantBuffer.ScaleStepFactor = 1.6f;
	constantBuffer.Threshold = 0.05f;

	ThrowIfFailed(
		RenderToolkitHelper::UpdateResource(context, m_constantBufferReinhard02.Get(), &constantBuffer)
	);

	RenderTexture luminance;
	RenderTexture intermediate;
	luminance.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
	intermediate.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

	ThrowIfFailed(
		m_renderToolkit.CSLuminance(
			context,
			luminance.Get(),
			source
		)
	);

	std::vector<RenderTexture> blurTextures(constantBuffer.MaxLevels);

	std::vector<ID3D11ShaderResourceView*> shaderResourceViews(constantBuffer.MaxLevels + 1);

	for (int i = 0; i < constantBuffer.MaxLevels; i++)
	{
		blurTextures[i].Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

		float scale = static_cast<float>(pow(constantBuffer.ScaleStepFactor, i + 1));
		float radiusInStandardDeviations = 2.0f;

		ThrowIfFailed(
			m_renderToolkit.CSBlur(
				context,
				blurTextures[i].Get(),
				intermediate.Get(),
				luminance.Get(),
				scale * 0.5f / radiusInStandardDeviations,
				radiusInStandardDeviations
			)
		);

		Show(blurTextures[i].Get());
	}

	shaderResourceViews[0] = source->GetShaderResourceView();
	for (int i = 0; i < constantBuffer.MaxLevels; i++)
	{
		shaderResourceViews[i + 1] = blurTextures[i].GetShaderResourceView();
	}

	ThrowIfFailed(
		m_renderToolkit.ComputeTexture(
			context,
			m_reinhardComputeShader.Get(),
			{ target->GetRenderTargetView() },
			{ target->GetUnorderedAccessView() },
			{ m_constantBufferReinhard02.Get() },
			shaderResourceViews,
			width,
			height
		)
	);

	progress.Tonemapping = 100.0;
	reporter.report(progress);
}

/// Code based on
/// Pfstools (http://pfstools.sourceforge.net/index.html)
/// see contrast_domain.cpp
void HdrPanel::Update_Mantiuk06(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source)
{
	critical_section::scoped_lock lock(m_criticalSection);

	reporter.report(progress);

	auto changes = m_parametersMantiuk06.PeekChanges();

	if (changes.HasChanged)
	{
		auto device = m_d3dDevice.Get();
		auto context = m_d3dContext.Get();

		auto const width = m_imageDetails.GetWidth();
		auto const height = m_imageDetails.GetHeight();

#ifdef MANTIUK_GPU
		using Level_t = std::pair<RenderTexture, RenderTexture>;
		using Pyramid_t = std::vector<Level_t>;
		using HalfPyramid_t = std::vector<RenderTexture>;

		auto readTextureValue = [=](RenderTexture& texture)
		{
			auto data = m_renderToolkit.CopyToVector(device, context, texture.Get());
			assert(data.size() == 1);
			return data.at(0);
		};

		auto csDot = [=](std::vector<RenderTexture>& pyramid, IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
		{
			m_renderToolkit.CSMultiply(context, target, firstSource, secondSource);
			m_renderToolkit.CSSum(context, pyramid, target);
			return readTextureValue(pyramid.at(0));
		};

		auto initPyramid = [=](UINT width, UINT height)
		{
			Pyramid_t pyramid;
			while (width >= 3 && height >= 3)
			{
				Level_t level;
				level.first.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
				level.second.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

				pyramid.push_back(level);

				width /= 2;
				height /= 2;
			}

			return pyramid;
		};

		auto initHalfPyramid = [=](UINT width, UINT height)
		{
			HalfPyramid_t pyramid;
			while (width >= 3 && height >= 3)
			{
				RenderTexture level;
				level.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

				pyramid.push_back(level);

				width /= 2;
				height /= 2;
			}

			return pyramid;
		};

		auto csDivergence = [=](IRenderTexture* target, IRenderTexture* firstSource, IRenderTexture* secondSource)
		{
			assert(target->GetWidth() == firstSource->GetWidth());
			assert(firstSource->GetWidth() == secondSource->GetWidth());
			return m_renderToolkit.ComputeTexture(
				context,
				m_divergenceComputeShader.Get(),
				{},
				{ target->GetUnorderedAccessView() },
				{},
				{ firstSource->GetShaderResourceView(), secondSource->GetShaderResourceView() },
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto csDownSample = [=](IRenderTexture* target, IRenderTexture* source)
		{
			assert(target->GetWidth() == source->GetWidth() / 2);
			assert(target->GetHeight() == source->GetHeight() / 2);
			return m_renderToolkit.ComputeTexture(
				context,
				m_downSampleComputeShader.Get(),
				{ target->GetRenderTargetView() },
				{ target->GetUnorderedAccessView() },
				{},
				{ source->GetShaderResourceView() },
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto csUpSample = [=](IRenderTexture* target, IRenderTexture* source)
		{
			assert(target->GetWidth() / 2 == source->GetWidth());
			assert(target->GetHeight() / 2 == source->GetHeight());
			return m_renderToolkit.ComputeTexture(
				context,
				m_upSampleComputeShader.Get(),
				{ target->GetRenderTargetView() },
				{ target->GetUnorderedAccessView() },
				{},
				{ source->GetShaderResourceView() },
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto csGradient = [=](IRenderTexture* firstTarget, IRenderTexture* secondTarget, IRenderTexture* source)
		{
			return m_renderToolkit.ComputeTexture(
				context,
				m_gradientComputeShader.Get(),
				{ firstTarget->GetRenderTargetView(), secondTarget->GetRenderTargetView() },
				{ firstTarget->GetUnorderedAccessView(), secondTarget->GetUnorderedAccessView() },
				{},
				{ source->GetShaderResourceView() },
				firstTarget->GetWidth(),
				firstTarget->GetHeight()
			);
		};

		auto csScaleFactor = [=](IRenderTexture* target, IRenderTexture* source)
		{
			return m_renderToolkit.ComputeTexture(
				context,
				m_scaleFactorComputeShader.Get(),
				{ target->GetRenderTargetView() },
				{ target->GetUnorderedAccessView() },
				{},
				{ source->GetShaderResourceView() },
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto csTransformToR = [=](IRenderTexture* target)
		{
			return m_renderToolkit.ComputeTexture(
				context,
				m_transformToRComputeShader.Get(),
				{},
				{ target->GetUnorderedAccessView() },
				{},
				{},
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto csTransformToG = [=](IRenderTexture* target)
		{
			return m_renderToolkit.ComputeTexture(
				context,
				m_transformToGComputeShader.Get(),
				{},
				{ target->GetUnorderedAccessView() },
				{},
				{},
				target->GetWidth(),
				target->GetHeight()
			);
		};

		auto calculateScalePyramid = [=](Pyramid_t& pyramid)
		{
			Pyramid_t scalePyramid;
			for (Level_t& level : pyramid)
			{
				Level_t scaleLevel;
				scaleLevel.first.Create(device, level.first.GetWidth(), level.first.GetHeight(), DXGI_FORMAT_R32_FLOAT);
				scaleLevel.second.Create(device, level.second.GetWidth(), level.second.GetHeight(), DXGI_FORMAT_R32_FLOAT);

				scalePyramid.push_back(scaleLevel);

				csScaleFactor(scaleLevel.first.Get(), level.first.Get());
				csScaleFactor(scaleLevel.second.Get(), level.second.Get());
			}

			return scalePyramid;
		};

		auto scalePyramid = [=](Pyramid_t& pyramid, Pyramid_t& scale)
		{
			assert(pyramid.size() == scale.size());
			for (UINT i = 0; i < pyramid.size(); i++)
			{
				m_renderToolkit.CSMultiplyEqual(context, pyramid.at(i).first.Get(), scale.at(i).first.Get());
				m_renderToolkit.CSMultiplyEqual(context, pyramid.at(i).second.Get(), scale.at(i).second.Get());
			}
		};

		auto gradientPyramid = [=](Pyramid_t& target, HalfPyramid_t& intermediate, IRenderTexture* source)
		{
			assert(target.size() == intermediate.size());
			for (UINT i = 0; i < target.size(); i++)
			{
				if (i == 0)
				{
					m_renderToolkit.CSCopy(context, intermediate.at(i).Get(), source);
				}
				else
				{
					csDownSample(intermediate.at(i).Get(), intermediate.at(i - 1).Get());
				}

				csGradient(target.at(i).first.Get(), target.at(i).second.Get(), intermediate.at(i).Get());
			}
		};

		auto divergencePyramid = [=](IRenderTexture* target, HalfPyramid_t& intermediate, Pyramid_t& pyramid)
		{
			assert(intermediate.size() == pyramid.size());
			for (int i = intermediate.size() - 1; i >= 0; i--)
			{
				if (i == intermediate.size() - 1)
				{
					RenderToolkitHelper::ClearRenderTarget(context, intermediate.at(i).GetRenderTargetView());
				}
				else
				{
					csUpSample(intermediate.at(i).Get(), intermediate.at(i + 1).Get());
				}

				csDivergence(intermediate.at(i).Get(), pyramid.at(i).first.Get(), pyramid.at(i).second.Get());
			}

			m_renderToolkit.CSCopy(context, target, intermediate.at(0).Get());
		};

		auto transformToR = [=](Pyramid_t& target)
		{
			for (Level_t& level : target)
			{
				csTransformToR(level.first.Get());
				csTransformToR(level.second.Get());
			}
		};

		auto transformToG = [=](Pyramid_t& target)
		{
			for (Level_t& level : target)
			{
				csTransformToG(level.first.Get());
				csTransformToG(level.second.Get());
			}
		};

		auto contrastMapPyramid = [=](Pyramid_t& pyramid, float scale)
		{
			for (Level_t& level : pyramid)
			{
				m_renderToolkit.CSMultiplyEqual(context, level.first.Get(), scale);
				m_renderToolkit.CSMultiplyEqual(context, level.second.Get(), scale);
			}
		};

		auto solveX = [=](IRenderTexture* target, IRenderTexture* source)
		{
			m_renderToolkit.CSMultiply(context, target, source, -0.25f);
		};

		auto multiplyA = [=](IRenderTexture* output, Pyramid_t& target, HalfPyramid_t& intermediate, Pyramid_t& scale, IRenderTexture* source)
		{
			gradientPyramid(target, intermediate, source);
			scalePyramid(target, scale);
			divergencePyramid(output, intermediate, target);
		};

		auto linbcg = [=, &reporter, &progress](HalfPyramid_t& intermediatePyramid, Pyramid_t& pyramid, Pyramid_t& scale, IRenderTexture* x, IRenderTexture* b, const int itmax, const float tol)
		{
			float const tol2 = tol * tol;

			RenderTexture z;
			z.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture zz;
			zz.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture p;
			p.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture pp;
			pp.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture r;
			r.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture rr;
			rr.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
			RenderTexture x_save;
			x_save.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			RenderTexture intermediate;
			intermediate.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			auto csPyramid = m_renderToolkit.Pyramid(device, width, height, DXGI_FORMAT_R32_FLOAT);

			float const bnrm2 = csDot(csPyramid, intermediate.Get(), b, b);

			multiplyA(r.Get(), pyramid, intermediatePyramid, scale, x); // r = A * x
			m_renderToolkit.CSCombineLinearEqual(context, r.Get(), -1.0f, b, 1.0f); // r = b - r
			float err2 = csDot(csPyramid, intermediate.Get(), r.Get(), r.Get()); // err2 = r * r

			multiplyA(rr.Get(), pyramid, intermediatePyramid, scale, r.Get()); // rr = A * r

			float bkden = 0.0f;
			float saved_err2 = err2;
			m_renderToolkit.CSCopy(context, x_save.Get(), x);

			float const ierr2 = err2;

			int iter = 0;
			bool reset = true;
			int num_backwards = 0;
			int const num_backwards_ceiling = 3;

			double const percent_sf = 100.0 / log(tol2 * bnrm2 / ierr2);

			for (; iter < itmax; iter++)
			{
				progress.Tonemapping = log(err2 / ierr2) * percent_sf;
				reporter.report(progress);

				solveX(z.Get(), r.Get()); // z = -0.25 r
				solveX(zz.Get(), rr.Get()); // zz = -0.25 * rr

				float const bknum = csDot(csPyramid, intermediate.Get(), z.Get(), rr.Get());

				if (reset)
				{
					reset = false;
					m_renderToolkit.CSCopy(context, p.Get(), z.Get());
					m_renderToolkit.CSCopy(context, pp.Get(), zz.Get());
				}
				else
				{
					float const bk = bknum / bkden;
					m_renderToolkit.CSCombineLinearEqual(context, p.Get(), bk, z.Get(), 1.0f); // p = z + bk * p
					m_renderToolkit.CSCombineLinearEqual(context, pp.Get(), bk, zz.Get(), 1.0f); // pp = zz + bk * pp
				}

				bkden = bknum;

				multiplyA(z.Get(), pyramid, intermediatePyramid, scale, p.Get()); // z = A * p
				multiplyA(zz.Get(), pyramid, intermediatePyramid, scale, pp.Get()); // zz = A * pp

				float const ak = bknum / csDot(csPyramid, intermediate.Get(), z.Get(), pp.Get());

				m_renderToolkit.CSCombineLinearEqual(context, r.Get(), 1.0f, z.Get(), -ak); // r = r + (-ak) * z
				m_renderToolkit.CSCombineLinearEqual(context, rr.Get(), 1.0f, zz.Get(), -ak); // rr = rr + (-ak) * zz

				float const old_err2 = err2;
				err2 = csDot(csPyramid, intermediate.Get(), r.Get(), r.Get());

				if (err2 > old_err2)
				{
					if (num_backwards == 0 && old_err2 < saved_err2)
					{
						saved_err2 = old_err2;
						m_renderToolkit.CSCopy(context, x_save.Get(), x);
					}
					num_backwards++;
				}
				else
				{
					num_backwards = 0;
				}

				m_renderToolkit.CSCombineLinearEqual(context, x, 1.0f, p.Get(), ak); // x = x + ak * p

				if (num_backwards > num_backwards_ceiling)
				{
					reset = true;
					num_backwards = 0;

					m_renderToolkit.CSCopy(context, x, x_save.Get());
					multiplyA(r.Get(), pyramid, intermediatePyramid, scale, x); // r = A * x

					m_renderToolkit.CSCombineLinearEqual(context, r.Get(), -1.0f, b, 1.0f); // r = b - r

					err2 = csDot(csPyramid, intermediate.Get(), r.Get(), r.Get());
					saved_err2 = err2;

					multiplyA(rr.Get(), pyramid, intermediatePyramid, scale, r.Get()); // rr = A * r
				}

#if defined(_DEBUG)
				OutputDebugStringW(("Iteration: " + (iter + 1) + ", error: " + sqrtf(err2 / bnrm2) + " = sqrt(" + err2 + "/" + bnrm2 + ")\n")->Data());
#endif

				Show(x);

				if (err2 / bnrm2 < tol2)
					break;
			}

			if (err2 > saved_err2)
			{
				err2 = saved_err2;
				m_renderToolkit.CSCopy(context, x, x_save.Get());
			}

			if (err2 / bnrm2 > tol2)
			{
				if (iter == itmax)
					OutputDebugStringW(("\nWarning: Not converged (hit maximum iterations), error = " + sqrtf(err2) + " (should be below " + tol + ").\n")->Data());
				else
					OutputDebugStringW(("\nWarning: Not converged (going unstable), error = " + sqrtf(err2) + " (should be below " + tol + ").\n")->Data());
			}
		};

		auto transformToLuminance = [=](Pyramid_t& pyramid, HalfPyramid_t& intermediatePyramid, IRenderTexture* target, int itmax, float tol)
		{
			auto scale = calculateScalePyramid(pyramid);
			scalePyramid(pyramid, scale);

			RenderTexture b;
			b.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

			divergencePyramid(b.Get(), intermediatePyramid, pyramid);

			linbcg(intermediatePyramid, pyramid, scale, target, b.Get(), itmax, tol);
		};

		RenderTexture luminance;
		luminance.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);
		RenderTexture intermediate;
		intermediate.Create(device, width, height, DXGI_FORMAT_R32_FLOAT);

		RenderTexture rgb;
		rgb.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
		RenderTexture rgbIntermediate;
		rgbIntermediate.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);

		// Compute luminance
		m_renderToolkit.CSLuminance(context, luminance.Get(), source);

		// Clip R,G,B,L values below 1e-7f * maxL
		float const clipValue = 1e-7f * m_imageDetails.GetMaxLuminance();
		m_renderToolkit.CSLowerBound(context, rgbIntermediate.Get(), source, clipValue);
		m_renderToolkit.CSLowerBound(context, intermediate.Get(), luminance.Get(), clipValue);

		// Scale R,G,B by L
		m_renderToolkit.CSDivideScalar(context, rgb.Get(), rgbIntermediate.Get(), intermediate.Get());
		// L = log10(L)
		m_renderToolkit.CSLog10(context, luminance.Get(), intermediate.Get());

		auto intermediatePyramid = initHalfPyramid(width, height);
		auto pyramid = initPyramid(width, height);
		// Calculate gradient
		gradientPyramid(pyramid, intermediatePyramid, luminance.Get());
		// Transform to R
		transformToR(pyramid);

		// Contrast map
		contrastMapPyramid(pyramid, m_parametersMantiuk06.GetContrastFactor());

		// Transform to G
		transformToG(pyramid);
		// Transform to L (solve system of equations)
		int const itmax = 501;
		float const tol = 1e-3f;
		transformToLuminance(pyramid, intermediatePyramid, luminance.Get(), itmax, tol);

		// Renormalize L
		auto temp = m_renderToolkit.CopyToVector(device, context, luminance.Get());
		auto const n = width * height;
		assert(n == temp.size());
		parallel_buffered_sort(temp.begin(), temp.end());
		const float CUT_MARGIN = 0.1f;
		float trim = (n - 1) * CUT_MARGIN * 0.01f;
		float delta = trim - floorf(trim);
		const float l_min = temp[(int)floorf(trim)] * delta + temp[(int)ceilf(trim)] * (1.0f - delta);

		trim = (n - 1) * (100.0f - CUT_MARGIN) * 0.01f;
		delta = trim - floorf(trim);
		const float l_max = temp[(int)floorf(trim)] * delta + temp[(int)ceilf(trim)] * (1.0f - delta);

		const float disp_dyn_range = 2.3f;
		m_renderToolkit.CSAdd(context, intermediate.Get(), luminance.Get(), -l_min);
		m_renderToolkit.CSMultiply(context, luminance.Get(), intermediate.Get(), disp_dyn_range / (l_max - l_min));
		m_renderToolkit.CSAdd(context, intermediate.Get(), luminance.Get(), -disp_dyn_range);

		// Rescale L,R,G,B
		m_renderToolkit.CSExp10(context, luminance.Get(), intermediate.Get());
		m_renderToolkit.CSMultiplyScalar(context, rgbIntermediate.Get(), rgb.Get(), luminance.Get());
		m_renderToolkit.CSClamp(context, target, rgbIntermediate.Get(), 0.0f, 1.0f);
#else
		ScratchImage scratchImage;
		ThrowIfFailed(
			CaptureTexture(
				device,
				context,
				source->GetTexture(),
				scratchImage
			)
		);

		auto image = scratchImage.GetImages();

		assert(width == image->width);
		assert(height == image->height);
		auto pixels = reinterpret_cast<XMFLOAT4*>(image->pixels);

		// Mantiuk 06
		auto outTexture = PfsTmo_Mantiuk06(
			device,
			width,
			height,
			std::vector<XMFLOAT4>(pixels, pixels + width * height),
			m_parametersMantiuk06.GetContrastFactor(),
			1.0f,
			true
		);

		//// Use Mai11

		//auto outTexture = PfsTmo_Mai11(
		//	device,
		//	width,
		//	height,
		//	std::vector<XMFLOAT4>(pixels, pixels + width * height)
		//);

		ThrowIfFailed(
			m_renderToolkit.CSCopy(
				context,
				target,
				outTexture.Get()
			)
		);
#endif
	}

	progress.Tonemapping = 100.0;
	reporter.report(progress);
}

void HdrPanel::Update_Mai11(progress_reporter<UpdateProgress>& reporter, UpdateProgress& progress, IRenderTexture* target, IRenderTexture* source)
{
	//critical_section::scoped_lock lock(m_criticalSection);
	m_criticalSection.lock();
	reporter.report(progress);

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();

	auto const width = m_imageDetails.GetWidth();
	auto const height = m_imageDetails.GetHeight();

	ScratchImage scratchImage;
	ThrowIfFailed(
		CaptureTexture(
			device,
			context,
			source->GetTexture(),
			scratchImage
		)
	);

	auto image = scratchImage.GetImages();

	assert(width == image->width);
	assert(height == image->height);
	auto pixels = reinterpret_cast<XMFLOAT4*>(image->pixels);

	// Mantiuk 06
	/*	auto outTexture = PfsTmo_Mantiuk06(
	device,
	width,
	height,
	std::vector<XMFLOAT4>(pixels, pixels + width * height),
	m_parametersMantiuk06.GetContrastFactor(),
	1.0f,
	true
	);*/

	// Use Mai11

	auto outTexture = PfsTmo_Mai11(
		device,
		width,
		height,
		std::vector<XMFLOAT4>(pixels, pixels + width * height)
	);

	ThrowIfFailed(
		m_renderToolkit.CSCopy(
			context,
			target,
			outTexture.Get()
		)
	);

	progress.Tonemapping = 100.0;
	reporter.report(progress);
	m_criticalSection.unlock();
}


void HdrPanel::CreateDeviceResources()
{
	m_loadingComplete = false;

	DirectXPanelBase::CreateDeviceResources();

	// Retrieve DXGIOutput representing the main adapter output.
	ComPtr<IDXGIFactory1> dxgiFactory;
	ThrowIfFailed(
		CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
	);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	ThrowIfFailed(
		dxgiFactory->EnumAdapters(0, &dxgiAdapter)
	);

	ThrowIfFailed(
		dxgiAdapter->EnumOutputs(0, &m_dxgiOutput)
	);

	std::vector<std::function<void()>> work
	{
		[=]() { m_renderToolkit.Init(m_d3dDevice.Get()); },
		[=]() { Init_Glare(); },
		[=]() { Init_Blob(); },
		[=]() { Init_Reinhard02(); },
		[=]() { Init_Mantiuk06(); },
	};

	parallel_for_each(work.begin(), work.end(), [](std::function<void()>& f) { f(); });

	m_loadingComplete = true;
}

void HdrPanel::CreateSizeDependentResources()
{
	m_renderTargetView = nullptr;
	m_depthStencilView = nullptr;

	DirectXPanelBase::CreateSizeDependentResources();

	// Create a render target view of the swap chain back buffer.
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
	);

	// Create render target view.
	ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView)
	);

	// Create and set viewport.
	m_viewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_renderTargetWidth,
		m_renderTargetHeight
	);

	// Allocate a 2-D surface as the depth/stencil buffer.
	ComPtr<ID3D11Texture2D> depthStencil;
	ThrowIfFailed(
		RenderToolkitHelper::CreateDepthStencil(
			m_d3dDevice.Get(),
			static_cast<UINT>(m_renderTargetWidth),
			static_cast<UINT>(m_renderTargetHeight),
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			depthStencil.ReleaseAndGetAddressOf(),
			m_depthStencilView.ReleaseAndGetAddressOf()
		)
	);
}

void HdrPanel::OnDeviceLost()
{
	m_textureLoaded = false;
	m_glareLoaded = false;

	Log(L"Device removed");

	DirectXPanelBase::OnDeviceLost();
}

void HdrPanel::Log(std::chrono::time_point<std::chrono::system_clock> time, std::wstring message)
{
	m_log << ::TimeToString(time) << L" " << message << "\n";
}

void HdrPanel::Log(std::wstring message)
{
	Log(std::chrono::system_clock::now(), message);
}

void HdrPanel::Init()
{
	try
	{
		CreateDeviceIndependentResources();
		CreateDeviceResources();
		CreateSizeDependentResources();
	}
	catch (...)
	{
		Log(L"An exception occured while initializing");
	}
}

void HdrPanel::Init_Glare()
{
	std::vector<std::function<void()>> work
	{
		// Create vertex shader and input layout for pupil primitives
		[=]()
		{
			auto shaderCode = DX::ReadData(L"PupilVertexShader.cso");

			ThrowIfFailed(
				m_d3dDevice->CreateVertexShader(
					shaderCode.data(),
					shaderCode.size(),
					nullptr,
					m_pupilVertexShader.ReleaseAndGetAddressOf()
				)
			);

			static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			ThrowIfFailed(
				m_d3dDevice->CreateInputLayout(
					vertexDesc,
					ARRAYSIZE(vertexDesc),
					shaderCode.data(),
					shaderCode.size(),
					m_pupilInputLayout.ReleaseAndGetAddressOf()
				)
			);
		},
		// Create shaders for fibers
		[=]() { DX::CreateShader(m_d3dDevice.Get(), m_pupilFiberGeometryShader.ReleaseAndGetAddressOf(), L"PupilFiberGeometryShader.cso"); },
		[=]() { DX::CreateShader(m_d3dDevice.Get(), m_pupilFiberPixelShader.ReleaseAndGetAddressOf(), L"PupilFiberPixelShader.cso"); },
			// Create constant buffers
			[=]()
			{
				RenderToolkitStructures::TextureRenderConstantBuffer constantBuffer;

				XMMATRIX perspectiveMatrix = XMMatrixOrthographicLH(2.0f, 2.0f, 1.0f, 2.0f);
				XMFLOAT4X4 orientation(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				);
				XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);
				XMStoreFloat4x4(
					&constantBuffer.ProjectionMatrix,
					XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
				);

				static const XMVECTORF32 eye = { 0.0f, 0.0f, -1.0f, 0.0f };
				static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
				static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
				XMStoreFloat4x4(
					&constantBuffer.ViewMatrix,
					XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up))
				);

				D3D11_SUBRESOURCE_DATA constantBufferData;
				constantBufferData.pSysMem = &constantBuffer;
				constantBufferData.SysMemPitch = sizeof(RenderToolkitStructures::TextureRenderConstantBuffer);
				constantBufferData.SysMemSlicePitch = sizeof(RenderToolkitStructures::TextureRenderConstantBuffer);

				CD3D11_BUFFER_DESC constantBufferDesc(sizeof(RenderToolkitStructures::TextureRenderConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
				ThrowIfFailed(
					m_d3dDevice->CreateBuffer(
						&constantBufferDesc,
						&constantBufferData,
						m_matricesConstantBuffer.ReleaseAndGetAddressOf()
					)
				);

				constantBufferDesc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
				ThrowIfFailed(
					m_d3dDevice->CreateBuffer(
						&constantBufferDesc,
						nullptr,
						m_pupilConstantBuffer.ReleaseAndGetAddressOf()
					)
				);
			},
			// Create vertex buffer
			[=]()
			{
				CD3D11_BUFFER_DESC vertexBufferDesc(NUM_PARTICES * sizeof(XMFLOAT2), D3D11_BIND_VERTEX_BUFFER);
				ThrowIfFailed(
					m_d3dDevice->CreateBuffer(
						&vertexBufferDesc,
						nullptr,
						m_pupilVertexBuffer.ReleaseAndGetAddressOf()
					)
				);
			},
				// Create shaders for particles
				[=]() { DX::CreateShader(m_d3dDevice.Get(), m_pupilParticleGeometryShader.ReleaseAndGetAddressOf(), L"PupilParticleGeometryShader.cso"); },
				[=]() { DX::CreateShader(m_d3dDevice.Get(), m_pupilParticlePixelShader.ReleaseAndGetAddressOf(), L"PupilParticlePixelShader.cso"); },
				// Create shader for exponential and pupil
				[=]() { DX::CreateShader(m_d3dDevice.Get(), m_eComputeShader.ReleaseAndGetAddressOf(), L"EComputeShader.cso"); },
				// Create shader for PSF
				[=]() { DX::CreateShader(m_d3dDevice.Get(), m_psfComputeShader.ReleaseAndGetAddressOf(), L"PsfComputeShader.cso"); },
				// Create FFT
				[=]() { m_fft.Init(m_d3dDevice.Get(), m_d3dContext.Get(), 512, 512); },
	};

	parallel_for_each(work.begin(), work.end(), [](std::function<void()>& f) { f(); });
}

void HdrPanel::Init_Blob()
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ConstantBuffer_MaskBuffer), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		m_d3dDevice->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_blobBuffer.ReleaseAndGetAddressOf()
		)
	);

	DX::CreateShader(m_d3dDevice.Get(), m_findExtremaComputeShader.ReleaseAndGetAddressOf(), L"FindExtremaComputeShader.cso");
	DX::CreateShader(m_d3dDevice.Get(), m_makeMaskComputeShader.ReleaseAndGetAddressOf(), L"MakeMaskComputeShader.cso");
	DX::CreateShader(m_d3dDevice.Get(), m_maskComputeShader.ReleaseAndGetAddressOf(), L"MaskComputeShader.cso");
	// composition
	//DX::CreateShader(m_d3dDevice.Get(), m_glareLensComputeShader.ReleaseAndGetAddressOf(), L"GlareLensComputeShader.cso");
}

void HdrPanel::Init_Reinhard02()
{
	CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ConstantBuffer_Reinhard02), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(
		m_d3dDevice->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			m_constantBufferReinhard02.ReleaseAndGetAddressOf()
		)
	);

	DX::CreateShader(m_d3dDevice.Get(), m_reinhardComputeShader.ReleaseAndGetAddressOf(), L"Reinhard02ComputeShader.cso");
}

void HdrPanel::Init_Mantiuk06()
{
	std::vector<std::pair<ComPtr<ID3D11ComputeShader>&, std::wstring>> shaders
	{
		{ m_divergenceComputeShader, L"Mantiuk06_DivergenceComputeShader.cso" },
		{ m_downSampleComputeShader, L"Mantiuk06_DownSampleComputeShader.cso" },
		{ m_upSampleComputeShader, L"Mantiuk06_UpSampleComputeShader.cso" },
		{ m_gradientComputeShader, L"Mantiuk06_GradientComputeShader.cso" },
		{ m_scaleFactorComputeShader, L"Mantiuk06_ScaleFactorComputeShader.cso" },
		{ m_transformToRComputeShader, L"Mantiuk06_TransformToRComputeShader.cso" },
		{ m_transformToGComputeShader, L"Mantiuk06_TransformToGComputeShader.cso" },
	};

	parallel_for_each(shaders.begin(), shaders.end(), [=](std::pair<ComPtr<ID3D11ComputeShader>&, std::wstring>& pair)
	{
		DX::CreateShader(m_d3dDevice.Get(), pair.first.ReleaseAndGetAddressOf(), pair.second);
	});
}

void HdrPanel::ProcessLoadedImage(HRESULT imageLoadingResult)
{
	if (imageLoadingResult == E_ACCESSDENIED)
	{
		Log(L"Image loading cancelled");
	}
	else if (SUCCEEDED(imageLoadingResult))
	{
		Log(L"Loaded image \"" + std::wstring((m_imageDetails.GetFileName() + m_imageDetails.GetFileType())->Data()) + L"\"");

		UINT width = m_imageDetails.GetWidth();
		UINT height = m_imageDetails.GetHeight();

		Width = width;
		Height = height;

		CreateTextures(width, height);

		m_parameters.ForceChange();
		m_parametersBlob.ForceChange();
		m_parametersReinhard02.ForceChange();
		m_parametersMantiuk06.ForceChange();
		m_textureLoaded = true;
		m_glareLoaded = false;
	}
	else
	{
		Log(L"An error occurred while trying to load an image");
	}
}

void HdrPanel::CreateTextures(UINT width, UINT height)
{
	auto device = m_d3dDevice.Get();
	auto const glareSize = 2 * std::min(width, height);
	auto const apertureSize = RoundUpToPowerOfTwo(glareSize);

	ThrowIfFailed(
		m_texture.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_outputTexture.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_tonemappedImage.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_tonemappedImage2.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_blobMask.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R8_UNORM,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_overlay.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_glareOverlay.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_apertureTexture.Create(
			device,
			apertureSize,
			apertureSize,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_glareTexture.Create(
			device,
			glareSize,
			glareSize,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);



	ThrowIfFailed(
		m_outputIntermediateTexture.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	ThrowIfFailed(
		m_outputSaveTexture.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);

	// Accumulate multiple glare
	for(int i = 0; i < 2; i++)
		ThrowIfFailed(
			m_accumTemp[i].Create(
				device,
				width,
				height,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_D24_UNORM_S8_UINT
			)
		);

	for (int i = 0; i < MAX_GLARES; i++)
	{
		RenderTexture glareTex;
		ThrowIfFailed(
			glareTex.Create(
				device,
				width,
				height,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_D24_UNORM_S8_UINT
			)
		);
		m_glareList.push_back(glareTex);
	}

	ThrowIfFailed(
		m_colormapped.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);


	ThrowIfFailed(
		m_temp.Create(
			device,
			width,
			height,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		)
	);
}

void HdrToolkit::HdrPanel::GetConnectedComps(ScratchImage & src, std::vector<std::vector<XMFLOAT4>>& dstImgs)
{

	auto image = src.GetImages();
	auto pixels = reinterpret_cast<XMFLOAT4*>(image->pixels);
	auto width = image->width;
	auto height = image->height;

	std::vector<float> red(width * height);
	std::vector<float> green(width * height);
	std::vector<float> blue(width * height);
	std::vector<float> luminance(width * height);

	for (UINT index = 0; index < width * height; index++)
	{
		XMFLOAT4 color = pixels[index];
		float r = color.x;
		float g = color.y;
		float b = color.z;
		float l = ComputeLuminance(r, g, b);
		red.at(index) = r;
		green.at(index) = g;
		blue.at(index) = b;
		luminance.at(index) = l;
	}
	// Run CC method
	std::vector<int>  components;
	std::vector<std::vector<float>> compsImgs;
	calcConnectedComps(width, height, luminance, components, compsImgs);

	// Clear bright pix cc list
	for (size_t i = 0; i < m_brightPixCCList.size(); i++)
		m_brightPixCCList[i].clear();
	m_brightPixCCList.clear();
	for (size_t i = 0; i < m_brightPixValList.size(); i++)
		m_brightPixValList[i].clear();
	m_brightPixValList.clear();
#define _DEBUG
	// Get CC location
	getNonZeroPixles(width, height, components, m_brightPixCCList);
	// Set glare revelation list
	m_showGlares.resize(m_brightPixCCList.size(), true);
#ifdef _DEBUG
	// DEBUG: write to file
	auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	String^ fullName = installFolder->Path + L"\\cclist.csv";
	String^ kmFullName = installFolder->Path + L"\\kmeans.csv";
	ofstream ofCCList(fullName->Data());
	ofstream ofKmList(kmFullName->Data());
	//ofCCList << "num of CompImages = " << compsImgs.size() << "; num of CCs = " << m_brightPixCCList.size() << endl;
	for (size_t i = 0; i < m_brightPixCCList.size(); i++)
	{
		//ofCCList << "CC " << i << ": " << endl;
		vector<D2D1_POINT_2F> cc = m_brightPixCCList.at(i);
		for (size_t jj = 0; jj < cc.size(); jj++)
			//ofCCList << "(" << cc.at(jj).x << "," << cc.at(jj).y << ");";
			ofCCList << cc.at(jj).x << "," << cc.at(jj).y << endl;
		//ofCCList << endl;
	}
#endif
	// Do kmeans on the cclist
	if (m_brightPixCCList.size() > MAX_GLARES)
	{
		size_t num_dataSamples = 0;
		std::vector<std::array<float, 2>> data;

		//Kmeans::DataFrame data;
		for (size_t i = 0; i < m_brightPixCCList.size(); i++)
		{
			for (size_t cci = 0; cci < m_brightPixCCList.at(i).size(); cci++) {
				std::array<float, 2> pt;
				pt[0] = m_brightPixCCList.at(i).at(cci).x;
				pt[1] = m_brightPixCCList.at(i).at(cci).y;
				data.push_back(pt);
			}
		}

		int k = MAX_GLARES;
		auto cluster_data = dkm::kmeans_lloyd(data, k);
		//num_dataSamples = data.size();
		auto assignments = std::get<1>(cluster_data);
		//vector<size_t> assignments;
		//Kmeans::DataFrame means = Kmeans::k_means(data, MAX_GLARES, 300, assignments);

		
		vector<vector<D2D1_POINT_2F>> ccClusterList(MAX_GLARES);
		size_t ccStartId = 0;
		for (size_t i = 0; i < m_brightPixCCList.size(); i++)
		{
			for (size_t jj = 0; jj < m_brightPixCCList.at(i).size(); jj++)
			{
				size_t ind = ccStartId + jj;
				size_t clusterId = assignments[ind];
				ccClusterList[clusterId].push_back(m_brightPixCCList.at(i).at(jj));
#ifdef _DEBUG
				ofKmList << clusterId << endl;
#endif
			}
			ccStartId += m_brightPixCCList.at(i).size();
		}
		// Set m_brightPixCClist
		m_brightPixCCList = ccClusterList;

	}
#ifdef _DEBUG
	ofCCList.close();
	ofKmList.close();
#endif


	// Set individual component image & CC val list
	dstImgs.resize(m_brightPixCCList.size());
	m_brightPixValList.resize(m_brightPixCCList.size());

	for (size_t i = 0; i < dstImgs.size(); i++)
	{
		std::vector<XMFLOAT4> img(width * height);
		m_brightPixValList[i].resize(m_brightPixCCList[i].size());
		for (size_t cci = 0; cci < m_brightPixCCList[i].size(); cci++)
		{
			D2D1_POINT_2F fpt = m_brightPixCCList[i][cci];
			size_t ptX = size_t(ceil(fpt.x * float(width) - 0.5f));
			size_t ptY = size_t(ceil(fpt.y * float(height) - 0.5f));
			size_t idx = ptY * width + ptX;
			float val = luminance.at(idx);

			m_brightPixValList[i][cci] = val;

			// Set actual tex position with tone-mapped value
			img.at(idx) = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
			//img.at(idx) = XMFLOAT4(val, val, val, val);
		}
		dstImgs[i] = img;
	}

}




void HdrPanel::Show(IRenderTexture* texture, bool show)
{
	if (!show)
		return;

	if (m_parameters.GetUndoDefaultGammaCorrection())
	{
		ThrowIfFailed(
			m_renderToolkit.GammaCorrectTexture(
				m_d3dContext.Get(),
				m_renderTarget.Get(),
				texture,
				2.2f,
				!(m_parameters.GetInvertImage() && m_outputTexture.Get() == texture)
			)
		);
	}
	else
	{
		ThrowIfFailed(
			m_renderToolkit.RenderTexture(
				m_d3dContext.Get(),
				m_renderTarget.Get(),
				texture
			)
		);
	}

	Present();
}

void HdrPanel::Show(IRenderTexture* texture)
{
	//m_criticalSection.lock();
	Show(texture, m_parameters.GetShowSteps());
	//m_criticalSection.unlock();
}

void HdrToolkit::HdrPanel::AnimateGlaresHighlight()
{
//	throw gcnew System::NotImplementedException();

	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	// Combine glared images
	RenderToolkitHelper::ClearRenderTarget(context, m_outputTexture.GetRenderTargetView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[0].GetRenderTargetView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[1].GetRenderTargetView());

	int dst = 1;
	int src = 0;
	m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_colormapped.Get());
	
	for (size_t i = 0; i < m_glareList.size(); i++)
	{
		m_renderToolkit.CSCombineLinear(context, m_accumTemp[dst].Get(), m_accumTemp[src].Get(), 1.0f, m_glareList[i].Get(), m_parameters.GetGlareIntensity());

		swap(dst, src);
	}

	m_renderToolkit.CSCopy(context, m_outputTexture.Get(), m_accumTemp[dst].Get());
	//// Output the colormapped texture
	//ScratchImage scratchImgCM;
	//ThrowIfFailed(
	//	CaptureTexture(
	//		device,
	//		context,
	//		m_outputTexture.GetTexture(),
	//		scratchImgCM
	//	)
	//);
	//auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	//wchar_t fileName[512];
	//int cx = swprintf_s(fileName, 512, L"result.png");
	//String^ sFilename = ref new Platform::String(fileName);
	//String^ fullName = installFolder->Path + L"\\" + sFilename;
	//cout << fullName->Data() << endl;
	//HRESULT result = HdrToolkit::SaveImageToFile(scratchImgCM.GetImages(), L".png", fullName);

	Show(m_outputTexture.Get());
	m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_outputTexture.Get());


	int numSteps = 7;
	float invSteps = 1.0f / float(numSteps);
	float duration = 1000.0f / 30.0f; // duration of a frame for 60fps?
	for (size_t i = 0; i < m_glareList.size(); i++)
	{
		//m_renderToolkit.CSCombineLinear(context, m_accumTemp[dst].Get(), m_accumTemp[src].Get(), 1.0f, m_glareList[i].Get(), 0.1f); // Effectively glaring with 120% intensity
		for (int steps = 0; steps < numSteps; steps++)
		{
			wait(duration);
			float a = 0.25f * invSteps * float(steps);
			m_renderToolkit.CSCombineLinear(context, m_temp.Get(), m_accumTemp[src].Get(), 1.0f, m_glareList[i].Get(), a); // Linear interpolate intensity between 0 and 0.1
			Show(m_temp.Get());
		}

		// Copy to dst
		//m_renderToolkit.CSCopy(context, m_accumTemp[dst].Get(), m_temp.Get());
		
		wait(400);// wait for a while before highlighting the next glare
		Show(m_accumTemp[dst].Get());
		swap(dst, src);
	}
	wait(duration);
	Show(m_outputTexture.Get());
}

void HdrToolkit::HdrPanel::AnimateGlaresFlicker()
{
	auto device = m_d3dDevice.Get();
	auto context = m_d3dContext.Get();
	// Combine glared images
	RenderToolkitHelper::ClearRenderTarget(context, m_outputTexture.GetRenderTargetView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[0].GetRenderTargetView());
	RenderToolkitHelper::ClearRenderTarget(context, m_accumTemp[1].GetRenderTargetView());

	int dst = 1;
	int src = 0;
	m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_colormapped.Get());

	for (size_t i = 0; i < m_glareList.size(); i++)
	{
		m_renderToolkit.CSCombineLinear(context, m_accumTemp[dst].Get(), m_accumTemp[src].Get(), 1.0f, m_glareList[i].Get(), m_parameters.GetGlareIntensity());

		swap(dst, src);
	}

	m_renderToolkit.CSCopy(context, m_outputTexture.Get(), m_accumTemp[dst].Get());
	//// Output the colormapped texture
	//ScratchImage scratchImgCM;
	//ThrowIfFailed(
	//	CaptureTexture(
	//		device,
	//		context,
	//		m_outputTexture.GetTexture(),
	//		scratchImgCM
	//	)
	//);
	//auto installFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	//wchar_t fileName[512];
	//int cx = swprintf_s(fileName, 512, L"result.png");
	//String^ sFilename = ref new Platform::String(fileName);
	//String^ fullName = installFolder->Path + L"\\" + sFilename;
	//cout << fullName->Data() << endl;
	//HRESULT result = HdrToolkit::SaveImageToFile(scratchImgCM.GetImages(), L".png", fullName);

	Show(m_outputTexture.Get());
	m_renderToolkit.CSCopy(context, m_accumTemp[src].Get(), m_outputTexture.Get());


	int numSteps = 200;
	float duration = 1000.0f / 60.0f; // duration of a frame for 60 fps (100 fps?)
	int numStepPerFlick = 3;
	float invStepPerFlick = 1.0f / float(numStepPerFlick);

	float minIntensity = 0.0f;
	float maxIntensity = 0.10f;
	//for (int numFlickers = 0; numFlickers < 20; numFlickers++)
	int numFlickers = 0;
	for(;;)
	{
		
		for (int i = 0; i < numStepPerFlick; i++)
		{
			float a = (numFlickers % 2 == 0)? 
				minIntensity + (maxIntensity - minIntensity) * float(i) * invStepPerFlick : // Lighten up
				maxIntensity - (maxIntensity - minIntensity) * float(i) * invStepPerFlick; // darken
			m_renderToolkit.CSCombineLinear(context, m_temp.Get(), m_accumTemp[src].Get(), 1.0f, m_glareOverlay.Get(), a); // Flickering
			Show(m_temp.Get());
			wait(duration);


		}
		numFlickers = (numFlickers + 1) % 2;		
		if (!m_playAnim)
			break;
	}

	Show(m_outputTexture.Get());
}
