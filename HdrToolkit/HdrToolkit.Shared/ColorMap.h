#pragma once
#include "pch.h"
#include "RenderTexture.h"

namespace HdrToolkit
{
	enum class NamedColorMap
	{
		// Customized
		blkCustom,
		// Perceptually Uniform Sequential
		viridis,
		plasma,
		inferno,
		magma,
		// Sequential
		Greys,
		Purples,
		Blues,
		Greens,
		Oranges,
		Reds,
		YlOrBr,
		YlOrRd,
		OrRd,
		PuRd,
		RdPu,
		BuPu,
		GnBu,
		PuBu,
		YlGnBu,
		PuBuGn,
		BuGn,
		YlGn,
		// Sequential (2)
		binary,
		gist_yarg,
		gist_gray,
		gray,
		bone,
		pink,
		spring,
		summer,
		autumn,
		winter,
		cool,
		Wistia,
		hot,
		afmhot,
		gist_heat,
		copper,
		// Diverging
		PiYG,
		PRGn,
		BrBG,
		PuOr,
		RdGy,
		RdBu,
		RdYlBu,
		RdYlGn,
		Spectral,
		coolwarm,
		bwr,
		seismic,
		// Miscellaneous
		ocean,
		gist_earth,
		terrain,
		gist_stern,
		gnuplot,
		gnuplot2,
		CMRmap,
		cubehelix,
		brg,
		hsv,
		gist_rainbow,
		rainbow,
		jet,
		nipy_spectral,
		gist_ncar,
	};

	public ref class ColorMap sealed : public Windows::UI::Xaml::Data::ICustomPropertyProvider
	{
	internal:
		template<typename Iterator>
		ColorMap(Iterator colorsBegin, Iterator colorsEnd, Platform::String^ name)
			: m_map(colorsBegin, colorsEnd), m_name(name)
		{
		}

		template<typename T>
		ColorMap(std::initializer_list<T> list, Platform::String^ name)
			: m_map(list), m_name(name)
		{
		}

		void CreateTexture(ID3D11Device* device, RenderTexture& texture, bool invert = false);

		static ColorMap^ GetColorMap(NamedColorMap name);

	public:
		ColorMap()
			: m_name(L"Invalid")
		{
		}

		Windows::UI::Xaml::Media::Brush^ GetFill(bool invert);

		static ColorMap^ GetColorMap(Platform::String^ name);

		// Geerbt über ICustomPropertyProvider
		virtual property Windows::UI::Xaml::Interop::TypeName Type
		{
			virtual Windows::UI::Xaml::Interop::TypeName get() sealed
			{
				return Windows::UI::Xaml::Interop::TypeName();
			}
		};
		virtual Windows::UI::Xaml::Data::ICustomProperty^ GetCustomProperty(Platform::String ^name) sealed;
		virtual Windows::UI::Xaml::Data::ICustomProperty^ GetIndexedProperty(Platform::String ^name, Windows::UI::Xaml::Interop::TypeName type) sealed;
		virtual Platform::String^ GetStringRepresentation() sealed;

	private:
		std::vector<DirectX::XMFLOAT4> m_map;
		Platform::String^ m_name;
	};

}