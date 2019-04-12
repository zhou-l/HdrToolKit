#pragma once

#include "pch.h"

#include "Color.h"

namespace HdrToolkit
{
	template<typename T>
	unsigned char ConvertToChar(T color)
	{
		color *= static_cast<T>(255);
		if (color >= static_cast<T>(255))
		{
			return 255;
		}
		else if (color <= static_cast<T>(0))
		{
			return 0;
		}
		else
		{
			return static_cast<unsigned char>(color);
		}
	}

	template<typename T>
	T ComputeLuminance(T red, T green, T blue)
	{
		// See http://www.itu.int/rec/R-REC-BT.709
		static T redCoefficient = static_cast<T>(0.2126);
		static T greenCoefficient = static_cast<T>(0.7152);
		static T blueCoefficient = static_cast<T>(0.0722);
		return red * redCoefficient + green * greenCoefficient + blue * blueCoefficient;
	}

	float ComputeLuminance(DirectX::XMFLOAT3 color);

	float ComputeLuminance(DirectX::XMFLOAT4 color);
	
	Windows::UI::Color ConvertToColor(Platform::Object^ value);
	Windows::UI::Color ConvertToColor(IColor^ color);
	Windows::UI::Color ConvertToColor(DirectX::XMFLOAT3 color);
	Windows::UI::Color ConvertToColor(DirectX::XMFLOAT4 color);

	DirectX::XMFLOAT3 WavelengthToRgb(float Wavelength);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ToBrushConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		ToBrushConverter();

		virtual Platform::Object^ Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);
		virtual Platform::Object^ ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);

	private:
		~ToBrushConverter();
	};

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class ToStringConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		ToStringConverter();

		virtual Platform::Object^ Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);
		virtual Platform::Object^ ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);

	private:
		~ToStringConverter();
	};
}