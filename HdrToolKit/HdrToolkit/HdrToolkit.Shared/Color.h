#pragma once

namespace HdrToolkit
{
	public interface class IColor
	{
	public:
		property float Red;
		property float Green;
		property float Blue;
		property float Alpha;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class GreyValueColor sealed : IColor
	{
	private:
		float m_color;

	public:
		GreyValueColor();

		virtual property float Red
		{
			virtual float get();
			virtual void set(float value);
		}
		virtual property float Green
		{
			virtual float get();
			virtual void set(float value);
		}
		virtual property float Blue
		{
			virtual float get();
			virtual void set(float value);
		}
		virtual property float Alpha;
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class Color sealed : IColor
	{
	public:
		Color();

		virtual property float Red;
		virtual property float Green;
		virtual property float Blue;
		virtual property float Alpha;
	};
}