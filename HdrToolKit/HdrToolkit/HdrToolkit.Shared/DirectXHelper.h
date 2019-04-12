/// Code partially from
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

#include <ppltasks.h>
#include <fstream>
#include <iterator>

// Helper utilities for DirectX apps.
namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Converts between Color types.
	inline D2D1_COLOR_F ConvertToColorF(Windows::UI::Color color)
	{
		return D2D1::ColorF(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);
	}

	// Converts between Point types.
	inline D2D1_POINT_2F ConvertToPoint2F(Windows::Foundation::Point point)
	{
		return D2D1::Point2F(point.X, point.Y);
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline Windows::Foundation::Point ConvertToScaledPoint(Windows::Foundation::Point point, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return Windows::Foundation::Point(point.X * dpi / dipsPerInch, point.Y * dpi / dipsPerInch);
	}

	// Function that reads from a binary file asynchronously.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	///
	/// Functions below this point are added
	///

	inline std::vector<byte> ReadData(const std::wstring& filename)
	{
		std::ifstream input(filename, std::ios::binary);

		return std::vector<byte>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
	}

	inline HRESULT CreateVertexShader(ID3D11Device* device, ID3D11VertexShader** shader, const std::wstring& filename)
	{
		auto shaderCode = ReadData(filename);

		return device->CreateVertexShader(shaderCode.data(), shaderCode.size(), nullptr, shader);
	}

	inline HRESULT CreateGeometryShader(ID3D11Device* device, ID3D11GeometryShader** shader, const std::wstring& filename)
	{
		auto shaderCode = ReadData(filename);

		return device->CreateGeometryShader(shaderCode.data(), shaderCode.size(), nullptr, shader);
	}

	inline HRESULT CreatePixelShader(ID3D11Device* device, ID3D11PixelShader** shader, const std::wstring& filename)
	{
		auto shaderCode = ReadData(filename);

		return device->CreatePixelShader(shaderCode.data(), shaderCode.size(), nullptr, shader);
	}

	inline HRESULT CreateComputeShader(ID3D11Device* device, ID3D11ComputeShader** shader, const std::wstring& filename)
	{
		auto shaderCode = ReadData(filename);

		return device->CreateComputeShader(shaderCode.data(), shaderCode.size(), nullptr, shader);
	}

	template<typename Shader>
	inline std::enable_if_t<std::is_same<Shader, ID3D11VertexShader>::value> CreateShader(ID3D11Device* device, Shader** shader, const std::wstring& filename)
	{
		ThrowIfFailed(
			CreateVertexShader(device, shader, filename)
		);
	}

	template<typename Shader>
	inline std::enable_if_t<std::is_same<Shader, ID3D11GeometryShader>::value> CreateShader(ID3D11Device* device, Shader** shader, const std::wstring& filename)
	{
		ThrowIfFailed(
			CreateGeometryShader(device, shader, filename)
		);
	}

	template<typename Shader>
	inline std::enable_if_t<std::is_same<Shader, ID3D11PixelShader>::value> CreateShader(ID3D11Device* device, Shader** shader, const std::wstring& filename)
	{
		ThrowIfFailed(
			CreatePixelShader(device, shader, filename)
		);
	}

	template<typename Shader>
	inline std::enable_if_t<std::is_same<Shader, ID3D11ComputeShader>::value> CreateShader(ID3D11Device* device, Shader** shader, const std::wstring& filename)
	{
		ThrowIfFailed(
			CreateComputeShader(device, shader, filename)
		);
	}

	// Platform::String^ from DXGI_FORMAT
	inline Platform::String^ FormatToString(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_UNKNOWN:
			return "DXGI_FORMAT_UNKNOWN";
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return "DXGI_FORMAT_R32G32B32A32_FLOAT";
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return "DXGI_FORMAT_R32G32B32A32_UINT";
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return "DXGI_FORMAT_R32G32B32A32_SINT";
		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return "DXGI_FORMAT_R32G32B32_TYPELESS";
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return "DXGI_FORMAT_R32G32B32_FLOAT";
		case DXGI_FORMAT_R32G32B32_UINT:
			return "DXGI_FORMAT_R32G32B32_UINT";
		case DXGI_FORMAT_R32G32B32_SINT:
			return "DXGI_FORMAT_R32G32B32_SINT";
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return "DXGI_FORMAT_R16G16B16A16_FLOAT";
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return "DXGI_FORMAT_R16G16B16A16_UNORM";
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return "DXGI_FORMAT_R16G16B16A16_UINT";
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return "DXGI_FORMAT_R16G16B16A16_SNORM";
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return "DXGI_FORMAT_R16G16B16A16_SINT";
		case DXGI_FORMAT_R32G32_TYPELESS:
			return "DXGI_FORMAT_R32G32_TYPELESS";
		case DXGI_FORMAT_R32G32_FLOAT:
			return "DXGI_FORMAT_R32G32_FLOAT";
		case DXGI_FORMAT_R32G32_UINT:
			return "DXGI_FORMAT_R32G32_UINT";
		case DXGI_FORMAT_R32G32_SINT:
			return "DXGI_FORMAT_R32G32_SINT";
		case DXGI_FORMAT_R32G8X24_TYPELESS:
			return "DXGI_FORMAT_R32G8X24_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return "DXGI_FORMAT_R10G10B10A2_UNORM";
		case DXGI_FORMAT_R10G10B10A2_UINT:
			return "DXGI_FORMAT_R10G10B10A2_UINT";
		case DXGI_FORMAT_R11G11B10_FLOAT:
			return "DXGI_FORMAT_R11G11B10_FLOAT";
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return "DXGI_FORMAT_R8G8B8A8_UNORM";
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return "DXGI_FORMAT_R8G8B8A8_UINT";
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return "DXGI_FORMAT_R8G8B8A8_SNORM";
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return "DXGI_FORMAT_R8G8B8A8_SINT";
		case DXGI_FORMAT_R16G16_TYPELESS:
			return "DXGI_FORMAT_R16G16_TYPELESS";
		case DXGI_FORMAT_R16G16_FLOAT:
			return "DXGI_FORMAT_R16G16_FLOAT";
		case DXGI_FORMAT_R16G16_UNORM:
			return "DXGI_FORMAT_R16G16_UNORM";
		case DXGI_FORMAT_R16G16_UINT:
			return "DXGI_FORMAT_R16G16_UINT";
		case DXGI_FORMAT_R16G16_SNORM:
			return "DXGI_FORMAT_R16G16_SNORM";
		case DXGI_FORMAT_R16G16_SINT:
			return "DXGI_FORMAT_R16G16_SINT";
		case DXGI_FORMAT_R32_TYPELESS:
			return "DXGI_FORMAT_R32_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT:
			return "DXGI_FORMAT_D32_FLOAT";
		case DXGI_FORMAT_R32_FLOAT:
			return "DXGI_FORMAT_R32_FLOAT";
		case DXGI_FORMAT_R32_UINT:
			return "DXGI_FORMAT_R32_UINT";
		case DXGI_FORMAT_R32_SINT:
			return "DXGI_FORMAT_R32_SINT";
		case DXGI_FORMAT_R24G8_TYPELESS:
			return "DXGI_FORMAT_R24G8_TYPELESS";
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return "DXGI_FORMAT_D24_UNORM_S8_UINT";
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
		case DXGI_FORMAT_R8G8_TYPELESS:
			return "DXGI_FORMAT_R8G8_TYPELESS";
		case DXGI_FORMAT_R8G8_UNORM:
			return "DXGI_FORMAT_R8G8_UNORM";
		case DXGI_FORMAT_R8G8_UINT:
			return "DXGI_FORMAT_R8G8_UINT";
		case DXGI_FORMAT_R8G8_SNORM:
			return "DXGI_FORMAT_R8G8_SNORM";
		case DXGI_FORMAT_R8G8_SINT:
			return "DXGI_FORMAT_R8G8_SINT";
		case DXGI_FORMAT_R16_TYPELESS:
			return "DXGI_FORMAT_R16_TYPELESS";
		case DXGI_FORMAT_R16_FLOAT:
			return "DXGI_FORMAT_R16_FLOAT";
		case DXGI_FORMAT_D16_UNORM:
			return "DXGI_FORMAT_D16_UNORM";
		case DXGI_FORMAT_R16_UNORM:
			return "DXGI_FORMAT_R16_UNORM";
		case DXGI_FORMAT_R16_UINT:
			return "DXGI_FORMAT_R16_UINT";
		case DXGI_FORMAT_R16_SNORM:
			return "DXGI_FORMAT_R16_SNORM";
		case DXGI_FORMAT_R16_SINT:
			return "DXGI_FORMAT_R16_SINT";
		case DXGI_FORMAT_R8_TYPELESS:
			return "DXGI_FORMAT_R8_TYPELESS";
		case DXGI_FORMAT_R8_UNORM:
			return "DXGI_FORMAT_R8_UNORM";
		case DXGI_FORMAT_R8_UINT:
			return "DXGI_FORMAT_R8_UINT";
		case DXGI_FORMAT_R8_SNORM:
			return "DXGI_FORMAT_R8_SNORM";
		case DXGI_FORMAT_R8_SINT:
			return "DXGI_FORMAT_R8_SINT";
		case DXGI_FORMAT_A8_UNORM:
			return "DXGI_FORMAT_A8_UNORM";
		case DXGI_FORMAT_R1_UNORM:
			return "DXGI_FORMAT_R1_UNORM";
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
			return "DXGI_FORMAT_R8G8_B8G8_UNORM";
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			return "DXGI_FORMAT_G8R8_G8B8_UNORM";
		case DXGI_FORMAT_BC1_TYPELESS:
			return "DXGI_FORMAT_BC1_TYPELESS";
		case DXGI_FORMAT_BC1_UNORM:
			return "DXGI_FORMAT_BC1_UNORM";
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return "DXGI_FORMAT_BC1_UNORM_SRGB";
		case DXGI_FORMAT_BC2_TYPELESS:
			return "DXGI_FORMAT_BC2_TYPELESS";
		case DXGI_FORMAT_BC2_UNORM:
			return "DXGI_FORMAT_BC2_UNORM";
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return "DXGI_FORMAT_BC2_UNORM_SRGB";
		case DXGI_FORMAT_BC3_TYPELESS:
			return "DXGI_FORMAT_BC3_TYPELESS";
		case DXGI_FORMAT_BC3_UNORM:
			return "DXGI_FORMAT_BC3_UNORM";
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return "DXGI_FORMAT_BC3_UNORM_SRGB";
		case DXGI_FORMAT_BC4_TYPELESS:
			return "DXGI_FORMAT_BC4_TYPELESS";
		case DXGI_FORMAT_BC4_UNORM:
			return "DXGI_FORMAT_BC4_UNORM";
		case DXGI_FORMAT_BC4_SNORM:
			return "DXGI_FORMAT_BC4_SNORM";
		case DXGI_FORMAT_BC5_TYPELESS:
			return "DXGI_FORMAT_BC5_TYPELESS";
		case DXGI_FORMAT_BC5_UNORM:
			return "DXGI_FORMAT_BC5_UNORM";
		case DXGI_FORMAT_BC5_SNORM:
			return "DXGI_FORMAT_BC5_SNORM";
		case DXGI_FORMAT_B5G6R5_UNORM:
			return "DXGI_FORMAT_B5G6R5_UNORM";
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return "DXGI_FORMAT_B5G5R5A1_UNORM";
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return "DXGI_FORMAT_B8G8R8A8_UNORM";
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return "DXGI_FORMAT_B8G8R8X8_UNORM";
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
		case DXGI_FORMAT_BC6H_TYPELESS:
			return "DXGI_FORMAT_BC6H_TYPELESS";
		case DXGI_FORMAT_BC6H_UF16:
			return "DXGI_FORMAT_BC6H_UF16";
		case DXGI_FORMAT_BC6H_SF16:
			return "DXGI_FORMAT_BC6H_SF16";
		case DXGI_FORMAT_BC7_TYPELESS:
			return "DXGI_FORMAT_BC7_TYPELESS";
		case DXGI_FORMAT_BC7_UNORM:
			return "DXGI_FORMAT_BC7_UNORM";
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return "DXGI_FORMAT_BC7_UNORM_SRGB";
		case DXGI_FORMAT_AYUV:
			return "DXGI_FORMAT_AYUV";
		case DXGI_FORMAT_Y410:
			return "DXGI_FORMAT_Y410";
		case DXGI_FORMAT_Y416:
			return "DXGI_FORMAT_Y416";
		case DXGI_FORMAT_NV12:
			return "DXGI_FORMAT_NV12";
		case DXGI_FORMAT_P010:
			return "DXGI_FORMAT_P010";
		case DXGI_FORMAT_P016:
			return "DXGI_FORMAT_P016";
		case DXGI_FORMAT_420_OPAQUE:
			return "DXGI_FORMAT_420_OPAQUE";
		case DXGI_FORMAT_YUY2:
			return "DXGI_FORMAT_YUY2";
		case DXGI_FORMAT_Y210:
			return "DXGI_FORMAT_Y210";
		case DXGI_FORMAT_Y216:
			return "DXGI_FORMAT_Y216";
		case DXGI_FORMAT_NV11:
			return "DXGI_FORMAT_NV11";
		case DXGI_FORMAT_AI44:
			return "DXGI_FORMAT_AI44";
		case DXGI_FORMAT_IA44:
			return "DXGI_FORMAT_IA44";
		case DXGI_FORMAT_P8:
			return "DXGI_FORMAT_P8";
		case DXGI_FORMAT_A8P8:
			return "DXGI_FORMAT_A8P8";
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return "DXGI_FORMAT_B4G4R4A4_UNORM";
		case DXGI_FORMAT_FORCE_UINT:
			return "DXGI_FORMAT_FORCE_UINT";
		default:
			return "DXGI_FORMAT";
		}
	}

	inline bool IsSingleChannel(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
			return true;
		default:
			return false;
		}
	}

	template<typename T>
	inline T Square(T x)
	{
		return x * x;
	}

	template<typename T>
	inline T Cube(T x)
	{
		return x * x * x;
	}

	template<typename T>
	inline T RoundUpToPowerOfTwo(T in)
	{
		T i = 1;
		for (; i < in; i *= 2);
		return i;
	}

	template<typename T1, typename T2>
	inline T1 Pow(T1 x, T2 n)
	{
		T1 y = 1;
		for (T2 i = 0; i < n; i++)
		{
			y *= x;
		}
		return y;
	}

	template<typename T>
	inline T DivRoundUp(T x, T y)
	{
		return x / y + (x % y != 0 ? 1 : 0);
	}
}
