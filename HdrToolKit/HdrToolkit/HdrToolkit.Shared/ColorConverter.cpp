#include "pch.h"
#include "ColorConverter.h"

using namespace HdrToolkit;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

float HdrToolkit::ComputeLuminance(DirectX::XMFLOAT3 color)
{
	return ComputeLuminance(color.x, color.y, color.z);
}

float HdrToolkit::ComputeLuminance(DirectX::XMFLOAT4 color)
{
	return ComputeLuminance(color.x, color.y, color.z);
}

// See http://jcgt.org/published/0002/02/01/
float xFit_1931(float wave)
{
	float t1 = (wave - 442.0f) * ((wave < 442.0f) ? 0.0624f : 0.0374f);
	float t2 = (wave - 599.8f) * ((wave < 599.8f) ? 0.0264f : 0.0323f);
	float t3 = (wave - 501.1f) * ((wave < 501.1f) ? 0.0490f : 0.0382f);
	return 0.362f * expf(-0.5f * t1 * t1) + 1.056f * expf(-0.5f * t2 * t2) - 0.065f * expf(-0.5f * t3 * t3);
}
float yFit_1931(float wave)
{
	float t1 = (wave - 568.8f) * ((wave < 568.8f) ? 0.0213f : 0.0247f);
	float t2 = (wave - 530.9f) * ((wave < 530.9f) ? 0.0613f : 0.0322f);
	return 0.821f * exp(-0.5f * t1 * t1) + 0.286f * expf(-0.5f * t2 * t2);
}
float zFit_1931(float wave)
{
	float t1 = (wave - 437.0f) * ((wave < 437.0f) ? 0.0845f : 0.0278f);
	float t2 = (wave - 459.0f) * ((wave < 459.0f) ? 0.0385f : 0.0725f);
	return 1.217f*exp(-0.5f * t1 * t1) + 0.681f * expf(-0.5f * t2 * t2);
}

float postprocess(float c)
{
	return std::max(0.0f, std::min(1.0f, c));
}

// See http://www.color.org/chardata/rgb/sRGB.pdf
DirectX::XMFLOAT3 HdrToolkit::WavelengthToRgb(float wave)
{
	float x = xFit_1931(wave);
	float y = yFit_1931(wave);
	float z = zFit_1931(wave);

	float rl = 3.2406255f * x + -1.537208f * y + -0.4986286f * z;
	float gl = -0.9689307f * x + 1.8757561f * y + 0.0415175f * z;
	float bl = 0.0557101f * x + -0.2040211f * y + 1.0569959f * z;

	return{ postprocess(rl), postprocess(gl), postprocess(bl) };
}

