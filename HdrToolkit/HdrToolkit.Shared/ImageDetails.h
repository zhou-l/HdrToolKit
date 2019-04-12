#pragma once

#include "Color.h"
#include "RenderToolkit.h"

namespace HdrToolkit
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class ImageDetailsView sealed
	{
	public:
		property Platform::String^ FileName;
		property Platform::String^ FileType;

		property UINT Width;
		property UINT Height;

		property Platform::String^ Format;

		property IColor^ MaxRed;
		property IColor^ MaxGreen;
		property IColor^ MaxBlue;
		property IColor^ MaxLuminance;

		property IColor^ MinRed;
		property IColor^ MinGreen;
		property IColor^ MinBlue;
		property IColor^ MinLuminance;

		property IColor^ AverageColor;
		property IColor^ AverageLuminance;
		property IColor^ LogAverageLuminance;
	};

	struct ImageDetails
	{
	private:
		bool m_isValid;

		Platform::String^ m_fileName;
		Platform::String^ m_fileType;

		UINT m_width;
		UINT m_height;
		DXGI_FORMAT m_format;

		float m_maxRed;
		float m_maxGreen;
		float m_maxBlue;
		float m_maxLuminance;

		float m_minRed;
		float m_minGreen;
		float m_minBlue;
		float m_minLuminance;

		float m_averageRed;
		float m_averageGreen;
		float m_averageBlue;
		float m_averageLuminance;

		float m_logAverageLuminance;

	public:
		void Reset();
		void SetImageMetaInformation(Platform::String^ fileName, Platform::String^ fileType, UINT width, UINT height, DXGI_FORMAT format);
		HRESULT Create(UINT width, UINT height, DirectX::XMFLOAT4 colorSum, DirectX::XMFLOAT4 colorMin, DirectX::XMFLOAT4 colorMax, float luminanceSum, float luminanceMin, float luminanceMax, float logLuminanceSum);

		bool IsValid() const;

		Platform::String^ GetFileName() const;
		Platform::String^ GetFileType() const;

		UINT GetWidth() const;
		UINT GetHeight() const;
		DXGI_FORMAT GetFormat() const;

		float GetMaxRed() const;
		float GetMaxGreen() const;
		float GetMaxBlue() const;
		float GetMaxLuminance() const;

		float GetMinRed() const;
		float GetMinGreen() const;
		float GetMinBlue() const;
		float GetMinLuminance() const;

		float GetAverageRed() const;
		float GetAverageGreen() const;
		float GetAverageBlue() const;
		float GetAverageLuminance() const;

		float GetLogAverageLuminance() const;

		ImageDetailsView^ Get() const;
	};
}