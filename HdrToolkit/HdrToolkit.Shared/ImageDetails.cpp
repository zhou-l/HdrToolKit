#include "pch.h"
#include "ImageDetails.h"

#include <functional>

#include <DirectXPackedVector.h>

#include <DirectXMath.h>

#include "DirectXHelper.h"
#include "ColorConverter.h"

using namespace std;
using namespace HdrToolkit;
using namespace DX;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Platform;

String^ RemoveExtension(String^ file)
{
	std::wstring fileString = file->Data();

	return ref new String(fileString.substr(0, fileString.find_last_of(L".")).data());
}

void ImageDetails::Reset()
{
	m_isValid = false;
}

void ImageDetails::SetImageMetaInformation(String^ fileName, String^ fileType, UINT width, UINT height, DXGI_FORMAT format)
{
	m_fileName = RemoveExtension(fileName);
	m_fileType = fileType;

	m_width = width;
	m_height = height;
	m_format = format;
}

HRESULT ImageDetails::Create(UINT width, UINT height, XMFLOAT4 colorSum, XMFLOAT4 colorMin, XMFLOAT4 colorMax, float luminanceSum, float luminanceMin, float luminanceMax, float logLuminanceSum)
{
	if (width != m_width || height != m_height)
		return E_INVALIDARG;
	if (m_isValid)
		return E_FAIL;

	auto pixelCount = width * height;

	m_averageRed = colorSum.x / pixelCount;
	m_averageGreen = colorSum.y / pixelCount;
	m_averageBlue = colorSum.z / pixelCount;

	m_minRed = colorMin.x;
	m_minGreen = colorMin.y;
	m_minBlue = colorMin.z;

	m_maxRed = colorMax.x;
	m_maxGreen = colorMax.y;
	m_maxBlue = colorMax.z;

	m_averageLuminance = luminanceSum / pixelCount;
	m_minLuminance = luminanceMin;
	m_maxLuminance = luminanceMax;
	m_logAverageLuminance = exp(logLuminanceSum / pixelCount);

	m_isValid = true;
	return S_OK;
}

bool ImageDetails::IsValid() const
{
	return m_isValid;
}

String^ ImageDetails::GetFileName() const
{
	return m_fileName;
}
String^ ImageDetails::GetFileType() const
{
	return m_fileType;
}

UINT ImageDetails::GetWidth() const
{
	return m_width;
}
UINT ImageDetails::GetHeight() const
{
	return m_height;
}
DXGI_FORMAT ImageDetails::GetFormat() const
{
	return m_format;
}

float ImageDetails::GetMaxRed() const
{
	return m_maxRed;
}

float ImageDetails::GetMaxGreen() const
{
	return m_maxGreen;
}

float ImageDetails::GetMaxBlue() const
{
	return m_maxBlue;
}

float ImageDetails::GetMaxLuminance() const
{
	return m_maxLuminance;
}

float ImageDetails::GetMinRed() const
{
	return m_minRed;
}

float ImageDetails::GetMinGreen() const
{
	return m_minGreen;
}

float ImageDetails::GetMinBlue() const
{
	return m_minBlue;
}

float ImageDetails::GetMinLuminance() const
{
	return m_minLuminance;
}

float ImageDetails::GetAverageRed() const
{
	return m_averageRed;
}

float ImageDetails::GetAverageGreen() const
{
	return m_averageGreen;
}

float ImageDetails::GetAverageBlue() const
{
	return m_averageBlue;
}

float ImageDetails::GetAverageLuminance() const
{
	return m_averageLuminance;
}

float ImageDetails::GetLogAverageLuminance() const
{
	return m_logAverageLuminance;
}

ImageDetailsView^ ImageDetails::Get() const
{
	if (!m_isValid)
		return nullptr;

	ImageDetailsView^ details = ref new ImageDetailsView();

	details->FileName = m_fileName;
	details->FileType = m_fileType;

	details->Width = m_width;
	details->Height = m_height;
	details->Format = FormatToString(m_format);

	details->MaxRed = ref new Color();
	details->MaxRed->Red = GetMaxRed();
	details->MaxGreen = ref new Color();
	details->MaxGreen->Green = GetMaxGreen();
	details->MaxBlue = ref new Color();
	details->MaxBlue->Blue = GetMaxBlue();
	details->MaxLuminance = ref new GreyValueColor();
	details->MaxLuminance->Red = GetMaxLuminance();

	details->MinRed = ref new Color();
	details->MinRed->Red = GetMinRed();
	details->MinGreen = ref new Color();
	details->MinGreen->Green = GetMinGreen();
	details->MinBlue = ref new Color();
	details->MinBlue->Blue = GetMinBlue();
	details->MinLuminance = ref new GreyValueColor();
	details->MinLuminance->Red = GetMinLuminance();

	details->AverageColor = ref new Color();
	details->AverageColor->Red = GetAverageRed();
	details->AverageColor->Green = GetAverageGreen();
	details->AverageColor->Blue = GetAverageBlue();
	details->AverageLuminance = ref new GreyValueColor();
	details->AverageLuminance->Red = GetAverageLuminance();

	details->LogAverageLuminance = ref new GreyValueColor();
	details->LogAverageLuminance->Red = GetLogAverageLuminance();

	return details;
}