//DirectX::XMFLOAT3 HdrToolkit::WavelengthToRgb(float Wavelength)
//{
//	float factor;
//	float Red;
//	float Green;
//	float Blue;
//
//	if ((Wavelength >= 380.0f) && (Wavelength < 440.0f))
//	{
//		Red = -(Wavelength - 440.0f) / (440.0f - 380.0f);
//		Green = 0.0;
//		Blue = 1.0;
//	}
//	else if ((Wavelength >= 440.0f) && (Wavelength < 490.0f))
//	{
//		Red = 0.0;
//		Green = (Wavelength - 440.0f) / (490.0f - 440.0f);
//		Blue = 1.0;
//	}
//	else if ((Wavelength >= 490.0f) && (Wavelength < 510.0f))
//	{
//		Red = 0.0;
//		Green = 1.0;
//		Blue = -(Wavelength - 510.0f) / (510.0f - 490.0f);
//	}
//	else if ((Wavelength >= 510.0f) && (Wavelength < 580.0f))
//	{
//		Red = (Wavelength - 510.0f) / (580.0f - 510.0f);
//		Green = 1.0;
//		Blue = 0.0;
//	}
//	else if ((Wavelength >= 580.0f) && (Wavelength < 645.0f))
//	{
//		Red = 1.0;
//		Green = -(Wavelength - 645.0f) / (645.0f - 580.0f);
//		Blue = 0.0;
//	}
//	else if ((Wavelength >= 645.0f) && (Wavelength < 781.0f))
//	{
//		Red = 1.0;
//		Green = 0.0;
//		Blue = 0.0;
//	}
//	else
//	{
//		Red = 0.0;
//		Green = 0.0;
//		Blue = 0.0;
//	};
//
//	// Let the intensity fall off near the vision limits
//
//	if ((Wavelength >= 380.0f) && (Wavelength < 420.0f))
//	{
//		factor = 0.3 + 0.7 * (Wavelength - 380.0f) / (420.0f - 380.0f);
//	}
//	else if ((Wavelength >= 420.0f) && (Wavelength < 701.0f))
//	{
//		factor = 1.0;
//	}
//	else if ((Wavelength >= 701.0f) && (Wavelength < 781.0f))
//	{
//		factor = 0.3 + 0.7 * (780.0f - Wavelength) / (780.0f - 700.0f);
//	}
//	else {
//		factor = 0.0;
//	};
//
//
//	//int[] rgb = new int[3];
//
//	//// Don't want 0^x = 1 for x <> 0
//	//rgb[0] = Red == 0.0 ? 0 : (int)Math.round(IntensityMax * Math.pow(Red * factor, Gamma));
//	//rgb[1] = Green == 0.0 ? 0 : (int)Math.round(IntensityMax * Math.pow(Green * factor, Gamma));
//	//rgb[2] = Blue == 0.0 ? 0 : (int)Math.round(IntensityMax * Math.pow(Blue * factor, Gamma));
//
//	DirectX::XMFLOAT3 rgb(Red * factor, Green * factor, Blue * factor);
//
//	return rgb;
//}

Windows::UI::Color HdrToolkit::ConvertToColor(Object^ value)
{
	if (auto icolor = safe_cast<IColor^>(value))
	{
		return ConvertToColor(icolor);
	}
	else
	{
		Windows::UI::Color color;
		color.R = color.G = color.B = 0;
		color.A = 255;
		return color;
	}
}

Windows::UI::Color HdrToolkit::ConvertToColor(IColor^ color)
{
	Windows::UI::Color result;
	result.R = ConvertToChar(color->Red);
	result.G = ConvertToChar(color->Green);
	result.B = ConvertToChar(color->Blue);
	result.A = ConvertToChar(color->Alpha);
	return result;
}

Windows::UI::Color HdrToolkit::ConvertToColor(DirectX::XMFLOAT3 color)
{
	Windows::UI::Color result;
	result.R = ConvertToChar(color.x);
	result.G = ConvertToChar(color.y);
	result.B = ConvertToChar(color.z);
	result.A = 255;
	return result;
}

Windows::UI::Color HdrToolkit::ConvertToColor(DirectX::XMFLOAT4 color)
{
	return ConvertToColor(DirectX::XMFLOAT3(color.x, color.y, color.z));
}

ToBrushConverter::ToBrushConverter()
{
}

Object^ ToBrushConverter::Convert(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	auto color = ConvertToColor(value);
	return ref new SolidColorBrush(color);
}

Object^ ToBrushConverter::ConvertBack(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	return value;
}

ToBrushConverter::~ToBrushConverter()
{
}

ToStringConverter::ToStringConverter()
{
}

Object^ ToStringConverter::Convert(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	if (auto icolor = safe_cast<IColor^>(value))
	{
		if (icolor->Red == icolor->Green && icolor->Green == icolor->Blue)
		{
			return "" + icolor->Red;
		}
		else
		{
			return "(" + icolor->Red + ", " + icolor->Green + ", " + icolor->Blue + ")";
		}
	}
	else
	{
		return "0";
	}
}

Object^ ToStringConverter::ConvertBack(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	return value;
}

ToStringConverter::~ToStringConverter()
{
}
